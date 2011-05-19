#include <QDomElement>

#include "Defines.h"
#include "Log.h"
#include "ApiTimers.h"
#include "Configurations.h"

ApiTimers::ApiTimers(const QString &id, QObject *parent) : QObject(parent)
{
    Configuration   *configuration;
    QDomNode        timer;

    this->id = id;
    // Load the timers from the configuration of the plugin
    configuration = Configurations::instance(id);
    timer = configuration->readDom().firstChildElement("timers");
    for (timer = timer.firstChild(); !timer.isNull(); timer = timer.nextSibling())
        if (timer.isElement())
            this->setTimer(timer.toElement().nodeName(), timer.toElement().text().toInt());
    configuration->release();
}

ApiTimers::~ApiTimers()
{
    QMapIterator<QString, Timer *> it(this->timers);
    while (it.hasNext())
    {
        it.peekNext().value()->stop();
        it.next().value()->quit();
    }
    this->timers.clear();
}

void    ApiTimers::setTimer(const QString &name, unsigned int timeout)
{
    if (!this->mutex.tryLockForWrite(MAXTRYLOCK))
    {
        Log::error("Deadlock", "ApiTimers", "setTimer");
        return ;
    }
    if (this->timers.contains(name))
    {
        Log::trace("Timer modified", Properties("id", this->id).add("name", name).add("timeout", QString::number(timeout)), "ApiTimers", "setTimer");
        this->timers.value(name)->setTimeout(timeout);
        this->mutex.unlock();
    }
    else
    {
        Log::trace("Timer created", Properties("id", this->id).add("name", name).add("timeout", QString::number(timeout)), "ApiTimers", "setTimer");
        this->timers.insert(name, new Timer(this->id, name, timeout, *this));
        this->mutex.unlock();
    }
}

unsigned int        ApiTimers::getTimer(const QString &name)
{
    unsigned int    timeout = 0;

    if (!this->mutex.tryLockForRead(MAXTRYLOCK))
    {
        Log::error("Deadlock", "ApiTimers", "getTimer");
        return (0);
    }
    if (this->timers.contains(name))
        timeout = this->timers.value(name)->getTimeout();
    this->mutex.unlock();
    return (timeout);
}

QMap<QString, unsigned int>     ApiTimers::getTimers()
{
    QMap<QString, unsigned int> timers;

    if (!this->mutex.tryLockForRead(MAXTRYLOCK))
    {
        Log::error("Deadlock", "ApiTimers", "getTimers");
        return (timers);
    }
    QMapIterator<QString, Timer *> it(this->timers);
    while (it.hasNext())
    {
        it.next();
        timers.insert(it.key(), it.value()->getTimeout());
    }
    this->mutex.unlock();
    return (timers);
}

bool    ApiTimers::removeTimer(const QString &name)
{
    if (!this->mutex.tryLockForWrite(MAXTRYLOCK))
    {
        Log::error("Deadlock", "ApiTimers", "removeTimer");
        return (false);
    }
    if (!this->timers.contains(name))
    {
        this->mutex.unlock();
        return (true);
    }
    this->timers.value(name)->stop();
    this->timers.value(name)->quit();
    this->timers.remove(name);
    Log::trace("Timer deleted", Properties("id", this->id).add("name", name), "ApiTimers", "removeTimer");
    this->mutex.unlock();
    return (true);
}
