#include <QCoreApplication>

#include "ITimer.h"

#include "ApiTimers.h"
#include "Plugins.hpp"
#include "Log.h"
#include "Threads.h"
#include "Timer.h"

Timer::Timer(QString i, QString n, unsigned int in, ApiTimers &timers)
    : id(i)
    , name(n)
    , timer(this)
    , interval(in)
    , apiTimers(timers)
    , stopped(false)
{
    LOG_DEBUG("Loading the timer", Properties("name", this->name).add("id", this->id).add("interval", QString::number(this->interval)), "Timer", "Timer");
    this->timer.setSingleShot(true);
    // Starts the Timer thread
    this->moveToThread(this);
    Threads::instance()->newThread(this);
}

Timer::~Timer()
{
    LOG_TRACE("Timer destroyed!", Properties("id", this->id).add("name", this->name), "Timer", "~Timer");
}

void    Timer::run()
{
    // If the plugin dsoesn't implements ITimer
    if (!Plugins::instance()->getInstance<LightBird::ITimer>(this->id) || this->stopped)
        return ;
    Plugins::instance()->release(this->id);
    QObject::connect(&this->timer, SIGNAL(timeout()), this, SLOT(_timeout()), Qt::QueuedConnection);
    QObject::connect(this, SIGNAL(setIntervalSignal()), this, SLOT(_setInterval()), Qt::QueuedConnection);
    this->timer.start(this->interval);
    LOG_TRACE("Timer thread started", Properties("id", this->id).add("name", this->name), "Timer", "run");
    this->exec();
    LOG_TRACE("Timer thread finished", Properties("id", this->id).add("name", this->name), "Timer", "run");
    // The thread where lives the Timer is changed to the thread main
    this->moveToThread(QCoreApplication::instance()->thread());
}

unsigned int    Timer::getInterval() const
{
    return (this->interval);
}

void            Timer::setInterval(unsigned int interval)
{
    this->interval = interval;
    emit this->setIntervalSignal();
}

void            Timer::_setInterval()
{
    this->timer.setInterval(this->interval);
}

void            Timer::stop()
{
    this->stopped = true;
}

void                    Timer::_timeout()
{
    LightBird::ITimer   *instance;
    bool                result;

    //LOG_TRACE("Timer timeout", Properties("id", this->id).add("name", this->name), "Timer", "_timeout");
    if (this->stopped || !(instance = Plugins::instance()->getInstance<LightBird::ITimer>(this->id)))
        return ;
    result = instance->timer(this->name);
    Plugins::instance()->release(this->id);
    if (!this->stopped && result)
        this->timer.start();
    // Removes the timer if the plugin returned false
    if (!result)
        this->apiTimers.removeTimer(this->name);
}
