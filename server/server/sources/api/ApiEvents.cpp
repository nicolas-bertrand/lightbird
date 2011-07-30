#include <QCoreApplication>

#include "IEvent.h"

#include "ApiEvents.h"
#include "Events.h"
#include "Log.h"
#include "Plugins.hpp"
#include "SmartMutex.h"
#include "Threads.h"

ApiEvents::ApiEvents(const QString &id, bool event)
{
    this->id = id;
    this->awake = false;
    // If the plugin implements IEvent, the thread is started
    if (event)
    {
        this->moveToThread(this);
        // Starting the events thread
        Threads::instance()->newThread(this);
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
    // The thread where lives the Timer is changed to the thread main
    this->moveToThread(QCoreApplication::instance()->thread());
}

void            ApiEvents::post(const QString &event, const QVariant &property)
{
    SmartMutex  mutex(this->mutex, "ApiEvents", "post");

    if (!mutex)
        return ;
    if (this->subscribed.contains(event))
    {
        this->events.push_back(QPair<QString, QVariant>(event, property));
        emit this->newEvent();
    }
}

void    ApiEvents::subscribe(const QString &event)
{
    this->subscribe(QStringList() << event);
}

void            ApiEvents::subscribe(const QStringList &events)
{
    SmartMutex  mutex(this->mutex, "ApiEvents", "subscribe");

    if (!mutex)
        return ;
    this->subscribed << events;
    this->subscribed.removeDuplicates();
    Log::debug("Events subscribed", Properties("id", this->id).add("events", events.join(";")), "ApiEvents", "subscribe");
}

void            ApiEvents::unsubscribe(const QString &event)
{
    SmartMutex  mutex(this->mutex, "ApiEvents", "unsubscribe");

    if (!mutex)
        return ;
    this->subscribed.removeAll(event);
    Log::debug("Event unsubscribed", Properties("id", this->id).add("event", event), "ApiEvents", "unsubscribe");
}

void            ApiEvents::unsubscribe(const QStringList &events)
{
    SmartMutex  mutex(this->mutex, "ApiEvents", "unsubscribe");

    if (!mutex)
        return ;
    QStringListIterator it(events);
    while (it.hasNext())
        this->subscribed.removeAll(it.next());
    Log::debug("Events unsubscribed", Properties("id", this->id).add("events", events.join(";")), "ApiEvents", "subscribe");
}

QStringList     ApiEvents::getEvents() const
{
    SmartMutex  mutex(this->mutex, "ApiEvents", "getEvents");

    if (!mutex)
        return (QStringList());
    return (this->subscribed);
}

void    ApiEvents::send(const QString &event, const QVariant &property)
{
    Events::instance()->send(event, property);
}

QList<QPair<QString, QVariant> > ApiEvents::receive()
{
    SmartMutex  mutex(this->mutex, "ApiEvents", "receive");
    QList<QPair<QString, QVariant> > result;

    if (!mutex)
        return (result);
    result = this->events;
    this->events.clear();
    return (result);
}

bool            ApiEvents::isAvailable() const
{
    SmartMutex  mutex(this->mutex, "ApiEvents", "isAvailable");

    if (!mutex)
        return (false);
    return (!this->events.isEmpty());
}

void                    ApiEvents::_newEvent()
{
    SmartMutex          mutex(this->mutex, "ApiEvents", "_newEvent");
    LightBird::IEvent   *plugin;

    if (!mutex)
        return ;
    if (!this->events.isEmpty())
    {
        QPair<QString, QVariant> event = this->events.front();
        this->events.pop_front();
        mutex.unlock();
        if ((plugin = Plugins::instance()->getInstance<LightBird::IEvent>(this->id)))
        {
            plugin->event(event.first, event.second);
            Plugins::instance()->release(this->id);
        }
    }
}
