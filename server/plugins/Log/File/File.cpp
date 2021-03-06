#include <iostream>
#include <QDateTime>

#include "LightBird.h"
#include "File.h"

File::File()
{
}

File::~File()
{
}

bool    File::onLoad(LightBird::IApi *api)
{
    QMap<QString, QString>  properties;

    this->api = api;
    this->path = LightBird::c().log.path + "/";
    this->directory.setPath(this->path);
    this->name = LightBird::c().log.file;
    if ((this->maxSize = LightBird::c().log.maxSize) < 1024)
        this->maxSize = 1024 * 1024;
    if ((this->expires = LightBird::c().log.expires) < 1)
        this->expires = 30;
    if ((this->maxNbOfFile = LightBird::c().log.maxNbOfFile) < 1)
    {
        LOG_WARNING("Invalid maxNbOfFile (" + QString::number(this->maxNbOfFile) + "). It should be greater than 0.", "File", "onLoad");
        this->maxNbOfFile = 10;
    }
    this->display = LightBird::c().log.display;

    // Display the configuration of the plugin
    properties["path"] = this->path;
    properties["file"] = this->name;
    properties["maxSize"] = QString::number(this->maxSize);
    properties["expires"] = QString::number(this->expires);
    properties["maxNbOfFile"] = QString::number(this->maxNbOfFile);
    properties["display"] = "false";
    if (this->display == true)
        properties["display"] = "true";
    LOG_DEBUG("Loading Log/File", properties, "File", "onLoad");

    // Map the names of the log levels
    this->levels[LightBird::ILogs::FATAL] = "Fatal";
    this->levels[LightBird::ILogs::ERROR] = "Error";
    this->levels[LightBird::ILogs::WARNING] = "Warning";
    this->levels[LightBird::ILogs::INFO] = "Info";
    this->levels[LightBird::ILogs::DEBUG] = "Debug";
    this->levels[LightBird::ILogs::TRACE] = "Trace";

    // Creates the log timer if it does not exist
    if (!this->api->timers().getTimer("writeLog"))
        this->api->timers().setTimer("writeLog", 1000);

    // Create the log file
    return (this->_createLogFile());
}

void    File::onUnload()
{
    // Write the remaining logs
    this->timer("writeLog");
}

bool    File::onInstall(LightBird::IApi *)
{
    return (true);
}

void    File::onUninstall(LightBird::IApi *)
{
}

void    File::getMetadata(LightBird::IMetadata &metadata) const
{
    metadata.name = "Log File";
    metadata.brief = "Write the logs entries into files.";
    metadata.description = "Saves the logs entries into files, and manage them.";
    metadata.autor = "LightBird team";
    metadata.site = "lightbird.cc";
    metadata.email = "team@lightbird.cc";
    metadata.version = "1.0";
    metadata.licence = "CC BY-NC-SA 3.0";
}

void        File::log(LightBird::ILogs::Level level, const QDateTime &date, const QString &message,
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

bool            File::timer(const QString &name)
{
    QByteArray  log;

    // Write the logs buffered in the file
    if (name == "writeLog")
    {
        this->mutex.lock();
        if(!this->file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text))
        {
            std::cerr << "Unable to open the log file (from the plugin Log/File)." << std::endl;
            this->_createLogFile();
            this->buffer.clear();
            this->mutex.unlock();
            return (true);
        }
        QStringListIterator it(this->buffer);
        while (it.hasNext())
        {
            if (log.size() + it.peekNext().size() + this->file.size() > this->maxSize)
            {
                this->file.write(log);
                log.clear();
                if (!this->_manageFiles())
                {
                    this->buffer.clear();
                    this->mutex.unlock();
                    return (true);
                }
            }
            log += it.next();
        }
        this->file.write(log);
        this->file.close();
        this->buffer.clear();
        this->mutex.unlock();
    }
    return (true);
}

bool        File::_createLogFile()
{
    QDir    directory;

    if (!this->directory.exists() && !directory.mkpath(this->path))
    {
        std::cerr << "Unable to create the log directory (from the plugin Log/File)." << std::endl;
        return (false);
    }
    this->file.setFileName(this->path + this->name);
    if(!(this->file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)))
    {
        std::cerr << "Unable to create the log file (from the plugin Log/File)." << std::endl;
        return (false);
    }
    this->file.close();
    return (true);
}

bool                            File::_manageFiles()
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
            LOG_WARNING("Cannot rename the log file to archive it", properties, "File", "_manageFiles");
        }
    }
    else
    {
        properties["archive"] = this->file.fileName();
        LOG_TRACE("Log archive file created", properties, "File", "_manageFiles");
    }
    this->file.close();
    this->_createLogFile();
    if(!this->file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text))
    {
        std::cerr << "Unable to open the log file (from the plugin Log/File)." << std::endl;
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
            this->api->log().warning("Can't remove the log archive file", properties, "File", "_manageFiles");
        else
            LOG_TRACE("Log archive file removed", properties, "File", "_manageFiles");
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
                this->api->log().warning("Can't remove the expired log archive file", properties, "File", "_manageFiles");
            else
                LOG_TRACE("Expired log archive file removed", properties, "File", "_manageFiles");
            flag = true;
        }
        files.erase(files.begin());
    }
    return (true);
}

QString File::_mapToString(const QMap<QString, QString> &properties)
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
