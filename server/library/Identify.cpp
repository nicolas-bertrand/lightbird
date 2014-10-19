#include <QCoreApplication>
#include <QCryptographicHash>
#include <QFileInfo>

#include "Identify.h"
#include "IMime.h"
#include "Library.h"
#include "LightBird.h"

Identify::Identify()
    : identifyThread(NULL)
    , hashThread(NULL)
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
    this->maxSizeHash = LightBird::c().hashSizeLimit;
}

Identify::~Identify()
{
    Thread  *identifyThread;
    Thread  *hashThread;

    // Waits for the thread to finish
    this->mutex.lock();
    if ((identifyThread = this->identifyThread))
    {
        this->mutex.unlock();
        identifyThread->wait();
        this->mutex.lock();
    }
    if ((hashThread = this->hashThread))
    {
        this->mutex.unlock();
        hashThread->wait();
        this->mutex.lock();
    }
    this->mutex.unlock();
}

void    Identify::identify(const QString &fileId)
{
    // Identifies the file via a thread
    this->mutex.lock();
    // Starts the identification thread
    if (!this->identifyThread)
    {
        this->identifyThread = new Thread();
        this->identifyThread->files << fileId;
        this->identifyThread->thread = &this->identifyThread;
        this->identifyThread->method = &Identify::_identifyThread;
        this->identifyThread->moveToThread(this->identifyThread);
        QObject::connect(this->identifyThread, SIGNAL(finished()), this, SLOT(finished()));
        this->identifyThread->start();
    }
    else
        this->identifyThread->files << fileId;
    // Starts the hash thread
    if (!this->hashThread)
    {
        this->hashThread = new Thread();
        this->hashThread->files << fileId;
        this->hashThread->thread = &this->hashThread;
        this->hashThread->method = &Identify::_hashThread;
        this->hashThread->moveToThread(this->hashThread);
        QObject::connect(this->hashThread, SIGNAL(finished()), this, SLOT(finished()));
        this->hashThread->start();
    }
    else
        this->hashThread->files << fileId;
    this->mutex.unlock();
}

void    Identify::identify(const QString &filePath, LightBird::IIdentify::Information &information)
{
    // Identifies the file directly
    information = this->_identify(filePath, filePath, true);
}

void    Identify::finished()
{
    Identify::Thread    *thread;

    if ((thread = dynamic_cast<Identify::Thread *>(this->sender())))
        delete thread;
}

void    Identify::Thread::run()
{
    Identify              *instance = LightBird::Library::getIdentify();
    QStringList           files;
    LightBird::TableFiles file;
    Identify::Info        information;

    // Gets the files to identify or hash
    instance->mutex.lock();
    (files = this->files).removeDuplicates();
    this->files.clear();
    instance->mutex.unlock();
    // While there are files to process
    while (!files.isEmpty())
    {
        QStringListIterator it(files);
        while (it.hasNext())
            // Identify or hash the file
            if (file.setId(it.next()))
            {
                (instance->*(this->method))(file, information);
                file.setInformations(information.data);
                information.data.clear();
            }
        files.clear();
        // If some files have been added in the meantime, we continue the processing
        instance->mutex.lock();
        (files = this->files).removeDuplicates();
        if (!files.isEmpty())
            this->files.clear();
        // No more file to process
        else
            *(this->thread) = NULL;
        instance->mutex.unlock();
    }
}

void    Identify::_identifyThread(LightBird::TableFiles &file, Identify::Info &information)
{
    information = this->_identify(file.getFullPath(), file.getName(), false);
    file.setType(this->typeString.value(information.type));
}

void    Identify::_hashThread(LightBird::TableFiles &file, Identify::Info &information)
{
    this->_hash(file.getFullPath(), information);
}

Identify::Info  Identify::_identify(const QString &file, const QString &fileName, bool computeHash)
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
        // If the plugin could identify the file, adds it to the map
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
    if (fileName.contains("."))
    {
        result.data.insert("extension", fileName.right(fileName.size() - fileName.lastIndexOf(".") - 1));
        extensions = LightBird::Library::extension().get("IMime");
        if (!extensions.isEmpty() && !(mime = static_cast<LightBird::IMime *>(extensions.first())->getMime(fileName)).isEmpty())
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
    if (computeHash)
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
            QMapIterator<QString, QVariant> j(i.next());
            while (j.hasNext())
                if (!j.next().value().toString().isEmpty())
                    result.data.insert(j.peekPrevious().key(), j.peekPrevious().value());
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
