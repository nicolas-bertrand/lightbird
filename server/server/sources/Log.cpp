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

Log::Log(QObject *parent) : lockWrite(QMutex::Recursive)
{
    this->parent = parent;
    this->display = false;
    this->mode = Log::BUFFER;
    qRegisterMetaType<Properties>("Properties");
    QObject::connect(this, SIGNAL(writeLog(char,QString,QString,Properties,QString,QString,QString,QDateTime)),
                     this, SLOT(_write(char,QString,QString,Properties,QString,QString,QString,QDateTime)),
                     Qt::QueuedConnection);

    // Map the names of the log levels
    this->levels[LightBird::ILogs::FATAL] = "Fatal";
    this->levels[LightBird::ILogs::ERROR] = "Error";
    this->levels[LightBird::ILogs::WARNING] = "Warning";
    this->levels[LightBird::ILogs::INFO] = "Info";
    this->levels[LightBird::ILogs::DEBUG] = "Debug";
    this->levels[LightBird::ILogs::TRACE] = "Trace";
    this->logLevel = LightBird::ILogs::TRACE;
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
    if (this->mode == Log::PRINT && this->levels.contains(level) && this->logLevel <= level)
        this->_print(level, message, plugin, properties, object, method, QString::number((unsigned long int)this->currentThread(), 16).toLower(), QDateTime::currentDateTime());
    else if (this->mode == Log::BUFFER)
        this->buffer.push_back(LogInformations(level, message, properties, plugin, object, method, QString::number((unsigned long int)this->currentThread(), 16).toLower(), QDateTime::currentDateTime()));
    else if (this->levels.contains(level) && this->logLevel <= level)
    {
        emit writeLog(level, message, plugin, properties, object, method, QString::number((unsigned long int)this->currentThread(), 16).toLower(), QDateTime::currentDateTime());
        this->_print(level, message, plugin, properties, object, method, QString::number((unsigned long int)this->currentThread(), 16).toLower(), QDateTime::currentDateTime());
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
    return (this->logLevel);
}

void    Log::setLevel(LightBird::ILogs::level level)
{
    if (this->levels.contains(level))
        this->logLevel = level;
}

bool    Log::isError()
{
    if (this->logLevel <= LightBird::ILogs::ERROR)
        return (true);
    return (false);
}

bool    Log::isWarning()
{
    if (this->logLevel <= LightBird::ILogs::WARNING)
        return (true);
    return (false);
}

bool    Log::isInfo()
{
    if (this->logLevel <= LightBird::ILogs::INFO)
        return (true);
    return (false);
}

bool    Log::isDebug()
{
    if (this->logLevel <= LightBird::ILogs::DEBUG)
        return (true);
    return (false);
}

bool    Log::isTrace()
{
    if (this->logLevel <= LightBird::ILogs::TRACE)
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
    this->waitLock.lock();
    this->waitRun.wakeAll();
    this->awake = true;
    this->waitLock.unlock();
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
    QMapIterator<LightBird::ILogs::level, QString> it(this->levels);
    while (it.hasNext())
    {
        it.next();
        if (it.value().toLower() == level)
        {
            this->logLevel = it.key();
            break ;
        }
    }
    this->display = false;
    if (Configurations::instance()->get("log/display") == "true")
        this->display = true;

    this->moveToThread(this);

    // Write the buffered logs
    QListIterator<Log::LogInformations> log(this->buffer);
    while (log.hasNext())
    {
        emit writeLog(log.peekNext().level, log.peekNext().message, log.peekNext().plugin, log.peekNext().properties, log.peekNext().object, log.peekNext().method, log.peekNext().thread, log.peekNext().date);
        this->_print(log.peekNext().level, log.peekNext().message, log.peekNext().plugin, log.peekNext().properties, log.peekNext().object, log.peekNext().method, log.peekNext().thread, log.peekNext().date);
        log.next();
    }
    this->buffer.clear();

    // Start the log thread
    this->awake = false;
    Threads::instance()->newThread(this);
    // Wait that the thread is started
    this->waitLock.lock();
    if (!this->awake)
        this->waitRun.wait(&waitLock);
    this->waitLock.unlock();
}

QString     Log::_mapToString(const QMap<QString, QString> &properties)
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

Log::LogInformations::LogInformations(LightBird::ILogs::level level, const QString &message, const Properties &properties, const QString &plugin,
                                      const QString &object, const QString &method, const QString &thread, const QDateTime &date)
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

void    Log::_write(char levelTmp, const QString &message, const QString &plugin, const Properties &properties,
                    const QString &object, const QString &method, const QString &thread, const QDateTime &date)
{
    LightBird::ILog         *instance;
    LightBird::ILogs::level  level = (LightBird::ILogs::level)levelTmp;

    QStringListIterator it(Plugins::instance()->getLoadedPlugins());
    if (this->levels.contains(level) && this->logLevel <= level)
        while (it.hasNext())
        {
            if ((instance = Plugins::instance()->getInstance<LightBird::ILog>(it.peekNext())))
            {
                this->lockWrite.lock();
                instance->log(level, date, message, properties.toMap(), thread, plugin, object, method);
                this->lockWrite.unlock();
                Plugins::instance()->release(it.peekNext());
            }
            it.next();
        }
}

void        Log::_print(LightBird::ILogs::level level, const QString &message, const QString &plugin, const Properties &properties, const QString &object, const QString &method, const QString &thread, const QDateTime &date)
{
    QString buffer;

    if (!this->display)
        return ;
    this->lockWrite.lock();
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
    std::cout << buffer.toStdString() << std::endl;
    this->lockWrite.unlock();
}

void    Log::print()
{
    QListIterator<Log::LogInformations> it(Log::buffer);
    bool                                display = this->display;

    this->display = true;
    while (it.hasNext())
    {
        Log::_print(it.peekNext().level, it.peekNext().message, it.peekNext().plugin, it.peekNext().properties, it.peekNext().object, it.peekNext().method, it.peekNext().thread, it.peekNext().date);
        it.next();
    }
    this->display = display;
}
