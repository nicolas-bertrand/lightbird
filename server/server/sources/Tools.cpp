#include <QDir>
#include <QFile>

#include "Log.h"
#include "Tools.h"

bool        Tools::copy(const QString &sourceName, const QString &destinationName)
{
    QFile   source(sourceName);
    QFile   destination(destinationName);

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
    if (destination.write(source.readAll()) < 0)
    {
        Log::error("Cannot write on the destination file", Properties("source", sourceName).add("destination", destinationName), "Configurations", "copy");
        return (false);
    }
    source.close();
    destination.close();
    return (true);
}

QString Tools::cleanPath(const QString &path)
{
    return (QDir::cleanPath(QString(path).replace('\\', '/')));
}
