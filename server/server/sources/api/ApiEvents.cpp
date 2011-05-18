#include "IEvent.h"
#include "Events.h"
#include "Log.h"
#include "Plugins.hpp"
#include "Threads.h"
#include "ApiEvents.h"

ApiEvents::ApiEvents(const QString &id, bool event)
{
    this->id = id;
    this->awake = false;
    // If the plugin implements IEvent, the thread is started
    if (event)
    {
        // Starting the events thread
        this->moveToThread(this);
        Threads::instance()->newThread(this, false);
        this->connect(this, SIGNAL(newEvent()), SLOT(_newEvent()), Qt::QueuedConnection);
        // Wait that the thread is started
        this->mutex.lock();
        if (!this->awake)
            this->wait.wait(&this->mutex);
        this->mutex.unlock();
    }
    Events::instance()->add(this);
}

ApiEvents::~ApiEvents()
{
    Events::instance()->remove(this);
}

void    ApiEvents::run()
{
    Log::trace("ApiEvents thread started", Properties("id", this->id), "ApiEvents", "run");
    // Tells to the thread that started the current thread that it is running
    this->mutex.lock();
    this->awake = true;
    this->wait.wakeAll();
    this->mutex.unlock();
    // Execute the event loop
    this->exec();
    Log::trace("ApiEvents thread finished", Properties("id", this->id), "ApiEvents", "run");
}

void    ApiEvents::post(const QString &event, const QVariant &property)
{
    if (!this->mutex.tryLock(MAXTRYLOCK))
        return Log::error("Deadlock", "ApiEvents", "post");
    if (this->subscribed.contains(event))
    {
        this->events.push_back(QPair<QString, QVariant>(event, property));
        emit this->newEvent();
    }
    this->mutex.unlock();
}

void    ApiEvents::subscribe(const QStringList &events)
{
    if (!this->mutex.tryLock(MAXTRYLOCK))
        return Log::error("Deadlock", "ApiEvents", "subscribe");
    this->subscribed = events;
    this->mutex.unlock();
}

void    ApiEvents::send(const QString &event, const QVariant &property)
{
    Events::instance()->send(event, property);
}

QList<QPair<QString, QVariant> > ApiEvents::receive()
{
    QList<QPair<QString, QVariant> > result;

    if (!this->mutex.tryLock(MAXTRYLOCK))
    {
        Log::error("Deadlock", "ApiEvents", "send");
        return (result);
    }
    result = this->events;
    this->events.clear();
    this->mutex.unlock();
    return (result);
}

bool        ApiEvents::isAvailable()
{
    bool    result;

    if (!this->mutex.tryLock(MAXTRYLOCK))
    {
        Log::error("Deadlock", "ApiEvents", "isAvailable");
        return (false);
    }
    result = !this->events.isEmpty();
    this->mutex.unlock();
    return (result);
}

void        ApiEvents::_newEvent()
{
    LightBird::IEvent *plugin;

    if (!this->mutex.tryLock(MAXTRYLOCK))
        return Log::error("Deadlock", "ApiEvents", "_newEvent");
    if (!this->events.isEmpty())
    {
        QPair<QString, QVariant> event = this->events.front();
        this->events.pop_front();
        if ((plugin = Plugins::instance()->getInstance<LightBird::IEvent>(this->id)))
        {
            this->mutex.unlock();
            plugin->event(event.first, event.second);
            Plugins::instance()->release(this->id);
            if (!this->mutex.tryLock(MAXTRYLOCK))
                return Log::error("Deadlock", "ApiEvents", "_newEvent");
        }
    }
    this->mutex.unlock();
}
