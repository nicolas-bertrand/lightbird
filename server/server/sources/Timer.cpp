#include <QCoreApplication>
#include <QTimer>

#include "ITimer.h"

#include "ApiTimers.h"
#include "Plugins.hpp"
#include "Log.h"
#include "Threads.h"
#include "Timer.h"

Timer::Timer(QString i, QString n, unsigned int t, ApiTimers &timers) : id(i),
                                                                        name(n),
                                                                        timeout(t),
                                                                        apiTimers(timers),
                                                                        stopped(false)
{
    Log::debug("Loading the timer", Properties("name", this->name).add("id", this->id).add("timeout", QString::number(this->timeout)), "Timer", "Timer");
    // Starts the Timer thread
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
    // The thread where lives the Timer is changed to the thread main
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
    bool                result;

    //Log::trace("Timer timeout", Properties("id", this->id).add("name", this->name), "Timer", "_timeout");
    if (this->stopped || !(instance = Plugins::instance()->getInstance<LightBird::ITimer>(this->id)))
        return ;
    result = instance->timer(this->name);
    Plugins::instance()->release(this->id);
    if (!this->stopped && result)
        QTimer::singleShot(this->timeout, this, SLOT(_timeout()));
    // Removes the timer if the plugin returned false
    if (!result)
        this->apiTimers.removeTimer(this->name);
}
