#include <QDomElement>

#include "Defines.h"
#include "Log.h"
#include "ApiTimers.h"
#include "Configurations.h"

ApiTimers::ApiTimers(const QString &id, QObject *parent) : QObject(parent)
{
    QDomElement     dom;
    QDomNode        timer;
    Configuration   *configuration;

    this->id = id;
    // Get the maximum numbre of timers autorized from the configuration
    this->maxTimers = DEFAULT_MAX_TIMERS;
    if (Configurations::instance()->count("maxTimers"))
        this->maxTimers = Configurations::instance()->get("maxTimers").toInt();
    if (this->maxTimers < 0)
        this->maxTimers = 0;
    // Load the timers from the configuration of the plugin
    configuration = Configurations::instance(Configurations::instance()->get("pluginsPath") + "/" + id + "/Configuration.xml");
    dom = configuration->readDom();
    for (timer = dom.elementsByTagName("timers").at(0).firstChild(); timer.isNull() == false && this->timers.size() < this->maxTimers; timer = timer.nextSibling())
        if (timer.isElement() == true)
            this->timers.insert(timer.toElement().nodeName(), new Timer(this->id, timer.toElement().nodeName(), timer.toElement().text().toInt(), this));
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
    Log::trace("ApiTimers destroyed!", Properties("id", this->id), "ApiTimers", "~ApiTimers");
}

bool    ApiTimers::setTimer(const QString &name, unsigned int timeout)
{
    if (!this->lockTimers.tryLockForWrite(MAXTRYLOCK))
    {
        Log::error("Deadlock", "ApiTimers", "setTimer");
        return (false);
    }
    if (this->timers.contains(name))
    {
        Log::trace("Timer modified", Properties("id", this->id).add("name", name).add("timeout", QString::number(timeout)), "ApiTimers", "setTimer");
        this->timers.value(name)->setTimeout(timeout);
        this->lockTimers.unlock();
        return (true);
    }
    if (this->timers.size() >= this->maxTimers)
    {
        Log::warning("Unable to create a new timer. The maximum number of timers for this plugin has already been reached.", Properties("id", this->id).add("timerName", name).add("maxTimers", this->maxTimers), "ApiTimers", "setTimer");
        this->lockTimers.unlock();
        return (false);
    }
    Log::trace("Timer created", Properties("id", this->id).add("name", name).add("timeout", QString::number(timeout)), "ApiTimers", "setTimer");
    this->timers.insert(name, new Timer(this->id, name, timeout, this));
    this->lockTimers.unlock();
    return (true);
}

unsigned int        ApiTimers::getTimer(const QString &name)
{
    unsigned int    timeout = 0;

    if (!this->lockTimers.tryLockForRead(MAXTRYLOCK))
    {
        Log::error("Deadlock", "ApiTimers", "getTimer");
        return (0);
    }
    if (this->timers.contains(name))
        timeout = this->timers.value(name)->getTimeout();
    this->lockTimers.unlock();
    return (timeout);
}

QMap<QString, unsigned int>     ApiTimers::getTimers()
{
    QMap<QString, unsigned int> timers;

    if (!this->lockTimers.tryLockForRead(MAXTRYLOCK))
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
    this->lockTimers.unlock();
    return (timers);
}

bool    ApiTimers::removeTimer(const QString &name)
{
    if (!this->lockTimers.tryLockForWrite(MAXTRYLOCK))
    {
        Log::error("Deadlock", "ApiTimers", "removeTimer");
        return (false);
    }
    if (!this->timers.contains(name))
    {
        this->lockTimers.unlock();
        return (true);
    }
    this->timers.value(name)->stop();
    this->timers.value(name)->quit();
    this->timers.remove(name);
    Log::trace("Timer deleted", Properties("id", this->id).add("name", name), "ApiTimers", "removeTimer");
    this->lockTimers.unlock();
    return (true);
}
