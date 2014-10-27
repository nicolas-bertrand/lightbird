#include <QCryptographicHash>
#include <QDir>
#include <QFile>
#include <QImageWriter>
#include <QUuid>
#include <QWaitCondition>

#include "Identify.h"
#include "FilesExtensions.h"
#include "Library.h"
#include "LightBird.h"
#include "Preview.h"

// The maximum number of bytes copied each time
static const unsigned READ_WRITE_SIZE = 1048576;

QString LightBird::addressToString(void *address)
{
    return (QString::number((quint64)address, 16).toLower());
}

bool    LightBird::copy(const QString &sourceName, const QString &destinationName)
{
    QFile       source(sourceName);
    QFile       destination(destinationName);
    QByteArray  data;

    if (source.open(QIODevice::ReadOnly) == false)
    {
        LightBird::Library::log().error("Cannot open the source file", Properties("source", sourceName).add("destination", destinationName).toMap(), "Configurations", "copy");
        return (false);
    }
    if (destination.open(QIODevice::WriteOnly | QIODevice::Truncate) == false)
    {
        LightBird::Library::log().error("Cannot open the destination file", Properties("source", sourceName).add("destination", destinationName).toMap(), "Configurations", "copy");
        return (false);
    }
    while ((data = source.read(READ_WRITE_SIZE)).isEmpty() == false)
        if (destination.write(data) < 0)
        {
            LightBird::Library::log().error("Cannot write on the destination file", Properties("source", sourceName).add("destination", destinationName).toMap(), "Configurations", "copy");
            return (false);
        }
    return (true);
}

QString LightBird::cleanPath(const QString &p, bool removeFirstSlash)
{
    QString result = QDir::cleanPath(QString(p).replace('\\', '/')).replace("//", "/").remove('~');
    if (removeFirstSlash && result.startsWith('/'))
        result.remove(0, 1);
    return (result);
}

QString LightBird::fileTypeToString(IIdentify::Type type)
{
    if (type == IIdentify::AUDIO)
        return "audio";
    else if (type == IIdentify::DOCUMENT)
        return "document";
    else if (type == IIdentify::IMAGE)
        return "image";
    else if (type == IIdentify::VIDEO)
        return "video";
    else
        return "other";
}

void    LightBird::identify(const QString &fileId)
{
    LightBird::Library::getIdentify()->identify(fileId);
}

void    LightBird::identify(const QString &filePath, LightBird::IIdentify::Information &information)
{
    LightBird::Library::getIdentify()->identify(filePath, information);
}

bool    LightBird::isValidName(const QString &objectName)
{
    if (objectName == "." || objectName == ".." || objectName.contains('/'))
        return (false);
    return (true);
}

QString LightBird::createUuid()
{
    return (QUuid::createUuid().toString().remove(0, 1).remove(36, 1));
}

QString LightBird::getFileMime(const QString &fileName)
{
    return (Library::getFilesExtensions()->getFileMime(fileName));
}

LightBird::IIdentify::Type LightBird::getFileType(const QString &fileName)
{
    return (Library::getFilesExtensions()->getFileType(fileName));
}

QString LightBird::getFilesPath(bool finalSlash)
{
    if (finalSlash)
        return (LightBird::c().filesPath + "/");
    return (LightBird::c().filesPath);
}

QString LightBird::getImageExtension(LightBird::IImage::Format format, bool dot)
{
    return ((dot ? "." : "") + LightBird::Library::getImageExtensions().value(format));
}

QList<ushort> LightBird::parsePorts(QString ports, int max)
{
    QList<ushort> result;
    int           from, to;

    // Replaces the other characters by a space
    for (int i = ports.size() - 1; i >= 0; --i)
        if (!ports[i].isDigit() && ports[i] != '-' && ports[i] != ' ')
            ports[i] = ' ';
    // Parses the ports
    QStringListIterator it(ports.split(' '));
    while (it.hasNext() && result.size() < max)
    {
        from = it.next().split('-').first().toUInt();
        to = it.peekPrevious().split('-').last().toUInt();
        if (from)
            for (int port = from; port <= to && result.size() < max; ++port)
                if (port && !result.contains(port))
                    result << port;
    }
    return (result);
}

QString LightBird::preview(const QString &fileId, LightBird::IImage::Format format, unsigned int width, unsigned int height, unsigned int position, float quality)
{
    return (LightBird::Library::getPreview()->generate(fileId, format, width, height, position, quality));
}

bool    LightBird::saveImage(QImage &image, QString &fileName, LightBird::IImage::Format format, float quality)
{
    QList<QByteArray> supported = QImageWriter::supportedImageFormats();
    QString extension = LightBird::getImageExtension(format);

    if (!supported.contains(extension.toLatin1()))
        return (false);
    if (!fileName.contains(QRegExp("\\." + extension + "$")))
        fileName.append("." + extension);
    if (quality >= 0)
        quality = qRound(quality * 100);
    return (image.save(fileName, extension.toLatin1().data(), (int)quality));
}

QByteArray  LightBird::sha256(const QByteArray &data)
{
    return (QCryptographicHash::hash(data, QCryptographicHash::Sha256).toHex());
}

QByteArray LightBird::simplify(QByteArray data, char replace, quint64 maxSize)
{
    if (maxSize)
        data = data.left(maxSize);
    int s = data.size();
    for (int i = 0; i < s; ++i)
        if (data.data()[i] < 32 || data.data()[i] > 126)
            data.data()[i] = replace;
    return (data);
}

void    LightBird::sleep(unsigned long time)
{
    QWaitCondition  wait;
    QMutex          mutex;

    mutex.lock();
    wait.wait(&mutex, time);
    mutex.unlock();
}

void    LightBird::srand()
{
    ::qsrand((unsigned int)(QDateTime::currentDateTime().toMSecsSinceEpoch() / 1000));
}

quint64 LightBird::stringToBytes(const QString &str)
{
    char    type;
    quint64 bytes;
    QString string = str;

    string.remove(' ');
    if (string.at(string.size() - 1).isLetter())
    {
        bytes = string.left(string.size() - 1).toInt();
        type = string.at(string.size() - 1).toUpper().toLatin1();
        if (type == 'K')
            bytes *= 1024;
        else if (type == 'M')
            bytes *= 1024 * 1024;
        else if (type == 'G')
            bytes *= 1024 * 1024 * 1024;
        else if (type == 'T')
            bytes *= quint64(1024) * 1024 * 1024 * 1024;
        else
            bytes = string.toULongLong();
    }
    else
        bytes = string.toULongLong();
    return (bytes);
}

qint64  LightBird::currentMSecsSinceEpochUtc()
{
    QDateTime   current(QDateTime::currentDateTimeUtc());

    return (QDateTime(current.date(), current.time()).toMSecsSinceEpoch());
}
