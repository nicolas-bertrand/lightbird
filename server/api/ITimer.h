#ifndef LIGHTBIRD_ITIMER_H
# define LIGHTBIRD_ITIMER_H

# include <QString>

namespace LightBird
{
    /// @brief This method is called periodically. The delay between each calls
    /// is defined in the configuration file of the plugin that implements it.
    /// It can also be modified at the runtime using LightBird::ITimers.
    /// As this method is called in a dedicated thread, the timer is suspended
    /// until the thread ends, i.e until the return of the method timer().
    class ITimer
    {
    public:
        virtual ~ITimer() {}

        /// @brief Called periodically by a timer.
        /// @param name : The name of the timer that called this method.
        /// @return False to stop the timer, true to continue.
        virtual bool    timer(const QString &name) = 0;
    };
}

Q_DECLARE_INTERFACE(LightBird::ITimer, "cc.lightbird.ITimer")

#endif // LIGHTBIRD_ITIMER_H
