#include <QCryptographicHash>
#include <QFileInfo>

#include "Identify.h"
#include "IMime.h"
#include "Library.h"
#include "LightBird.h"

Identify::Identify() : thread(NULL)
{
    this->mimeDocument.push_back("text/");
    this->mimeDocument.push_back("pdf");
    this->mimeDocument.push_back("excel");
    this->mimeDocument.push_back("msword");
    this->mimeDocument.push_back("powerpoint");
    this->mimeDocument.push_back("opendocument");
    this->mimeDocument.push_back("postscript");
    this->mimeDocument.push_back("xml");
    this->mimeDocument.push_back("json");
    this->typeString.insert(LightBird::IIdentify::AUDIO, "audio");
    this->typeString.insert(LightBird::IIdentify::DOCUMENT, "document");
    this->typeString.insert(LightBird::IIdentify::IMAGE, "image");
    this->typeString.insert(LightBird::IIdentify::OTHER, "other");
    this->typeString.insert(LightBird::IIdentify::VIDEO, "video");
    this->maxSizeHash = LightBird::Library::configuration().get("hashSizeLimit").toLongLong();
    if (!LightBird::Library::configuration().count("hashSizeLimit"))
        this->maxSizeHash = -1;
}

Identify::~Identify()
{
    Thread  *thread;

    // Waits for the thread to finish
    this->mutex.lock();
    if ((thread = this->thread))
    {
        this->mutex.unlock();
        thread->wait();
        this->mutex.lock();
    }
    this->mutex.unlock();
}

void    Identify::identify(const QString &file, LightBird::IIdentify::Information *information)
{
    // Identifies the file directly
    if (information)
    {
        *information = this->_identify(file);
        return ;
    }
    // Identifies the file via the thread
    this->mutex.lock();
    this->files << file;
    // Starts the thread
    if (!this->thread)
    {
        this->thread = new Thread();
        this->thread->start();
    }
    this->mutex.unlock();
}

void    Identify::Thread::run()
{
    Identify              *instance = LightBird::Library::getIdentify();
    QStringList           files;
    LightBird::TableFiles file;
    LightBird::IIdentify::Information information;

    // Gets the files to identify
    instance->mutex.lock();
    (files = instance->files).removeDuplicates();
    instance->files.clear();
    instance->mutex.unlock();
    // While there are files to identify
    while (!files.isEmpty())
    {
        QStringListIterator it(files);
        while (it.hasNext())
            // Identify the file
            if (file.setId(it.next()))
            {
                information = instance->_identify(file.getFullPath());
                file.setType(instance->typeString.value(information.type));
                file.setInformations(information.data);
                information.data.clear();
            }
        files.clear();
        // If some files have been added in the meantime, we continue the identification
        instance->mutex.lock();
        (files = instance->files).removeDuplicates();
        if (!files.isEmpty())
            instance->files.clear();
        // No more file to identify
        else
        {
            instance->thread = NULL;
            this->deleteLater();
        }
        instance->mutex.unlock();
    }
}

LightBird::IIdentify::Information Identify::_identify(const QString &file)
{
    Info            result;
    Info            tmp;
    QList<void *>   extensions;
    QString         mime;
    QMap<LightBird::IIdentify::Type, QVariantMap> info;

    result.type = LightBird::IIdentify::OTHER;
    // Checks that the file exists
    if (!QFileInfo(file).isFile())
        return (result);
    // Gets the extensions that implements IIdentify
    QListIterator<void *> it(extensions = LightBird::Library::extension().get("IIdentify"));
    while (it.hasNext())
    {
        tmp.data.clear();
        tmp.type = LightBird::IIdentify::OTHER;
        // If the plugin could identify the file, add it to the map
        if (static_cast<LightBird::IIdentify *>(it.peekNext())->identify(file, tmp))
            info.insertMulti(tmp.type, tmp.data);
        it.next();
    }
    // Releases the extensions
    LightBird::Library::extension().release(extensions);
    // Puts the data gathered in the result
    if (info.size() > 0)
        this->_identify(info, result);
    // Gets the size, the extension and the mime of the file
    result.data.insert("size", QFileInfo(file).size());
    if (file.contains("."))
    {
        result.data.insert("extension", file.right(file.size() - file.lastIndexOf(".") - 1));
        extensions = LightBird::Library::extension().get("IMime");
        if (!extensions.isEmpty() && !(mime = static_cast<LightBird::IMime *>(extensions.first())->getMime(file)).isEmpty())
            result.data.insert("mime", mime);
        LightBird::Library::extension().release(extensions);
    }
    // Determines if the file is a document
    if (result.type == LightBird::IIdentify::OTHER)
        this->_document(result);
    // If the type is still other, we try to guess it using the MIME
    if (result.type == LightBird::IIdentify::OTHER)
        this->_typeFromMime(result);
    // Computes the hashes of the file
    this->_hash(file, result);
    // Debug
    /**this->api.log().debug("Type: " + QString::number(result.type));
    QMapIterator<QString, QVariant> i(result.data);
    while (i.hasNext())
    {
        i.next();
        this->api.log().debug(i.key() + QString(17 - i.key().size(), ' ') + "=  " + i.value().toString());
    }//*/
    return (result);
}

void    Identify::_identify(QMap<LightBird::IIdentify::Type, QVariantMap> info, Info &result)
{
    if (this->_add(LightBird::IIdentify::DOCUMENT, info, result))
        return ;
    if (this->_add(LightBird::IIdentify::IMAGE, info, result))
        return ;
    if (this->_add(LightBird::IIdentify::VIDEO, info, result))
        return ;
    if (this->_add(LightBird::IIdentify::AUDIO, info, result))
        return ;
}

bool    Identify::_add(LightBird::IIdentify::Type type, QMap<LightBird::IIdentify::Type, QVariantMap> info, Info &result)
{
    if (info.contains(type))
    {
        QListIterator<QVariantMap> i(info.values(type));
        while (i.hasNext())
        {
            result.data = i.peekNext();
            i.next();
        }
        if (!result.data.isEmpty() || type == LightBird::IIdentify::DOCUMENT)
        {
            result.type = type;
            return (true);
        }
    }
    return (false);
}

void        Identify::_document(Info &result)
{
    QString mime = result.data.value("mime").toString();

    QStringListIterator it(this->mimeDocument);
    while (it.hasNext())
        if (mime.contains(it.next()))
        {
            result.type = LightBird::IIdentify::DOCUMENT;
            return ;
        }
}

void        Identify::_typeFromMime(Info &result)
{
    QString mime = result.data.value("mime").toString();

    if (mime.startsWith("image"))
        result.type = LightBird::IIdentify::IMAGE;
    else if (mime.startsWith("audio"))
        result.type = LightBird::IIdentify::AUDIO;
    else if (mime.startsWith("video"))
        result.type = LightBird::IIdentify::VIDEO;
}

void    Identify::_hash(const QString &fileName, Info &result)
{
    QCryptographicHash  md5(QCryptographicHash::Md5);
    QCryptographicHash  sha1(QCryptographicHash::Sha1);
    QFile               file(fileName);
    QByteArray          data;

    if ((this->maxSizeHash >= 0 && (this->maxSizeHash == 0 || this->maxSizeHash < QFileInfo(fileName).size()))
        || !file.open(QIODevice::ReadOnly))
        return ;
    do
    {
        data.clear();
        data = file.read(READ_FILE_SIZE);
        md5.addData(data);
        sha1.addData(data);
    }
    while (data.size() == READ_FILE_SIZE);
    result.data.insert("md5", QString(md5.result().toHex()));
    result.data.insert("sha1", QString(sha1.result().toHex()));
}
