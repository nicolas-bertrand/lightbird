#include <QFileInfo>

#include "Configurations.h"
#include "Content.h"
#include "Log.h"

Content::Content(QObject *parent) : QObject(parent)
{
    this->storage = LightBird::IContent::BYTEARRAY;
    this->byteArray = new QByteArray();
    this->variant = NULL;
    this->file = NULL;
    this->temporaryFile = NULL;
    this->seek = 0;
    this->validFile = false;
}

Content::~Content()
{
    if (this->byteArray)
        delete this->byteArray;
    if (this->variant)
        delete this->variant;
    if (this->file)
        delete this->file;
    if (this->temporaryFile)
        delete this->temporaryFile;
}

LightBird::IContent::Storage Content::getStorage() const
{
    return (this->storage);
}

LightBird::IContent &Content::setStorage(LightBird::IContent::Storage storage, const QString &fileName)
{
    this->clear();
    this->storage = storage;
    if (this->storage == LightBird::IContent::BYTEARRAY && !this->byteArray)
        this->byteArray = new QByteArray();
    else if (this->storage == LightBird::IContent::VARIANT && !this->variant)
        this->variant = new QVariant();
    else if (this->storage == LightBird::IContent::FILE && !this->file)
    {
        this->file = new QFile(fileName);
        if (QFileInfo(*this->file).isFile())
            this->validFile = true;
    }
    else if (this->storage == LightBird::IContent::TEMPORARYFILE && !this->temporaryFile)
    {
        QString filePath = Configurations::instance()->get("temporaryPath");
        if (filePath.isEmpty())
            filePath = ".";
        this->temporaryFile = new QTemporaryFile(filePath + "/" + fileName);
        this->temporaryFile->open();
    }
    return (*this);
}

QByteArray  Content::getData(quint64 size)
{
    QByteArray  content;

    if (this->storage == LightBird::IContent::BYTEARRAY)
    {
        if (size == 0)
            content = *this->byteArray;
        else
        {
            content = this->byteArray->mid(this->seek, size);
            this->seek += content.size();
        }
    }
    else if (this->storage == LightBird::IContent::FILE)
    {
        if (this->file->open(QIODevice::ReadOnly))
        {
            if (size == 0)
                content = this->file->readAll();
            else
            {
                this->file->seek(this->seek);
                content.resize(size);
                content.resize(this->file->read(content.data(), size));
                this->seek += content.size();
            }
            this->file->close();
        }
        else
            LOG_WARNING("Unable to open the file", Properties("file", this->file->fileName()), "Content", "getContent");
    }
    else if (this->storage == LightBird::IContent::TEMPORARYFILE)
    {
        if (size == 0)
        {
            this->temporaryFile->seek(0);
            content = this->temporaryFile->readAll();
        }
        else
        {
            this->temporaryFile->seek(this->seek);
            content.resize(size);
            content.resize(this->temporaryFile->read(content.data(), size));
            this->seek += content.size();
        }
    }
    return (content);
}

void    Content::setData(const QByteArray &data, bool append)
{
    int wrote;

    if (!append)
        this->seek = 0;
    if (this->storage == LightBird::IContent::BYTEARRAY)
    {
        if (append)
            this->byteArray->append(data);
        else
            *this->byteArray = data;
    }
    else if (this->storage == LightBird::IContent::FILE)
    {
        if (append)
            this->file->open(QIODevice::WriteOnly | QIODevice::Append);
        else
            this->file->open(QIODevice::WriteOnly | QIODevice::Truncate);
        if ((wrote = this->file->write(data)) != data.size())
            LOG_WARNING("All data has not been written in the file", Properties("size", data.size())
                        .add("wrote", wrote).add("file", this->file->fileName()), "Content", "setContent");
        this->file->close();
    }
    else if (this->storage == LightBird::IContent::TEMPORARYFILE)
    {
        if (append)
            this->temporaryFile->seek(this->temporaryFile->size());
        else
            this->temporaryFile->resize(0);
        if ((wrote = this->temporaryFile->write(data)) != data.size())
            LOG_WARNING("All data has not been written in the temporary file", Properties("size", data.size())
                        .add("wrote", wrote).add("file", this->temporaryFile->fileName()), "Content", "setContent");
    }
}

QByteArray      *Content::getByteArray()
{
    if (this->storage == LightBird::IContent::BYTEARRAY)
        return (this->byteArray);
    return (NULL);
}

QVariant        *Content::getVariant()
{
    if (this->storage == LightBird::IContent::VARIANT)
        return (this->variant);
    return (NULL);
}

QFile           *Content::getFile()
{
    if (this->storage == LightBird::IContent::FILE)
        return (this->file);
    return (NULL);
}

QTemporaryFile  *Content::getTemporaryFile()
{
    if (this->storage == LightBird::IContent::TEMPORARYFILE)
        return (this->temporaryFile);
    return (NULL);
}

qint64  Content::size() const
{
    if (this->storage == LightBird::IContent::BYTEARRAY)
        return (this->byteArray->size());
    else if (this->storage == LightBird::IContent::FILE)
        return (this->validFile ? this->file->size() : 0);
    else if (this->storage == LightBird::IContent::TEMPORARYFILE)
        return (this->temporaryFile->size());
    return (0);
}

qint64  Content::getSeek() const
{
    return (this->seek);
}

void    Content::setSeek(qint64 position)
{
    if (position < 0)
        this->seek = 0;
    else
        this->seek = position;
}

void    Content::clear()
{
    this->storage = LightBird::IContent::BYTEARRAY;
    if (this->byteArray)
        this->byteArray->clear();
    if (this->variant)
    {
        delete this->variant;
        this->variant = NULL;
    }
    if (this->file)
    {
        delete this->file;
        this->file = NULL;
        this->validFile = false;
    }
    if (this->temporaryFile)
    {
        delete this->temporaryFile;
        this->temporaryFile = NULL;
    }
    this->seek = 0;
}
