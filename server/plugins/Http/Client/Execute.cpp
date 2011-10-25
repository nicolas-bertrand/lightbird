#include <QtPlugin>
#include <QDir>
#include <QFileInfo>
#include <QUrl>
#include <QUuid>

#include "IIdentifier.h"
#include "ITableFiles.h"
#include "ITableTags.h"

#include "Audio.h"
#include "Execute.h"
#include "Medias.h"
#include "Plugin.h"
#include "Preview.h"
#include "Uploads.h"

Execute::Execute(LightBird::IApi &a, LightBird::IClient &c, const QString &com) :
         api(a), client(c), request(c.getRequest()), response(c.getResponse())
{
    QList<QByteArray>           l;
    QString                     command = com;

    // If is not connected and it doesn't try to identify
    if (client.getAccount().getId().isEmpty())
    {
        Plugin::response(this->client, 403, "Forbidden");
        return ;
    }

    // Get the command
    if (command.contains("."))
        command = command.left(command.indexOf("."));

    // Initialize the list of the available commands
    commands["Audio"] = &Execute::_audio;
    commands["Disconnect"] = &Execute::_disconnect;
    commands["Preview"] = &Execute::_preview;
    commands["Select"] = &Execute::_select;
    commands["StartUpload"] = &Execute::_startUpload;
    commands["StateUpload"] = &Execute::_stateUpload;
    commands["StopUpload"] = &Execute::_stopUpload;
    commands["StopStream"] = &Execute::_stopStream;
    commands["Video"] = &Execute::_video;
    commands["DeleteFile"] = &Execute::_deleteFile;

    // Execute the command
    if (commands.contains(command))
        (this->*(this->commands[command]))();
    // The command is unknow
    else
        Plugin::response(this->client, 404, "Not Found");
}

Execute::~Execute()
{
}

void        Execute::_audio()
{
    Medias::getInstance().start(this->client, false);
}

void        Execute::_disconnect()
{
    // Destroy the session
    Plugin::getInstance().removeSession(this->client);
    // Destroy the cookies
    this->response.getHeader().remove("set-cookie");
    this->response.getHeader().insertMulti("set-cookie", "sid=; path=/");
    this->response.getHeader().insertMulti("set-cookie", "identifiant=; path=/");
}

void        Execute::_preview()
{
    Preview preview(this->api, this->client);
    preview.go();
}

void        Execute::_select()
{
    QSqlQuery                               query;
    QVector<QMap<QString, QVariant> >       result;
    int                                     s = 0;
    QSharedPointer<LightBird::ITableFiles>  file(this->api.database().getFiles());
    QMap<QString, QVariant>                 row;
    QList<QVariant>                         rows;

    query.prepare(this->api.database().getQuery("HttpClient", "select_all_files"));
    if (!this->api.database().query(query, result))
        return Plugin::response(this->client, 500, "Internal Server Error");
    s = s;
    for (int i = 0, s = result.size(); i < s; ++i)
    {
        file->setId(result[i]["id"].toString());
        row = file->getInformations();
        row.unite(result[i]);
        rows.push_back(row);
    }
    this->response.getContent().setStorage(LightBird::IContent::VARIANT);
    *this->response.getContent().getVariant() = rows;
    this->response.setType("application/json");
}

void        Execute::_startUpload()
{
    QSharedPointer<LightBird::ITableFiles> fileTable(this->api.database().getFiles());
    QString         uri = this->request.getUri().path();
    QString         fileName;
    QString         realFileName;
    QString         realPath;
    QFile           file;
    QList<void *>   extensions;
    LightBird::IIdentify::Information information;

    // Defines the real name of the file
    fileName = QDir().cleanPath(this->request.getUri().queryItemValue("name").replace("\\", "/"));
    fileName = fileName.right(fileName.size() - fileName.lastIndexOf('/') - 1);
    if (fileName.contains("."))
        realFileName = fileName.left(fileName.indexOf('.'));
    else
        realFileName = fileName;
    realFileName += '.' + QUuid::createUuid().toString().remove(0, 1).remove(36, 1);
    if (fileName.contains('.'))
        realFileName += fileName.right(fileName.size() - fileName.indexOf('.'));
    realPath = QDir().cleanPath(this->api.configuration().get("filesPath")) + "/" + realFileName;
    // Add the file to the database
    if (!fileTable->add(fileName, realFileName, "other", "", this->client.getAccount().getId()))
    {
        this->response.setCode(403);
        this->response.setMessage("Forbidden");
        return ;
    }
    // Copy the file in the files directory
    // From a temporary file
    if (this->request.getContent().getStorage() == LightBird::IContent::TEMPORARYFILE)
    {
        // Remove the multipart
        QByteArray data = this->request.getContent().getContent(2000);
        quint64 boundary = 0;
        if (data.contains("\r\n\r\n"))
        {
            boundary = data.left(data.indexOf("\r\n")).size() + 6;
            data.remove(0, data.indexOf("\r\n\r\n") + 4);
        }
        // Copy the file
        file.setFileName(realPath);
        file.open(QIODevice::WriteOnly);
        while (data.size() > 0)
        {
            if (file.write(data) != data.size())
            {
                this->response.setCode(403);
                this->response.setMessage("Forbidden");
                file.remove();
                fileTable->remove();
                return ;
            }
            data = this->request.getContent().getContent(BUFFER_COPY_SIZE);
        }
        file.resize(file.size() - boundary);
        file.close();
    }
    // From a byte array
    else
    {
        // Remove the multipart
        QByteArray *data = this->request.getContent().getByteArray();
        if (data->contains("\r\n\r\n"))
        {
            int boundary = data->left(data->indexOf("\r\n")).size() + 6;
            data->remove(0, data->indexOf("\r\n\r\n") + 4);
            data->remove(data->size() - boundary, boundary);
        }
        // Save the file
        file.setFileName(realPath);
        if (!file.open(QIODevice::WriteOnly) || file.write(this->request.getContent().getContent()) != this->request.getContent().size())
        {
            this->response.setCode(403);
            this->response.setMessage("Forbidden");
            file.remove();
            fileTable->remove();
            return ;
        }
        file.close();
    }
    // Get information on the file
    if (!(extensions = this->api.extensions().get("IIdentifier")).isEmpty())
        information = static_cast<LightBird::IIdentifier *>(extensions.first())->identify(fileTable->getFullPath());
    this->api.extensions().release(extensions);
    if (information.type == LightBird::IIdentify::AUDIO)
        fileTable->setType("audio");
    else if (information.type == LightBird::IIdentify::DOCUMENT)
        fileTable->setType("document");
    else if (information.type == LightBird::IIdentify::IMAGE)
        fileTable->setType("image");
    else if (information.type == LightBird::IIdentify::VIDEO)
        fileTable->setType("video");
    else
        fileTable->setType("other");
    fileTable->setInformations(information.data);
}

void        Execute::_stateUpload()
{
    Uploads::Upload state = Uploads::getInstance().state(client);
    client.getResponse().getContent().setContent("{\"size\":" + QByteArray::number(state.size) + ",\"progress\":" + QByteArray::number(state.progress) + "}");
}

void        Execute::_stopUpload()
{
    Uploads::getInstance().stop(client);
}

void        Execute::_stopStream()
{
    Medias::getInstance().stop(client);
}

void        Execute::_video()
{
    Medias::getInstance().start(this->client, true);
}

void    Execute::_deleteFile()
{
    QSharedPointer<LightBird::ITableFiles> file(this->api.database().getFiles(request.getUri().queryItemValue("id")));

    if (file->exists())
    {
        if (file->isAllowed(this->client.getAccount().getId(), "delete"))
        {
            QString path = file->getFullPath();
            file->remove();
            QFile::remove(path);
        }
        else
            Plugin::response(this->client, 403, "Forbidden");
    }
    else
        Plugin::response(this->client, 404, "Not Found");
}
