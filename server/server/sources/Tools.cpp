#include <QDir>
#include <QFile>
#include <QUuid>

#include "Log.h"
#include "Tools.h"

// The maximum size of data copied each time
static const unsigned READ_WRITE_SIZE = 1048576;

bool            Tools::copy(const QString &sourceName, const QString &destinationName)
{
    QFile       source(sourceName);
    QFile       destination(destinationName);
    QByteArray  data;

    if (source.open(QIODevice::ReadOnly) == false)
    {
        Log::error("Cannot open the source file", Properties("source", sourceName).add("destination", destinationName), "Configurations", "copy");
        return (false);
    }
    if (destination.open(QIODevice::WriteOnly | QIODevice::Truncate) == false)
    {
        Log::error("Cannot open the destination file", Properties("source", sourceName).add("destination", destinationName), "Configurations", "copy");
        return (false);
    }
    while ((data = source.read(READ_WRITE_SIZE)).isEmpty() == false)
        if (destination.write(data) < 0)
        {
            Log::error("Cannot write on the destination file", Properties("source", sourceName).add("destination", destinationName), "Configurations", "copy");
            return (false);
        }
    return (true);
}

QString     Tools::cleanPath(const QString &p)
{
    return (QDir::cleanPath(QString(p).replace('\\', '/')).replace("//", "/"));
}

QString Tools::createUuid()
{
    return (QUuid::createUuid().toString().remove(0, 1).remove(36, 1));
}

QByteArray Tools::simplify(QByteArray data, char replace, quint64 maxSize)
{
    if (maxSize)
        data = data.left(maxSize);
    int s = data.size();
    for (int i = 0; i < s; ++i)
        if (data.data()[i] < 32 || data.data()[i] > 126)
            data.data()[i] = replace;
    return (data);
}
