#include <iostream>
#include <QtPlugin>

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
    this->path = this->_getNodeValue("path") + "/";
    if (this->path.size() == 1)
        this->path = "./";
    if ((this->name = this->_getNodeValue("file")).isEmpty())
        this->name = "server.log";
    this->maxSize = this->_toBytes(this->_getNodeValue("maxSize"));
    if ((this->expires = this->_getNodeValue("expires").toInt()) < 1)
        this->expires = 30;
    if ((this->maxNbOfFile = this->_getNodeValue("maxNbOfFile").toInt()) < 1)
    {
        this->api->log().warning("Invalid maxNbOfFile (" + QString::number(this->maxNbOfFile) + "). It should be greater than 0.", "Files", "onLoad");
        this->maxNbOfFile = 10;
    }
    this->display = true;
    if (this->_getNodeValue("display") == "false")
        this->display = false;
    if ((this->delay = this->_getNodeValue("delay").toInt()) < 1)
        this->delay = 1;

    // Display the configuration of the plugin
    properties["path"] = this->path;
    properties["file"] = this->name;
    properties["maxSize"] = QString::number(this->maxSize);
    properties["expires"] = QString::number(this->expires);
    properties["maxNbOfFile"] = QString::number(this->maxNbOfFile);
    properties["display"] = "false";
    if (this->display == true)
        properties["display"] = "true";
    this->api->log().debug("Loading Files", properties, "Files", "onLoad");

    // Map the names of the log levels
    this->levels[LightBird::ILogs::FATAL] = "Fatal";
    this->levels[LightBird::ILogs::ERROR] = "Error";
    this->levels[LightBird::ILogs::WARNING] = "Warning";
    this->levels[LightBird::ILogs::INFO] = "Info";
    this->levels[LightBird::ILogs::DEBUG] = "Debug";
    this->levels[LightBird::ILogs::TRACE] = "Trace";

    // Creates the log timer
    this->api->timers().setTimer("writeLog", this->delay * 1000);

    // Create the log file
    return (this->_createLogFile());
}

void    Files::onUnload()
{
    // Write the remaining logs
    this->timer("writeLog");
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

void        Files::log(LightBird::ILogs::level level, const QDateTime &date, const QString &message,
                       const QMap<QString, QString> &properties, const QString &thread,
                       const QString &plugin, const QString &object, const QString &method)
{
    QString log;

    log = date.toString("[yyyy/MM/dd hh:mm:ss:zzz]");
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
    this->mutex.lock();
    this->buffer << log;
    this->mutex.unlock();
}

void            Files::timer(const QString &name)
{
    QByteArray  log;

    // Write the logs buffered in the file
    if (name == "writeLog")
    {
        this->mutex.lock();
        if(!this->file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text))
        {
            std::cerr << "Unable to open the log file (from the plugin Files::log)." << std::endl;
            this->mutex.lock();
            return ;
        }
        QStringListIterator it(this->buffer);
        while (it.hasNext())
        {
            if (log.size() + it.peekNext().size() + this->file.size() > this->maxSize)
            {
                this->file.write(log);
                log.clear();
                if (!this->_manageFiles())
                    return this->mutex.unlock();
            }
            log += it.next();
        }
        this->file.write(log);
        this->file.close();
        this->buffer.clear();
        this->mutex.unlock();
    }
}

QString Files::_getNodeValue(const QString &nodeName)
{
    // Return the node of the configuration of the server if it exists
    if (this->api->configuration(false).count("log/" + nodeName))
        return (this->api->configuration(false).get("log/" + nodeName));
    // Otherwise return the node of the configuration of the plugin
    return (this->api->configuration(true).get(nodeName));
}

unsigned        Files::_toBytes(const QString &str)
{
    char        type;
    unsigned    bytes;
    QString     string = str;

    if (string.remove(' ').isEmpty())
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

    if (!this->directory.exists() && !this->directory.mkpath(this->path))
    {
        std::cerr << "Unable to create the log directory (from the plugin Files::_createLogFile)." << std::endl;
        return (false);
    }
    this->file.setFileName(this->path + this->name);
    if(!(this->file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)))
    {
        std::cerr << "Unable to create the log file (from the plugin Files::_createLogFile)." << std::endl;
        return (false);
    }
    this->file.close();
    return (true);
}

bool                            Files::_manageFiles()
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

    // Rename the file
    date = QDateTime::currentDateTime();
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
            this->api->log().warning("Cannot rename the log file to archive it", properties, "Files", "_manageFiles");
        }
    }
    else
    {
        properties["archive"] = this->file.fileName();
        this->api->log().trace("Log archive file created", properties, "Files", "_manageFiles");
    }
    this->file.close();
    this->_createLogFile();
    if(!this->file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text))
    {
        std::cerr << "Unable to open the log file (from the plugin Files::_manageFiles)." << std::endl;
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
            this->api->log().warning("Can't remove the log archive file", properties, "Files", "_manageFiles");
        else
            this->api->log().trace("Log archive file removed", properties, "Files", "_manageFiles");
        files.erase(files.begin());
    }

    // Deletes files created from more than this->expires days
    deadLine = QDateTime::currentDateTime().addDays(-this->expires);
    flag = true;
    while (flag == true && files.size() > 0)
    {
        flag = false;
        if (files.begin().key() < deadLine)
        {
            properties["file"] = this->path + files.begin().value();
            if (!QFile::remove(this->path + files.begin().value()))
                this->api->log().warning("Can't remove the expired log archive file", properties, "Files", "_manageFiles");
            else
                this->api->log().trace("Expired log archive file removed", properties, "Files", "_manageFiles");
            flag = true;
        }
        files.erase(files.begin());
    }
    return (true);
}

QString Files::_mapToString(const QMap<QString, QString> &properties)
{
    QMapIterator<QString, QString>  it(properties);
    QString                         result;

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
