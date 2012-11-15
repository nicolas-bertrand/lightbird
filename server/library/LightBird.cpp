#include <QDir>
#include <QFile>
#include <QUuid>
#include <QWaitCondition>

#include "Library.h"
#include "LightBird.h"
#include "Preview.h"

// The maximum number of bytes copied each time
static const unsigned READ_WRITE_SIZE = 1048576;

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

QString LightBird::getFilesPath(bool finalSlash)
{
    if (finalSlash)
        return (Library::configuration().get("filesPath") + "/");
    return (Library::configuration().get("filesPath"));
}

QString LightBird::preview(const QString &fileId, LightBird::IImage::Format format, unsigned int width, unsigned int height, unsigned int position)
{
    return (LightBird::Library::getPreview()->generate(fileId, format, width, height, position));
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

qint64  LightBird::currentMSecsSinceEpochUtc()
{
    QDateTime   current(QDateTime::currentDateTimeUtc());

    return (QDateTime(current.date(), current.time()).toMSecsSinceEpoch());
}
