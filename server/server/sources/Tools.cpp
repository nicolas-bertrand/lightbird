#include <QDir>
#include <QFile>
#include <QUuid>

#include "Log.h"
#include "Tools.h"

// The maximum size of data copied each time
#define READ_WRITE_SIZE 1048576

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

QString Tools::cleanPath(const QString &path)
{
    return (QDir::cleanPath(QString(path).replace('\\', '/')));
}

QString Tools::createUuid()
{
    return (QUuid::createUuid().toString().remove(0, 1).remove(36, 1));
}
