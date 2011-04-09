#include <QCoreApplication>
#include <QTimer>

#include "Timer.h"
#include "Plugins.hpp"
#include "ITimer.h"
#include "Log.h"
#include "Threads.h"

Timer::Timer(QString id, QString name, unsigned int timeout, QObject *parent)
{
    this->parent = parent;
    this->id = id;
    this->name = name;
    this->timeout = timeout;
    this->stopped = false;
    Log::debug("Loading the timer", Properties("name", this->name).add("id", this->id).add("timeout", QString::number(this->timeout)), "Timer", "Timer");
    // Start the Timer thread
    this->moveToThread(this);
    Threads::instance()->newThread(this);
}

Timer::~Timer()
{
    Log::trace("Timer destroyed!", Properties("id", this->id).add("name", this->name), "Timer", "~Timer");
}

void    Timer::run()
{
    // If the plugin dsoesn't implements ITimer
    if (!Plugins::instance()->getInstance<LightBird::ITimer>(this->id) || this->stopped)
        return ;
    Plugins::instance()->release(this->id);
    QTimer::singleShot(this->timeout, this, SLOT(_timeout()));
    Log::trace("Timer thread started", Properties("id", this->id).add("name", this->name), "Timer", "run");
    this->exec();
    Log::trace("Timer thread finished", Properties("id", this->id).add("name", this->name), "Timer", "run");
    // The thread where lives the Timer is changed to the thread of its parent
    this->moveToThread(QCoreApplication::instance()->thread());
}

unsigned int    Timer::getTimeout()
{
    return (this->timeout);
}

void            Timer::setTimeout(unsigned int timeout)
{
    this->timeout = timeout;
}

void            Timer::stop()
{
    this->stopped = true;
}

void                    Timer::_timeout()
{
    LightBird::ITimer   *instance;

    Log::trace("Timer timeout", Properties("id", this->id).add("name", this->name), "Timer", "_timeout");
    if (this->stopped || !(instance = Plugins::instance()->getInstance<LightBird::ITimer>(this->id)))
        return ;
    instance->timer(this->name);
    if (!this->stopped)
        QTimer::singleShot(this->timeout, this, SLOT(_timeout()));
    Plugins::instance()->release(this->id);
}
