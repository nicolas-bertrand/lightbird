#include <QDir>
#include <QJsonArray>
#include <QJsonDocument>
#include <QUrlQuery>
#include <QUuid>

#include "LightBird.h"
#include "Plugin.h"
#include "Uploads.h"

Uploads::Uploads(QObject *parent)
    : QObject(parent)
{
}

Uploads::~Uploads()
{
}

void    Uploads::onDeserialize(LightBird::IClient &client, LightBird::IOnDeserialize::Deserialize type, const QString &uri)
{
    if (uri.endsWith("/send"))
    {
        if (type == LightBird::IOnDeserialize::IDoDeserializeHeader)
            this->_send_onDeserializeHeader(client);
        else if (type == LightBird::IOnDeserialize::IDoDeserializeContent)
            this->_send_onDeserializeContent(client);
    }
    else if (uri.endsWith("/cancel"))
    {
        if (type == LightBird::IOnDeserialize::IDoDeserializeHeader)
            this->_cancel_onDeserializeHeader(client);
    }
}

void        Uploads::onFinish(LightBird::IClient &client)
{
    QString id = client.getInformations().value("idUpload").toString();
    this->mutex.lock();
    QSharedPointer<Upload> upload(this->uploads.value(id));
    this->mutex.unlock();

    if (upload && upload->idClient == client.getId())
    {
        upload->fileTable.remove();
        if (upload->file->isOpen())
            upload->file->remove();
        upload->finished = QDateTime::currentDateTime();
    }
}

void        Uploads::onDestroy(LightBird::IClient &client)
{
    QString id = client.getInformations().value("idUpload").toString();
    this->mutex.lock();
    QSharedPointer<Upload> upload(this->uploads.value(id));
    this->mutex.unlock();

    if (this->uploads.contains(id) && upload->idClient == client.getId())
    {
        upload->fileTable.remove();
        if (upload->file->isOpen())
            upload->file->remove();
        upload->finished = QDateTime::currentDateTime();
    }
}

void    Uploads::cancel(LightBird::IClient &client)
{
    LightBird::IContent &content = client.getRequest().getContent();
    QJsonDocument document;
    if (content.getStorage() == LightBird::IContent::VARIANT)
        document = content.getVariant()->toJsonDocument();
    else
        document = QJsonDocument::fromJson(content.getData());
    QJsonArray array = document.array();
    if (array.isEmpty())
        return Plugin::httpError(client, 400, "Bad request", "Empty array");
    QStringList filesToRemove;

    // Searches the files to delete
    for (int i = 0, s = array.size(); i < s; ++i)
    {
        this->mutex.lock();
        QMapIterator<QString, QSharedPointer<Upload> > it(this->uploads);
        while (it.hasNext())
        {
            it.next();
            if (it.value()->cancelId == array[i].toString())
            {
                // Only the account that started the upload can cancel it
                if (it.value()->idAccount == client.getAccount().getId())
                    // Removes the files of the upload
                    for (QListIterator<File> file(it.value()->files); file.hasNext(); file.next())
                        filesToRemove.append(file.peekNext().id);
                break;
            }
        }
        this->mutex.unlock();
    }

    // Removes the files outside the mutex
    LightBird::TableFiles tableFiles;
    for (QStringListIterator file(filesToRemove); file.hasNext(); file.next())
    {
        tableFiles.setId(file.peekNext());
        tableFiles.remove(true);
    }
}

void        Uploads::_send_onDeserializeHeader(LightBird::IClient &client)
{
    Upload  *upload = new Upload();

    // Gets some data on the files to upload
    upload->id = LightBird::createUuid();
    upload->cancelId = QUrlQuery(client.getRequest().getUri()).queryItemValue("cancelId");
    upload->idClient = client.getId();
    upload->idAccount = client.getAccount().getId();
    upload->file = QSharedPointer<QFile>(new QFile());
    upload->progress = 0;
    upload->path = LightBird::cleanPath(QUrl::fromPercentEncoding(QUrlQuery(client.getRequest().getUri()).queryItemValue("path").toLatin1()), true) + "/";
    upload->size = client.getRequest().getHeader().value("content-length").toULongLong();
    upload->boundary = client.getRequest().getInformations().value("media-type").toMap().value("boundary").toByteArray();
    upload->boundary = "--" + upload->boundary.right(upload->boundary.size() - upload->boundary.indexOf('=') - 1);
    upload->header = true;
    upload->complete = false;
    client.getInformations().insert("idUpload", upload->id);
    this->mutex.lock();
    this->uploads.insert(upload->id, QSharedPointer<Upload>(upload));
    this->mutex.unlock();
    if (Plugin::api().log().isDebug())
        Plugin::api().log().debug("Upload started", Properties("idUpload", upload->id).add("idClient", client.getId()).add("path", upload->path).toMap(), "Uploads", "_send_onDeserializeHeader");
    // Some browsers send a negative content length when there is too many files (firefox 10, safari 5)
    if (client.getRequest().getHeader().value("content-length").toLongLong() < 0)
    {
        Plugin::api().log().error("Negative content-length", Properties("idUpload", upload->id).add("idClient", client.getId()).toMap(), "Uploads", "_send_onDeserializeHeader");
        Plugin::api().network().disconnect(client.getId(), true);
    }
    // Tells the parser to store the data in the memory and not in a temporary file
    client.getInformations().insert("keepInMemory", true);
    this->_send_removeCompleteUploads();
    // Prepare the response, which will contain the id of the files successfully uploaded
    (*client.getResponse().getContent().setStorage(LightBird::IContent::VARIANT).getVariant()) = QJsonDocument(QJsonArray());
    client.getResponse().setType("application/json");
}

void            Uploads::_send_onDeserializeContent(LightBird::IClient &client)
{
    QString     id = client.getInformations().value("idUpload").toString();
    QByteArray  &data = *client.getRequest().getContent().getByteArray();
    qint64      size = data.size();
    qint64      position = 0;
    qint64      i = 0, j = 0;
    qint64      boundarySize;

    this->mutex.lock();
    QSharedPointer<Upload> upload(this->uploads.value(id));
    this->mutex.unlock();
    if (!upload || upload->idClient != client.getId())
        return ;
    // \r\n + boundary
    boundarySize = upload->boundary.size() + 2;
    // The upload has already been completed
    if (upload->complete)
    {
        data.clear();
        return ;
    }
    // Too few data to continue
    if (data.size() < boundarySize && upload->progress + size < upload->size)
        return ;
    // Parses the multipart/form-data
    while (position < size)
    {
        // Each header represents an uploaded file
        if (upload->header)
        {
            // Header not found
            if ((j = data.indexOf("\r\n\r\n", position)) < 0 && size - position > MAX_HEADER_LENGTH)
                return this->_send_error(client, *upload, "Header missing");
            // Extracts data from it
            else if (j >= 0)
            {
                File file;
                position = data.indexOf("filename=\"", position);
                i = position + 10;
                if (position >= 0)
                    file.name = QString::fromUtf8(data.mid(i, data.indexOf("\"\r\n", i) - i));
                position = data.indexOf("Content-Type: ", position);
                i = position + 14;
                if (position >= 0)
                    file.contentType = data.mid(i, j - i);
                position = j + 4;
                // If the content is a file we create it
                if (!file.name.isEmpty() && !file.contentType.isEmpty())
                    this->_send_createFile(client, *upload, file);
                upload->header = false;
            }
            // The header is not complete yet, so we wait for more data
            else
            {
                data = data.right(data.size() - position);
                upload->progress += position;
                return ;
            }
        }
        // The content of the current file
        if (!upload->header)
        {
            // The boundary of the content is between two piece of data
            if (!upload->oldData.isEmpty() && (i = (upload->oldData + data.left(boundarySize)).indexOf("\r\n" + upload->boundary)) > 0)
            {
                // Writes the old data content in the file
                upload->file->write(upload->oldData.data(), i);
                position = boundarySize - upload->oldData.size() + i;
                upload->oldData.clear();
                // The file is complete
                this->_send_fileComplete(client, *upload);
                this->_send_clean(*upload);
                // End of the upload
                if ((upload->complete = (data.indexOf("\r\n", position) != position)))
                    break;
                // Extracts the next header
                continue;
            }
            // Writes the old data in the file since they don't contains a part of the boundary
            else if (!upload->oldData.isEmpty())
            {
                upload->file->write(upload->oldData);
                upload->oldData.clear();
            }
            // Searches the end of the content
            if ((i = data.indexOf("\r\n" + upload->boundary, position)) >= 0)
            {
                // Writes the data in the file
                upload->file->write(data.data() + position, i - position);
                position = i + 2;
                // The file is complete
                this->_send_fileComplete(client, *upload);
                this->_send_clean(*upload);
                // End of the upload
                if ((upload->complete = (data.indexOf("\r\n", position) - position > upload->boundary.size())))
                    break;
                // Extracts the next header
                continue;
            }
            // The end of the content is missing
            else if (upload->progress + size == upload->size)
                return this->_send_error(client, *upload, "The end of content is missing");
            // Writes the content in the file and waits for more data
            else if (size - position > boundarySize)
            {
                upload->file->write(data.data() + position, size - position - boundarySize);
                upload->oldData = data.right(boundarySize);
                break;
            }
            // Waits for more data
            else
            {
                upload->oldData = data.right(size - position);
                break;
            }
        }
    }
    upload->progress += size;
    data.clear();
}

void    Uploads::_send_removeCompleteUploads()
{
    this->mutex.lock();
    QMutableMapIterator<QString, QSharedPointer<Upload> > it(this->uploads);
    while (it.hasNext())
    {
        it.next();
        if (it.value()->finished.isValid() && it.value()->finished.addSecs(REMOVE_COMPLETE_UPLOAD_TIME) < QDateTime::currentDateTime())
            it.remove();
    }
    this->mutex.unlock();
}

void        Uploads::_send_createFile(LightBird::IClient &client, Upload &upload, File &file)
{
    QString filesPath = LightBird::getFilesPath();
    LightBird::TableDirectories directory;
    QString idDirectory;

    // Defines the name of the file in the file system
    file.name = LightBird::cleanPath(file.name);
    file.name = file.name.right(file.name.size() - file.name.lastIndexOf('/') - 1);
    if (file.name.contains('.'))
        file.path = file.name.left(file.name.lastIndexOf('.'));
    else
        file.path = file.name;
    file.path += '.' + LightBird::createUuid();
    if (file.name.contains('.'))
        file.path += file.name.right(file.name.size() - file.name.lastIndexOf('.'));
    upload.file->setFileName(filesPath + file.path);
    upload.files.push_back(file);
    // Creates the virtual path if it doesn't exist
    if (upload.path == "/" || !(idDirectory = directory.createVirtualPath(upload.path)).isEmpty())
    {
        directory.setId(idDirectory);
        // Checks if the account has the right to add a file
        if (directory.isAllowed(client.getAccount().getId(), "add"))
        {
            // Adds the files to the database
            if (upload.fileTable.add(file.name, file.path, "other", directory.getId(), client.getAccount().getId()))
            {
                upload.files.last().id = upload.fileTable.getId();
                upload.fileTable.setInformation("mime", file.contentType);
                // Creates the filesPath if it doesn't exist
                if (!QFileInfo(filesPath).isDir())
                    QDir().mkpath(filesPath);
                // And opens the file
                if (!upload.file->open(QIODevice::WriteOnly))
                {
                    upload.fileTable.remove(true);
                    Plugin::api().log().warning("Failed to open the file", Properties("idUpload", upload.id).add("file", file.path).toMap(), "Uploads", "_send_createFile");
                }
            }
            else
                Plugin::api().log().warning("Failed to add the file in the database", Properties("idDirectory", directory.getId()).add("path", file.path).add("name", file.name).add("idClient", client.getId()).toMap(), "Uploads", "_send_createFile");
        }
        else
            Plugin::api().log().warning("The account is not allowed to add a file in this directory", Properties("idDirectory", directory.getId()).add("path", file.path).add("name", file.name).add("idClient", client.getId()).toMap(), "Uploads", "_send_createFile");
    }
    else
        Plugin::api().log().warning("Failed to create the virtual path", Properties("path", upload.path).add("idUpload", upload.id).add("file", file.name).add("idClient", client.getId()).toMap(), "Uploads", "_send_createFile");
    (*client.getResponse().getContent().getVariant()) = QJsonDocument(client.getResponse().getContent().getVariant()->toJsonDocument().array() << "");
    // Disconnects the client if the file can't be uploaded
    if (!upload.file->isOpen() && QUrlQuery(client.getRequest().getUri()).queryItemValue("disconnectOnError") == "true")
        Plugin::api().network().disconnect(client.getId(), true);
}

void        Uploads::_send_fileComplete(LightBird::IClient &client, Upload &upload)
{
    File    file = upload.files.last();

    // The file could not be opened
    if (!upload.file->isOpen())
        return ;
    // Identify the file
    LightBird::identify(upload.fileTable.getId());
    Plugin::api().log().info("File uploaded", Properties("idFile", upload.fileTable.getId()).add("path", upload.path + file.name).add("idClient", client.getId()).toMap(), "Uploads", "_send_fileComplete");
    // Adds the id of the file to the json response
    QJsonArray filesId = client.getResponse().getContent().getVariant()->toJsonDocument().array();
    filesId[filesId.size() - 1] = upload.fileTable.getId();
    (*client.getResponse().getContent().getVariant()) = QJsonDocument(filesId);
}

void    Uploads::_send_clean(Upload &upload)
{
    upload.file->close();
    upload.fileTable.clear();
    upload.header = true;
}

void    Uploads::_send_error(LightBird::IClient &client, Upload &upload, const QString &error)
{
    // If a file was downloading we remove it
    upload.fileTable.remove();
    if (upload.file->isOpen())
        upload.file->remove();
    // Disconnects the client
    client.getRequest().getContent().clear();
    Plugin::api().network().disconnect(client.getId(), true);
    Plugin::api().log().error("An error occurred in the upload: " + error, Properties("idClient", client.getId()).add("idUpload", upload.id).toMap(), "Uploads", "_send_error");
}

void    Uploads::_cancel_onDeserializeHeader(LightBird::IClient &client)
{
    int contentLength = client.getRequest().getHeader()["content-length"].toInt();

    if (contentLength > CANCEL_MAX_CONTENT_LENGTH)
    {
        Plugin::api().network().disconnect(client.getId(), true);
        if (Plugin::api().log().isDebug())
            Plugin::api().log().debug("Content length of the cancel request too big", Properties("idClient", client.getId()).add("content-length", contentLength).add("maxContentLength", CANCEL_MAX_CONTENT_LENGTH).toMap(), "Uploads", "_cancel_onDeserializeHeader");
    }
}
