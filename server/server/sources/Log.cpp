#include <iostream>

#include "Configurations.h"
#include "Plugins.hpp"
#include "Log.h"
#include "Threads.h"
#include "ILog.h"

Log     *Log::_instance = NULL;

Log *Log::instance(QObject *parent)
{
    if (Log::_instance == NULL)
        Log::_instance = new Log(parent);
    return (Log::_instance);
}

Log::Log(QObject *parent) : mutex(QMutex::Recursive)
{
    this->parent = parent;
    this->display = false;
    this->mode = Log::BUFFER;
    qRegisterMetaType<Properties>("Properties");
    qRegisterMetaType<LightBird::ILogs::level>("LightBird::ILogs::level");
    QObject::connect(this, SIGNAL(writeLog(LightBird::ILogs::level,QDateTime,QString,Properties,QString,QString,QString,QString)),
                     this, SLOT(_write(LightBird::ILogs::level,QDateTime,QString,Properties,QString,QString,QString,QString)),
                     Qt::QueuedConnection);

    // Map the names of the log levels
    this->levels[LightBird::ILogs::FATAL] = "Fatal";
    this->levels[LightBird::ILogs::ERROR] = "Error";
    this->levels[LightBird::ILogs::WARNING] = "Warning";
    this->levels[LightBird::ILogs::INFO] = "Info";
    this->levels[LightBird::ILogs::DEBUG] = "Debug";
    this->levels[LightBird::ILogs::TRACE] = "Trace";
    this->level = LightBird::ILogs::TRACE;
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

void    Log::write(LightBird::ILogs::level level, const QString &message, const QString &plugin,
        const Properties &properties, const QString &object, const QString &method)
{
    switch (this->mode)
    {
        case Log::WRITE:
            if (this->levels.contains(level) && this->level <= level)
            {
                emit writeLog(level, QDateTime::currentDateTime(), message, properties, QString::number((quint64)this->currentThread(), 16).toLower(), plugin, object, method);
                this->_print(level, QDateTime::currentDateTime(), message, properties, QString::number((quint64)this->currentThread(), 16).toLower(), plugin, object, method);
            }
            break;
        case Log::BUFFER:
            this->buffer.push_back(LogInformations(level, QDateTime::currentDateTime(), message, properties, QString::number((quint64)this->currentThread(), 16).toLower(), plugin, object, method));
            break;
        default:
            if (this->levels.contains(level) && this->level <= level)
                this->_print(level, QDateTime::currentDateTime(), message, properties, QString::number((quint64)this->currentThread(), 16).toLower(), plugin, object, method);
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

LightBird::ILogs::level Log::getlevel()
{
    return (this->level);
}

void    Log::setLevel(LightBird::ILogs::level level)
{
    if (this->levels.contains(level))
        this->level = level;
}

bool    Log::isError()
{
    if (this->level <= LightBird::ILogs::ERROR)
        return (true);
    return (false);
}

bool    Log::isWarning()
{
    if (this->level <= LightBird::ILogs::WARNING)
        return (true);
    return (false);
}

bool    Log::isInfo()
{
    if (this->level <= LightBird::ILogs::INFO)
        return (true);
    return (false);
}

bool    Log::isDebug()
{
    if (this->level <= LightBird::ILogs::DEBUG)
        return (true);
    return (false);
}

bool    Log::isTrace()
{
    if (this->level <= LightBird::ILogs::TRACE)
        return (true);
    return (false);
}

void    Log::setMode(Log::Mode mode)
{
    if (this->mode == mode)
        return ;
    this->mode = mode;
    if (this->mode == Log::WRITE)
        this->_initializeWrite();
}

void        Log::run()
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

void    Log::_initializeWrite()
{
    QString level;

    if (this->mode != Log::WRITE || this->isRunning() || this->isFinished())
        return ;

    // Load the current log level
    level = Configurations::instance()->get("log/level").toLower();
    // Put the first letter in upper case, to match the values of the map
    level = level.left(1).toUpper() + level.right(level.size() - 1);
    this->level = this->levels.key(level, LightBird::ILogs::INFO);
    this->display = false;
    if (Configurations::instance()->get("log/display") == "true")
        this->display = true;
    // Write the buffered logs
    QListIterator<Log::LogInformations> log(this->buffer);
    while (log.hasNext())
    {
        emit writeLog(log.peekNext().level, log.peekNext().date, log.peekNext().message, log.peekNext().properties, log.peekNext().thread, log.peekNext().plugin, log.peekNext().object, log.peekNext().method);
        this->_print(log.peekNext().level, log.peekNext().date, log.peekNext().message, log.peekNext().properties, log.peekNext().thread, log.peekNext().plugin, log.peekNext().object, log.peekNext().method);
        log.next();
    }
    this->buffer.clear();
    // Start the log thread
    this->moveToThread(this);
    this->awake = false;
    Threads::instance()->newThread(this);
    // Wait that the thread is started
    this->waitMutex.lock();
    if (!this->awake)
        this->waitRun.wait(&waitMutex);
    this->waitMutex.unlock();
}

QString     Log::_mapToString(const QMap<QString, QString> &properties)
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

Log::LogInformations::LogInformations(LightBird::ILogs::level level, const QDateTime &date, const QString &message, const Properties &properties,
                                      const QString &thread, const QString &plugin, const QString &object, const QString &method)
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

void    Log::_write(LightBird::ILogs::level level, const QDateTime &date, const QString &message, const Properties &properties,
                    const QString &thread, const QString &plugin, const QString &object, const QString &method)
{
    LightBird::ILog *instance;

    QStringListIterator it(Plugins::instance()->getLoadedPlugins());
    if (this->levels.contains(level) && this->level <= level)
        while (it.hasNext())
        {
            if ((instance = Plugins::instance()->getInstance<LightBird::ILog>(it.peekNext())))
            {
                instance->log(level, date, message, properties.toMap(), thread, plugin, object, method);
                Plugins::instance()->release(it.peekNext());
            }
            it.next();
        }
}

void        Log::_print(LightBird::ILogs::level level, const QDateTime &date, const QString &message, const Properties &properties,
                        const QString &thread, const QString &plugin, const QString &object, const QString &method)
{
    QString buffer;

    if (!this->display)
        return ;
    buffer += date.toString("[hh:mm:ss:zzz]");
    if (this->levels.contains(level))
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
    this->mutex.lock();
    std::cout << buffer.toStdString() << std::endl;
    this->mutex.unlock();
}

void    Log::print()
{
    QListIterator<Log::LogInformations> it(Log::buffer);
    bool                                display = this->display;

    this->display = true;
    while (it.hasNext())
    {
        Log::_print(it.peekNext().level, it.peekNext().date, it.peekNext().message, it.peekNext().properties, it.peekNext().thread, it.peekNext().plugin, it.peekNext().object, it.peekNext().method);
        it.next();
    }
    this->display = display;
}
