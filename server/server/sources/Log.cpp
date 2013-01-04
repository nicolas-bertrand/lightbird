#include <iostream>

#include "ILog.h"

#include "ApiLogs.h"
#include "Configurations.h"
#include "Library.h"
#include "LightBird.h"
#include "Log.h"
#include "Plugins.hpp"
#include "Threads.h"

Log     *Log::_instance = NULL;

Log *Log::instance()
{
    if (Log::_instance == NULL)
        Log::_instance = new Log();
    return (Log::_instance);
}

Log::Log()
    : mutex(QMutex::Recursive)
{
    this->display = false;
    this->state = Log::BEGIN;
    qRegisterMetaType<Properties>("Properties");
    qRegisterMetaType<LightBird::ILogs::Level>("LightBird::ILogs::Level");
    QObject::connect(this, SIGNAL(writeLog(LightBird::ILogs::Level,QDateTime,QString,Properties,QString,QString,QString,QString)),
                     this, SLOT(_write(LightBird::ILogs::Level,QDateTime,QString,Properties,QString,QString,QString,QString)),
                     Qt::QueuedConnection);
    // Maps the names of the log levels
    this->levels[LightBird::ILogs::FATAL] = "Fatal";
    this->levels[LightBird::ILogs::ERROR] = "Error";
    this->levels[LightBird::ILogs::WARNING] = "Warning";
    this->levels[LightBird::ILogs::INFO] = "Info";
    this->levels[LightBird::ILogs::DEBUG] = "Debug";
    this->levels[LightBird::ILogs::TRACE] = "Trace";
    this->level = LightBird::ILogs::TRACE;
    // Allows the library to use the logs
    LightBird::Library::setLog(new ApiLogs());
}

Log::~Log()
{
    if (this->isRunning())
    {
        this->quit();
        this->wait();
    }
    Log::trace("Log destroyed!", "Log", "Log");
}

void    Log::write(LightBird::ILogs::Level level, const QString &message, const QString &plugin, const Properties &properties, const QString &object, const QString &method)
{
    switch (this->state)
    {
        case Log::PLUGIN:
            if (this->level <= level)
            {
                emit writeLog(level, QDateTime::currentDateTime(), message, properties, LightBird::addressToString(this->currentThread()), plugin, object, method);
                this->_print(level, QDateTime::currentDateTime(), message, properties, LightBird::addressToString(this->currentThread()), plugin, object, method);
            }
            break;
        case Log::CONFIGURATION:
            this->mutex.lock();
            if (this->state == Log::CONFIGURATION && this->level <= level)
            {
                this->buffer.push_back(LogInformations(level, QDateTime::currentDateTime(), message, properties, LightBird::addressToString(this->currentThread()), plugin, object, method));
                this->_print(level, QDateTime::currentDateTime(), message, properties, LightBird::addressToString(this->currentThread()), plugin, object, method);
            }
            else if (this->state != Log::CONFIGURATION)
                this->write(level, message, plugin, properties, object, method);
            this->mutex.unlock();
            break;
        case Log::BEGIN:
            this->mutex.lock();
            if (this->state == Log::BEGIN)
                this->buffer.push_back(LogInformations(level, QDateTime::currentDateTime(), message, properties, LightBird::addressToString(this->currentThread()), plugin, object, method));
            else
                this->write(level, message, plugin, properties, object, method);
            this->mutex.unlock();
            break;
        case Log::END:
            if (this->level <= level)
                this->_print(level, QDateTime::currentDateTime(), message, properties, LightBird::addressToString(this->currentThread()), plugin, object, method);
    }
}

void    Log::fatal(const QString &message, const QString &object, const QString &method)
{
    Log::instance()->write(LightBird::ILogs::FATAL, message, "", Properties(), object, method);
}

void    Log::fatal(const QString &message, const Properties &properties, const QString &object, const QString &method)
{
    Log::instance()->write(LightBird::ILogs::FATAL, message, "", properties, object ,method);
}

void    Log::error(const QString &message, const QString &object, const QString &method)
{
    Log::instance()->write(LightBird::ILogs::ERROR, message, "", Properties(), object, method);
}

void    Log::error(const QString &message, const Properties &properties, const QString &object, const QString &method)
{
    Log::instance()->write(LightBird::ILogs::ERROR, message, "", properties, object, method);
}

void    Log::warning(const QString &message, const QString &object, const QString &method)
{
    Log::instance()->write(LightBird::ILogs::WARNING, message, "", Properties(), object,method);
}

void    Log::warning(const QString &message, const Properties &properties, const QString &object, const QString &method)
{
    Log::instance()->write(LightBird::ILogs::WARNING, message, "", properties, object, method);
}

void    Log::info(const QString &message, const QString &object, const QString &method)
{
    Log::instance()->write(LightBird::ILogs::INFO, message, "", Properties(), object,method);
}

void    Log::info(const QString &message, const Properties &properties, const QString &object, const QString &method)
{
    Log::instance()->write(LightBird::ILogs::INFO, message, "", properties, object, method);
}

void    Log::debug(const QString &message, const QString &object, const QString &method)
{
    Log::instance()->write(LightBird::ILogs::DEBUG, message, "", Properties(), object,method);
}

void    Log::debug(const QString &message, const Properties &properties, const QString &object, const QString &method)
{
    Log::instance()->write(LightBird::ILogs::DEBUG, message, "", properties, object, method);
}

void    Log::trace(const QString &message, const QString &object, const QString &method)
{
    Log::instance()->write(LightBird::ILogs::TRACE, message, "", Properties(), object, method);
}

void    Log::trace(const QString &message, const Properties &properties, const QString &object, const QString &method)
{
    Log::instance()->write(LightBird::ILogs::TRACE, message, "", properties, object,method);
}

LightBird::ILogs::Level Log::getLevel() const
{
    return (this->level);
}

void    Log::setLevel(LightBird::ILogs::Level level)
{
    this->level = level;
}

bool    Log::isDisplay() const
{
    return (this->display);
}

void    Log::isDisplay(bool display)
{
    this->display = display;
}

bool    Log::isError() const
{
    return (this->level <= LightBird::ILogs::ERROR);
}

bool    Log::isWarning() const
{
    return (this->level <= LightBird::ILogs::WARNING);
}

bool    Log::isInfo() const
{
    return (this->level <= LightBird::ILogs::INFO);
}

bool    Log::isDebug() const
{
    return (this->level <= LightBird::ILogs::DEBUG);
}

bool    Log::isTrace() const
{
    return (this->level <= LightBird::ILogs::TRACE);
}

void    Log::setState(Log::State mode)
{
    Mutex   mutex(this->mutex, "Log", "setState");

    if (this->state == Log::BEGIN && mode == Log::CONFIGURATION)
        this->_initializeConfiguration();
    else if (this->state == Log::CONFIGURATION && mode == Log::PLUGIN)
        this->_initializePlugin();
    this->state = mode;
}

void    Log::run()
{
    // Tells to the thread that started the current thread that it is running
    this->waitMutex.lock();
    this->waitRun.wakeAll();
    this->awake = true;
    this->waitMutex.unlock();
    Log::debug("Log thread started", "Log", "run");
    this->exec();
    Log::debug("Log thread finished", "Log", "run");
    // The thread where lives the Log is changed to the thread of its parent
    if (this->parent != NULL)
        this->moveToThread(this->parent->thread());
}

void    Log::_initializeConfiguration()
{
    QString level;

    // Loads the current log level
    level = Configurations::instance()->get("log/level").toLower();
    // Puts the first letter in upper case, to match the values of the map
    level = level.left(1).toUpper() + level.right(level.size() - 1);
    this->level = this->levels.key(level, LightBird::ILogs::INFO);
    this->display = false;
    if (Configurations::instance()->get("log/display") == "true")
        this->display = true;
    // Prints the buffered logs
    QListIterator<Log::LogInformations> log(this->buffer);
    while (log.hasNext())
        if (this->level <= log.next().level)
            this->_print(log.peekPrevious().level, log.peekPrevious().date, log.peekPrevious().message, log.peekPrevious().properties, log.peekPrevious().thread, log.peekPrevious().plugin, log.peekPrevious().object, log.peekPrevious().method);
}

void    Log::_initializePlugin()
{
    // Writes the buffered logs
    QListIterator<Log::LogInformations> log(this->buffer);
    while (log.hasNext())
        if (this->level <= log.next().level)
            emit writeLog(log.peekPrevious().level, log.peekPrevious().date, log.peekPrevious().message, log.peekPrevious().properties, log.peekPrevious().thread, log.peekPrevious().plugin, log.peekPrevious().object, log.peekPrevious().method);
    this->buffer.clear();
    this->moveToThread(this);
    this->awake = false;
    // Starts the log thread
    Threads::instance()->newThread(this, false);
    // Waits that the thread is started
    this->waitMutex.lock();
    if (!this->awake)
        this->waitRun.wait(&waitMutex);
    this->waitMutex.unlock();
    this->state = Log::PLUGIN;
}

QString     Log::_mapToString(const QMap<QString, QString> &properties) const
{
    QMapIterator<QString, QString>  it(properties);
    QString                         result;

    while (it.hasNext())
    {
        it.next();
        if (!result.isEmpty())
            result += ", ";
        result += it.key() + ":" + it.value();
    }
    return (result);
}

Log::LogInformations::LogInformations(LightBird::ILogs::Level level, const QDateTime &date, const QString &message, const Properties &properties, const QString &thread, const QString &plugin, const QString &object, const QString &method)
{
    this->level = level;
    this->message = message;
    this->properties = properties;
    this->plugin = plugin;
    this->object = object;
    this->method = method;
    this->thread = thread;
    this->date = date;
}

void    Log::_write(LightBird::ILogs::Level level, const QDateTime &date, const QString &message, const Properties &properties, const QString &thread, const QString &plugin, const QString &object, const QString &method)
{
    QMap<QString, LightBird::ILog *> plugins;

    // Ensures that the mode is correct before getting the plugins
    this->mutex.lock();
    if (this->state == Log::PLUGIN)
        plugins = Plugins::instance()->getInstances<LightBird::ILog>();
    this->mutex.unlock();
    QMapIterator<QString, LightBird::ILog *> it(plugins);
    while (it.hasNext())
    {
        it.peekNext().value()->log(level, date, message, properties.toMap(), thread, plugin, object, method);
        Plugins::instance()->release(it.next().key());
    }
}

void    Log::_print(LightBird::ILogs::Level level, const QDateTime &date, const QString &message, const Properties &properties, const QString &thread, const QString &plugin, const QString &object, const QString &method) const
{
    QString buffer;

    if (!this->display)
        return ;
    buffer += date.toString("[hh:mm:ss:zzz]");
    buffer += " [" + this->levels[level] + "]";
    buffer += " [" + thread;
    if (!plugin.isEmpty())
        buffer += "::" + plugin;
    if (!object.isEmpty())
        buffer += "::" + object;
    if (!method.isEmpty())
        buffer += "::" + method;
    buffer += "]";
    if (properties.toMap().size())
        buffer += " [" + this->_mapToString(properties.toMap()) + "]";
    buffer += " : " + message;
    this->printMutex.lock();
    std::cout << buffer.toStdString() << std::endl;
    this->printMutex.unlock();
}

void    Log::print()
{
    bool    display = this->display;

    this->mutex.lock();
    this->display = true;
    QListIterator<Log::LogInformations> it(Log::buffer);
    while (it.hasNext())
    {
        this->_print(it.peekNext().level, it.peekNext().date, it.peekNext().message, it.peekNext().properties, it.peekNext().thread, it.peekNext().plugin, it.peekNext().object, it.peekNext().method);
        it.next();
    }
    this->display = display;
    this->mutex.unlock();
}
