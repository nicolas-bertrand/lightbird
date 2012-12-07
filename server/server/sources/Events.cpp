#include "Defines.h"
#include "Events.h"
#include "Log.h"
#include "Server.h"

Events::Events(QObject *parent)
    : QObject(parent)
{
    LOG_TRACE("Events created", "Events", "Events");
}

Events::~Events()
{
    LOG_TRACE("Events destroyed!", "Events", "~Events");
}

void    Events::add(ApiEvents *apiEvents)
{
    if (!this->mutex.tryLock(MAXTRYLOCK))
        return Log::error("Deadlock", "Events", "add");
    if (!this->events.contains(apiEvents))
        this->events.push_back(apiEvents);
    this->mutex.unlock();
}

void    Events::remove(ApiEvents *apiEvents)
{
    if (!this->mutex.tryLock(MAXTRYLOCK))
        return Log::error("Deadlock", "Events", "remove");
    this->events.removeAll(apiEvents);
    this->mutex.unlock();
}

void    Events::send(const QString &event, const QVariant &property)
{
    if (!this->mutex.tryLock(MAXTRYLOCK))
        return Log::error("Deadlock", "Events", "send");
    QListIterator<ApiEvents *> it(this->events);
    while (it.hasNext())
        it.next()->post(event, property);
    LOG_TRACE("Event sent", Properties("event", event).add("property", property.toString(), false), "Events", "send");
    this->mutex.unlock();
}

Events  *Events::instance()
{
    return (Server::instance().getEvents());
}
