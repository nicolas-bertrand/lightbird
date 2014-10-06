#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include "IDatabase.h"

#include "Files.h"
#include "Plugin.h"
#include "TableFiles.h"
#include "Properties.h"
#include "Defines.h"

Files::Files()
{
}

Files::~Files()
{
}

void    Files::get(LightBird::IClient &client)
{
    LightBird::TableFiles file;
    QSqlQuery             query(Plugin::api().database().getDatabase());
    QVector<QVariantMap>  result;
    QJsonObject           rootObject;
    QJsonArray            filesArray;

    query.prepare(Plugin::api().database().getQuery("HttpClient", "select_all_files"));
    if (!Plugin::api().database().query(query, result))
        return Plugin::response(client, 500, "Internal Server Error");
    rootObject.insert("date", QDateTime::currentDateTimeUtc().addSecs(-1).toString(DATE_FORMAT));
    for (QVectorIterator<QVariantMap> it(result); it.hasNext(); it.next())
    {
        file.setId(it.peekNext()["id"].toString());
        if (file.isAllowed(client.getAccount().getId(), "read"))
        {
            QJsonObject fileObject;
            QMapIterator<QString, QVariant> info(file.getInformations());
            while (info.hasNext())
            {
                fileObject.insert(info.peekNext().key(), info.peekNext().value().toString());
                info.next();
            }
            fileObject.insert("id", it.peekNext()["id"].toString());
            fileObject.insert("name", it.peekNext()["name"].toString());
            fileObject.insert("type", it.peekNext()["type"].toString());
            fileObject.insert("id_directory", it.peekNext()["id_directory"].toString());
            fileObject.insert("modified", it.peekNext()["modified"].toString());
            fileObject.insert("created", it.peekNext()["created"].toString());
            filesArray.append(fileObject);
        }
    }
    rootObject.insert("files", filesArray);
    client.getResponse().setType("application/json");
    (*client.getResponse().getContent().setStorage(LightBird::IContent::VARIANT).getVariant()) = QJsonDocument(rootObject);
}

void    Files::deleteFiles(LightBird::IClient &client)
{
    LightBird::IRequest &request = client.getRequest();
    QByteArray data = request.getContent().getData();
    bool started = request.getInformations().contains("files/delete/oldData"); // False the first time this request is parsed
    QByteArray oldData = request.getInformations().value("files/delete/oldData").toByteArray(); // The part of the id of a file parsed during the previous pass
    bool readingNextId = request.getInformations().value("files/delete/readingNextId").toBool(); // True if an id is being read
    int indexOf;

    if (data.isEmpty())
        return ;

    request.getContent().clear();
    request.getInformations()["files/delete/oldData"] = "";

    if (request.getHeader()["content-length"].toInt() > deleteMaxContentLength)
        return this->_deleteError(client, "Maximum content length exceeded: " + request.getHeader()["content-length"] + " > " + QString::number(deleteMaxContentLength));
    // First pass
    if (!started)
    {
        if (data[0] != '[')
            return this->_deleteError(client, "The content must start with [");
        data.remove(0, 1);
        (*client.getResponse().getContent().setStorage(LightBird::IContent::VARIANT).getVariant()) = QJsonDocument(QJsonArray());
        client.getResponse().setType("application/json");
    }
    data.prepend(oldData);
    while (!data.isEmpty())
    {
        indexOf = data.indexOf('\"');
        // We are reading the next id
        if (readingNextId)
        {
            // Not enougth data have been received. Saves the data read so far.
            if (indexOf < 0)
            {
                request.getInformations()["files/delete/oldData"] = data;
                if (data.size() > deleteFileIdMaxSize)
                    return this->_deleteError(client, "The size of this file id is abnormally large");
            }
            // File id complete
            else
            {
                this->_deleteFile(client, QString(data.left(indexOf)));
                data.remove(0, indexOf + 1);
                readingNextId = false;
            }
        }
        // We are at the start of the next id
        else if (indexOf >= 0)
        {
            data.remove(0, indexOf + 1);
            readingNextId = true;
        }
        // Not enougth data
        if (indexOf < 0)
            break ;
    }
    request.getInformations()["files/delete/readingNextId"] = readingNextId;
}

void    Files::_deleteFile(LightBird::IClient &client, const QString &id)
{
    LightBird::TableFiles file(id);

    // If the file can't be deleted we add its id in the response
    if (!file.exists() || !file.isAllowed(client.getAccount().getId(), "delete"))
        (*client.getResponse().getContent().getVariant()) = QJsonDocument(client.getResponse().getContent().getVariant()->toJsonDocument().array() << id);
    if (Plugin::api().log().isDebug())
        Plugin::api().log().debug("File removed", Properties("idClient", client.getId()).add("idFile", id).toMap(), "Files", "_deleteFile");
    // Removes the file from the database
    file.remove(true);
}

void    Files::_deleteError(LightBird::IClient &client, const QString &message)
{
    if (Plugin::api().log().isTrace())
        Plugin::api().log().trace("Error: " + message, Properties("idClient", client.getId()).toMap(), "Files", "deleteFiles");
    Plugin::api().network().disconnect(client.getId(), true);
}
