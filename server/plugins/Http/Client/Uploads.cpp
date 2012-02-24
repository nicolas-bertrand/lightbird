#include <QUuid>
#include <QDir>

#include "IIdentify.h"
#include "IIdentifier.h"

#include "LightBird.h"
#include "Plugin.h"
#include "Uploads.h"

Uploads::Uploads(QObject *parent) : QObject(parent)
{
    this->maxHeaderLength = MAX_HEADER_LENGTH;
}

Uploads::~Uploads()
{
}

void        Uploads::onUnserializeHeader(LightBird::IClient &client)
{
    QString id = client.getRequest().getUri().queryItemValue("id");
    Upload  upload;
    LightBird::TableDirectories directory;

    // Gets some data on the files to upload
    upload.idClient = client.getId();
    upload.idAccount = client.getAccount().getId();
    upload.file = QSharedPointer<QFile>(new QFile());
    upload.progress = 0;
    upload.path = LightBird::cleanPath(QUrl::fromPercentEncoding(client.getRequest().getUri().queryItemValue("path").toAscii()), true) + "/";
    upload.size = client.getRequest().getHeader().value("content-length").toULongLong();
    upload.boundary = client.getRequest().getInformations().value("media-type").toMap().value("boundary").toByteArray();
    upload.boundary = "--" + upload.boundary.right(upload.boundary.size() - upload.boundary.indexOf('=') - 1);
    upload.header = true;
    upload.complete = false;
    client.getInformations().insert("idUpload", id);
    // Creates the destination directory
    directory.createVirtualPath(upload.path);
    this->mutex.lock();
    // If the id is valid, insert it in the map
    if (!id.isEmpty() && !this->uploads.contains(id))
    {
        Plugin::api().log().debug("Upload started", Properties("idUpload", id).add("idClient", client.getId()).toMap(), "Uploads", "onUnserializeHeader");
        this->uploads.insert(id, upload);
    }
    // Otherwise an upload is already using this id
    else
    {
        Plugin::api().log().error("This upload id is already used", Properties("idUpload", id).add("idClient", client.getId()).toMap(), "Uploads", "onUnserializeHeader");
        client.getRequest().setError(true);
        Plugin::api().network().disconnect(client.getId());
    }
    this->mutex.unlock();
    // Some browsers send a negative content length when there is too many files (firefox 10, safari 5)
    if (client.getRequest().getHeader().value("content-length").toLongLong() < 0)
    {
        Plugin::api().log().error("Negative content-length", Properties("idClient", client.getId()).toMap(), "Uploads", "onUnserializeHeader");
        client.getRequest().setError(true);
        Plugin::api().network().disconnect(client.getId());
    }
    // Tells the parser to store the data in the memory and not in a temporary file
    client.getInformations().insert("keepInMemory", true);
    this->_removeCompleteUploads();
}

void            Uploads::onUnserializeContent(LightBird::IClient &client)
{
    QString     id = client.getInformations().value("idUpload").toString();
    QString     filesPath = LightBird::TableFiles::getFilesPath();
    QByteArray  &data = *client.getRequest().getContent().getByteArray();
    qint64      size = data.size();
    qint64      position = 0;
    qint64      i, j = 0;
    qint64      boundarySize;

    this->mutex.lock();
    if (!this->uploads.contains(id) || this->uploads[id].idClient != client.getId())
        return this->mutex.unlock();
    Upload &upload = this->uploads[id];
    // \r\n + boundary
    boundarySize = upload.boundary.size() + 2;
    // The upload has already been completed
    if (upload.complete)
    {
        data.clear();
        return this->mutex.unlock();
    }
    // Too few data to continue
    if (data.size() < boundarySize && upload.progress + size < upload.size)
        return this->mutex.unlock();
    // Parse the multipart/form-data
    while (position < size)
    {
        // Each header represents an uploaded file
        if (upload.header)
        {
            // Header not found
            if ((j = data.indexOf("\r\n\r\n", position)) < 0 && size - position > this->maxHeaderLength)
                return this->_error(client, upload, "Header missing");
            // Extracts data from it
            else if (j >= 0)
            {
                File file;
                position = data.indexOf("filename=\"", position);
                i = position + 10;
                file.name = QString::fromUtf8(data.mid(i, data.indexOf("\"\r\n", i) - i));
                position = data.indexOf("Content-Type: ", position);
                i = position + 14;
                file.contentType = data.mid(i, j - i);
                position = j + 4;
                // The header does not contains the required informations
                if (file.name.isEmpty() || file.contentType.isEmpty())
                    return this->_error(client, upload, "The fileName or the content-type is missing in the header");
                // Defines the real name of the file
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
                upload.header = false;
                // Creates the filesPath if it doesn't exist
                if (!QFileInfo(filesPath).isDir())
                    QDir().mkpath(filesPath);
                // And opens the file
                if (!upload.file->open(QIODevice::WriteOnly))
                    Plugin::api().log().warning("Failed to open the uploaded file", Properties("idUpload", id).add("file", file.path).toMap(), "Uploads", "onUnserializeContent");
            }
            // The header is not complete yet, so we wait for more data
            else
            {
                data = data.right(data.size() - position);
                upload.progress += position;
                return this->mutex.unlock();
            }
        }
        // The content of the current file
        if (!upload.header)
        {
            // The boundary of the content is between two piece of data
            if (!upload.oldData.isEmpty() && (i = (upload.oldData + data.left(boundarySize)).indexOf("\r\n" + upload.boundary)) > 0)
            {
                // Writes the old data content in the file
                upload.file->write(upload.oldData.data(), i);
                position = boundarySize - upload.oldData.size() + i;
                upload.oldData.clear();
                // Inserts the file in the database
                this->_insert(client, upload);
                this->_clean(upload);
                // End of the upload
                if ((upload.complete = (data.indexOf("\r\n", position) != position)))
                    break;
                // Extracts the next header
                continue;
            }
            // Writes the old data in the file since they don't contains a part of the boundary
            else if (!upload.oldData.isEmpty())
            {
                upload.file->write(upload.oldData);
                upload.oldData.clear();
            }
            // Searches the end of the content
            if ((i = data.indexOf("\r\n" + upload.boundary, position)) >= 0)
            {
                // Writes the data in the file
                upload.file->write(data.data() + position, i - position);
                position = i + 2;
                // Inserts the file in the database
                this->_insert(client, upload);
                this->_clean(upload);
                // End of the upload
                if ((upload.complete = (data.indexOf("\r\n", position) - position > upload.boundary.size())))
                    break;
                // Extracts the next header
                continue;
            }
            // The end of the content is missing
            else if (upload.progress + size == upload.size)
                return this->_error(client, upload, "The end of content is missing");
            // Writes the content in the file and waits for more data
            else if (size - position > boundarySize)
            {
                upload.file->write(data.data() + position, size - position - boundarySize);
                upload.oldData = data.right(boundarySize);
                break;
            }
            // Waits for more data
            else
            {
                upload.oldData = data.right(size - position);
                break;
            }
        }
    }
    upload.progress += size;
    data.clear();
    this->mutex.unlock();
}

void        Uploads::doExecution(LightBird::IClient &client)
{
    QString id = client.getInformations().value("idUpload").toString();

    this->mutex.lock();
    if (this->uploads.contains(id) && this->uploads[id].idClient == client.getId())
        Plugin::api().timers().setTimer("uploads");
    this->mutex.unlock();
}

bool                Uploads::timer()
{
    QList<void *>   extensions;
    QStringList     files;
    LightBird::IIdentify::Information information;
    LightBird::TableFiles file;

    // Gets the files to identify
    this->mutex.lock();
    files = this->identify;
    this->identify.clear();
    this->mutex.unlock();
    // While there are files to identify
    while (!files.isEmpty())
    {
        QStringListIterator it(files);
        while (it.hasNext())
            // Identify the file
            if (file.setIdFromVirtualPath(it.next()))
            {
                if (!(extensions = Plugin::api().extensions().get("IIdentifier")).isEmpty())
                    information = static_cast<LightBird::IIdentifier *>(extensions.first())->identify(file.getFullPath());
                Plugin::api().extensions().release(extensions);
                file.setType(information.type_string);
                if (information.data.value("mime").toString() == "application/octet-stream")
                    information.data.remove("mime");
                file.setInformations(information.data);
                information.data.clear();
            }
        files.clear();
        // If some files have been uploaded in the meantime, we continue the identification
        this->mutex.lock();
        files = this->identify;
        this->identify.clear();
        this->mutex.unlock();
    }
    return (false);
}

void                Uploads::check(LightBird::IClient &client)
{
    QString         path = QUrl::fromPercentEncoding(client.getRequest().getUri().queryItemValue("path").toAscii());
    QVariantList    files;
    QVariantList    denied;
    LightBird::TableFiles file;

    if (client.getRequest().getContent().getStorage() == LightBird::IContent::VARIANT
        && !(files = client.getRequest().getContent().getVariant()->toList()).isEmpty())
    {
        // Generates the list of the files that can't be uploaded
        QListIterator<QVariant> it(files);
        while (it.hasNext())
            if (!it.next().toString().isEmpty() && file.setIdFromVirtualPath(path + "/" + it.peekPrevious().toByteArray()))
                denied << it.peekPrevious().toByteArray();
        // Send the list
        (*client.getResponse().getContent().setStorage(LightBird::IContent::VARIANT).getVariant()) = denied;
        client.getResponse().setType(client.getRequest().getType());
    }
}

void        Uploads::progress(LightBird::IClient &client)
{
    QString id = client.getRequest().getUri().queryItemValue("id");

    client.getResponse().setType("application/json");
    this->mutex.lock();
    if (this->uploads.contains(id) && this->uploads[id].idAccount == client.getAccount().getId())
        client.getResponse().getContent().setContent("{\"size\":" + QByteArray::number(this->uploads[id].size) +
                                                     ",\"progress\":" + QByteArray::number(this->uploads[id].progress) +
                                                     ",\"complete\":" + QVariant(this->uploads[id].complete).toString().toAscii() + "}");
    // The upload request is not yet arrived.
    else
        client.getResponse().getContent().setContent("{\"complete\":false}");
    this->mutex.unlock();
}

void        Uploads::stop(LightBird::IClient &client)
{
    QString id = client.getRequest().getUri().queryItemValue("id");

    this->mutex.lock();
    if (this->uploads.contains(id) && this->uploads[id].idAccount == client.getAccount().getId())
    {
        Plugin::api().log().info("Upload stopped", Properties("idClient", this->uploads[id].idClient).add("idUpload", id).toMap(), "Uploads", "stop");
        if (!this->uploads[id].complete)
            Plugin::api().network().disconnect(this->uploads[id].idClient);
        this->uploads[id].complete = true;
        Plugin::api().timers().setTimer("uploads");
    }
    this->mutex.unlock();
}

void                Uploads::cancel(LightBird::IClient &client)
{
    QString         id = client.getRequest().getUri().queryItemValue("id");
    QVariantList    files;
    LightBird::TableFiles file;

    this->mutex.lock();
    if (this->uploads.contains(id) && this->uploads[id].idAccount == client.getAccount().getId())
    {
        // Removes all the files uploaded so far
        QListIterator<File> it(this->uploads[id].files);
        while (it.hasNext())
            if (file.setIdFromVirtualPath(this->uploads[id].path + it.next().name)
                && file.getIdAccount() == client.getAccount().getId())
            {
                file.remove(true);
                Plugin::api().log().debug("File removed", Properties("idClient", this->uploads[id].idClient).add("idUpload", id).add("file", file.getFullPath()).toMap(), "Uploads", "cancel");
            }
        Plugin::api().log().info("Upload canceled", Properties("idClient", this->uploads[id].idClient).add("idUpload", id).toMap(), "Uploads", "cancel");
        if (!this->uploads[id].complete)
            Plugin::api().network().disconnect(this->uploads[id].idClient);
        this->uploads[id].complete = true;
    }
    // If the upload is not found, we remove the files asked by the client, provided that he is the owner
    else if (client.getRequest().getContent().getStorage() == LightBird::IContent::VARIANT
             && !(files = client.getRequest().getContent().getVariant()->toList()).isEmpty())
    {
        QListIterator<QVariant> it(files);
        while (it.hasNext())
            if (!it.next().toString().isEmpty() && file.setIdFromVirtualPath(it.peekPrevious().toByteArray())
                && file.getIdAccount() == client.getAccount().getId())
            {
                file.remove(true);
                Plugin::api().log().debug("File removed", Properties("idClient", client.getId()).add("idUpload", id).add("file", file.getFullPath()).toMap(), "Uploads", "cancel");
            }
    }
    this->mutex.unlock();
}

void        Uploads::onFinish(LightBird::IClient &client)
{
    QString id = client.getInformations().value("idUpload").toString();

    this->mutex.lock();
    if (this->uploads.contains(id) && this->uploads[id].idClient == client.getId())
    {
        if (this->uploads[id].file->isOpen())
            this->uploads[id].file->remove();
        this->uploads[id].finished = QDateTime::currentDateTime();
    }
    this->mutex.unlock();
}

void        Uploads::onDestroy(LightBird::IClient &client)
{
    QString id = client.getInformations().value("idUpload").toString();

    this->mutex.lock();
    if (this->uploads.contains(id) && this->uploads[id].idClient == client.getId())
    {
        if (this->uploads[id].file->isOpen())
            this->uploads[id].file->remove();
        this->uploads[id].finished = QDateTime::currentDateTime();
    }
    this->mutex.unlock();
}

void    Uploads::_removeCompleteUploads()
{
    this->mutex.lock();
    QMutableMapIterator<QString, Upload> it(this->uploads);
    while (it.hasNext())
    {
        it.next();
        if (it.value().finished.isValid() && it.value().finished.addSecs(REMOVE_COMPLETE_UPLOAD_TIME) < QDateTime::currentDateTime())
            it.remove();
    }
    this->mutex.unlock();
}

void        Uploads::_insert(LightBird::IClient &client, Upload &upload)
{
    File    file = upload.files.last();
    LightBird::TableFiles fileTable;
    LightBird::TableDirectories directory;

    // The file could not be opened
    if (!upload.file->isOpen())
        return ;
    // Adds the files to the database
    if (!directory.setIdFromVirtualPath(upload.path))
        directory.setId(directory.createVirtualPath(upload.path));
    if (fileTable.add(file.name, file.path, "other", directory.getId(), client.getAccount().getId()))
    {
        fileTable.setInformation("mime", file.contentType);
        this->identify << (upload.path + file.name);
        Plugin::api().log().info("File uploaded", Properties("idFile", fileTable.getId()).add("name", file.name).add("idClient", client.getId()).toMap(), "Uploads", "_insert");
    }
    else
    {
        upload.file->close();
        upload.file->remove(LightBird::TableFiles::getFilesPath() + file.path);
        Plugin::api().log().warning("Failed to add the uploaded file in the database", Properties("idDirectory", directory.getId()).add("path", file.path).add("name", file.name).add("idClient", client.getId()).toMap(), "Uploads", "_insert");
    }
}

void    Uploads::_clean(Upload &upload)
{
    upload.file->close();
    upload.header = true;
}

void    Uploads::_error(LightBird::IClient &client, Upload &upload, const QString &error)
{
    // If a file was downloading we remove it
    if (upload.file->isOpen())
        upload.file->remove();
    this->mutex.unlock();
    // Disconnects the client
    client.getRequest().setError(true);
    client.getRequest().getContent().clear();
    Plugin::api().network().disconnect(client.getId());
    Plugin::api().log().error("An error occured in the upload: " + error, Properties("idClient", client.getId()).add("idUpload", client.getInformations().value("idUpload")).toMap(), "Uploads", "_error");
}
