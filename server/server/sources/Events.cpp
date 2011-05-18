#include "Defines.h"
#include "Log.h"
#include "Events.h"

Events  *Events::_instance = NULL;

Events  *Events::instance(QObject *parent)
{
    if (Events::_instance == NULL)
        Events::_instance = new Events(parent);
    return (Events::_instance);
}

Events::Events(QObject *parent) : QObject(parent)
{
    Log::trace("Events created", "Events", "Events");
}

Events::~Events()
{
    Log::trace("Events destroyed!", "Events", "~Events");
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
    this->mutex.unlock();
}
