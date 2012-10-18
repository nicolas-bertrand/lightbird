#include <QDomNode>

#include "ApiTimers.h"
#include "Configurations.h"
#include "Defines.h"
#include "Log.h"

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

void    ApiTimers::setTimer(const QString &name, unsigned int interval)
{
    if (!this->mutex.tryLockForWrite(MAXTRYLOCK))
    {
        LOG_ERROR("Deadlock", "ApiTimers", "setTimer");
        return ;
    }
    if (this->timers.contains(name))
    {
        LOG_TRACE("Timer modified", Properties("id", this->id).add("name", name).add("interval", QString::number(interval)), "ApiTimers", "setTimer");
        this->timers.value(name)->setInterval(interval);
        this->mutex.unlock();
    }
    else
    {
        LOG_TRACE("Timer created", Properties("id", this->id).add("name", name).add("interval", QString::number(interval)), "ApiTimers", "setTimer");
        this->timers.insert(name, new Timer(this->id, name, interval, *this));
        this->mutex.unlock();
    }
}

unsigned int        ApiTimers::getTimer(const QString &name) const
{
    unsigned int    interval = 0;

    if (!this->mutex.tryLockForRead(MAXTRYLOCK))
    {
        LOG_ERROR("Deadlock", "ApiTimers", "getTimer");
        return (0);
    }
    if (this->timers.contains(name))
        interval = this->timers.value(name)->getInterval();
    this->mutex.unlock();
    return (interval);
}

QMap<QString, unsigned int>     ApiTimers::getTimers() const
{
    QMap<QString, unsigned int> timers;

    if (!this->mutex.tryLockForRead(MAXTRYLOCK))
    {
        LOG_ERROR("Deadlock", "ApiTimers", "getTimers");
        return (timers);
    }
    QMapIterator<QString, Timer *> it(this->timers);
    while (it.hasNext())
    {
        it.next();
        timers.insert(it.key(), it.value()->getInterval());
    }
    this->mutex.unlock();
    return (timers);
}

bool    ApiTimers::removeTimer(const QString &name)
{
    if (!this->mutex.tryLockForWrite(MAXTRYLOCK))
    {
        LOG_ERROR("Deadlock", "ApiTimers", "removeTimer");
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
    LOG_TRACE("Timer deleted", Properties("id", this->id).add("name", name), "ApiTimers", "removeTimer");
    this->mutex.unlock();
    return (true);
}
