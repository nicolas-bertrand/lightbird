#include <QtPlugin>
#include <iostream>

#include "Files.h"

Files::Files()
{
}

Files::~Files()
{
}

bool    Files::onLoad(LightBird::IApi *api)
{
    QMap<QString, QString>  properties;

    this->api = api;
    this->path = this->_getNodeValue("log/path") + "/";
    if (this->path.size() == 1)
        this->path = "./";
    if ((this->name = this->_getNodeValue("log/file")).isEmpty())
        this->name = "server.log";
    this->maxSize = this->_toByte(this->_getNodeValue("log/maxSize"));
    if ((this->validityPeriod = this->_getNodeValue("log/validityPeriod").toInt()) < 1)
        this->validityPeriod = 30;
    if ((this->maxNbOfFile = this->_getNodeValue("log/maxNbOfFile").toInt()) < 1)
    {
        this->api->log().warning(tr("Invalid maxNbOfFile (") + QString::number(this->maxNbOfFile) + tr("). It should be greater than 0."), "Files", "onLoad");
        this->maxNbOfFile = 128;
    }
    this->display = true;
    if (this->_getNodeValue("log/display") == "false")
        this->display = false;
    this->directory.setPath(this->path);

    // Display the configuration of the plugin
    properties["path"] = this->path;
    properties["file"] = this->name;
    properties["maxSize"] = QString::number(this->maxSize);
    properties["validityPeriod"] = QString::number(this->validityPeriod);
    properties["maxNbOfFile"] = QString::number(this->maxNbOfFile);
    properties["display"] = "false";
    if (this->display == true)
        properties["display"] = "true";
    this->api->log().debug(tr("Loading Files"), properties, "Files", "onLoad");

    // Map the names of the log levels
    this->levels[LightBird::ILogs::FATAL] = tr("Fatal");
    this->levels[LightBird::ILogs::ERROR] = tr("Error");
    this->levels[LightBird::ILogs::WARNING] = tr("Warning");
    this->levels[LightBird::ILogs::INFO] = tr("Info");
    this->levels[LightBird::ILogs::DEBUG] = tr("Debug");
    this->levels[LightBird::ILogs::TRACE] = tr("Trace");

    // Create the log file
    return (this->_createLogFile());
}

void    Files::onUnload()
{
}

bool    Files::onInstall(LightBird::IApi *)
{
    return (true);
}

void    Files::onUninstall(LightBird::IApi *)
{
}

void    Files::getMetadata(LightBird::IMetadata &metadata) const
{
    metadata.name = "Logs Files";
    metadata.brief = "Write the logs entries into files.";
    metadata.description = "Saves the logs entries into files, and manage them.";
    metadata.autor = "LightBird team";
    metadata.site = "lightbird.cc";
    metadata.email = "team@lightbird.cc";
    metadata.version = "1.0";
    metadata.licence = "LGPL";
}

void    Files::log(LightBird::ILogs::level level, const QDateTime &date, const QString &message,
                     const QMap<QString, QString> &properties, const QString &thread,
                     const QString &plugin, const QString &object, const QString &method)
{
    QByteArray  log;

    if(!this->file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text))
    {
        std::cerr << tr("Unable to open the log file (from the plugin Files::log).").toStdString() << std::endl;
        return ;
    }
    if (this->file.size() > this->maxSize && plugin != this->api->getId())
        if (!this->_manageFiles())
            return ;
    log += " [" + this->levels[level] + "]";
    log += " [" + thread;
    if (!plugin.isEmpty())
        log += "::" + plugin;
    if (!object.isEmpty())
        log += "::" + object;
    if (!method.isEmpty())
        log += "::" + method;
    log += "]";
    if (properties.size())
        log += " [" + this->_mapToString(properties) + "]";
    log += " : " + message + "\n";
    this->file.write(date.toString("[yyyy/MM/dd hh:mm:ss:zzz]").toAscii() + log);
    this->file.close();
}

QString Files::_getNodeValue(const QString &nodeName)
{
    // Return the node of the configuration of the server if it exists
    if (this->api->configuration(false).count(nodeName))
        return (this->api->configuration(false).get(nodeName));
    // Otherwise return the node of the configuration of the plugin
    return (this->api->configuration(true).get(nodeName));
}

int     Files::_toByte(const QString &string)
{
    char    type;
    int     bytes;

    if (string.isEmpty())
        return (1024);
    if (string.at(string.size() - 1).isLetter())
    {
        bytes = string.left(string.size() - 1).toInt();
        type = string.at(string.size() - 1).toUpper().toAscii();
        if (type == 'K')
            bytes *= 1024;
        else if (type == 'M')
            bytes *= 1024 * 1024;
        else if (type == 'G')
            bytes *= 1024 * 1024 * 1024;
    }
    else
        bytes = string.toInt();
    if (bytes <= 1024)
        bytes = 1024;
    return (bytes);
}

bool        Files::_createLogFile()
{
    QDir    directory;

    if (!this->directory.exists() && !directory.mkpath(this->path))
    {
        std::cerr << tr("Unable to create the log directory (from the plugin Files::_createLogFile).").toStdString() << std::endl;
        return (false);
    }
    this->file.setFileName(this->path + this->name);
    if(!(this->file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)))
    {
        std::cerr << tr("Unable to create the log file (from the plugin Files::_createLogFile).").toStdString() << std::endl;
        return (false);
    }
    this->file.close();
    return (true);
}

bool    Files::_manageFiles()
{
    QDateTime                   date;
    QString                     name;
    QString                     extension;
    QStringList                 filesNames;
    QStringList::iterator       it, end;
    QMap<QDateTime, QString>    files;
    QDateTime                   deadLine;
    bool                        flag;
    QMap<QString, QString>      properties;

    date = QDateTime::currentDateTime();
    // If the log file has exceed the max size
    if (this->file.size() > this->maxSize)
    {
        // Rename the file
        name = this->path + this->name.left(this->name.indexOf(".") + 1);
        name += date.toString("yyyy-MM-dd hh-mm-ss");
        extension = this->name.right(this->name.size() - this->name.indexOf("."));
        name += extension;
        if (!this->file.rename(name))
        {
            properties["source"] = this->file.fileName();
            properties["destination"] = name;
            if (date.toString("yyyy-MM-dd hh-mm-ss") != lastError)
            {
                lastError = date.toString("yyyy-MM-dd hh-mm-ss");
                this->api->log().warning(tr("Cannot rename the log file to archive it"), properties, "Files", "_manageFiles");
            }
        }
        else
        {
            properties["archive"] = this->file.fileName();
            this->api->log().trace(tr("Log archive file created"), properties, "Files", "_manageFiles");
        }
        this->file.close();
        this->_createLogFile();
        if(!this->file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text))
        {
            std::cerr << tr("Unable to open the log file (from the plugin Files::_manageFiles).").toStdString() << std::endl;
            return (false);
        }

        // Sort the files per date
        filesNames = this->directory.entryList(QDir::Files, QDir::Name);
        name = this->name.left(this->name.indexOf(".") + 1);
        for (it = filesNames.begin(), end = filesNames.end(); it != end; ++it)
            if (*it != this->name)
            {
                date = QDateTime::fromString(it->mid(name.size(), 19), "yyyy-MM-dd hh-mm-ss");
                files[date] = *it;
            }

        // Deletes the older files when they are more than this->maxNbOfFiles in the log directory
        properties.clear();
        while (files.size() > this->maxNbOfFile - 1 && files.size() > 0)
        {
            properties["file"] = this->path + files.begin().value();
            if (!QFile::remove(this->path + files.begin().value()))
                this->api->log().warning(tr("Can't remove the log archive file"), properties, "Files", "_manageFiles");
            else
                this->api->log().trace(tr("Log archive file removed"), properties, "Files", "_manageFiles");
            files.erase(files.begin());
        }

        // Deletes files created from more than this->validityPeriod days
        deadLine = QDateTime::currentDateTime().addDays(-this->validityPeriod);
        flag = true;
        while (flag == true && files.size() > 0)
        {
            flag = false;
            if (files.begin().key() < deadLine)
            {
                properties["file"] = this->path + files.begin().value();
                if (!QFile::remove(this->path + files.begin().value()))
                    this->api->log().warning(tr("Can't remove the expired log archive file"), properties, "Files", "_manageFiles");
                else
                    this->api->log().trace(tr("Expired log archive file removed"), properties, "Files", "_manageFiles");
                flag = true;
            }
            files.erase(files.begin());
        }
    }
    return (true);
}

QString Files::_mapToString(const QMap<QString, QString> &properties)
{
    QMapIterator<QString, QString>    it(properties);
    QString                             result;

    while (it.hasNext())
    {
        it.next();
        if (result.size() > 0)
            result += ", ";
        result += it.key() + ":" + it.value();
    }
    return (result);
}

Q_EXPORT_PLUGIN2(Files, Files)
