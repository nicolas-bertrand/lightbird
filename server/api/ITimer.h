#ifndef ITIMER_H
# define ITIMER_H

# include <QString>

namespace Streamit
{
    /// @brief This method is called periodically. The delay between each calls
    /// is defined in the configuration file of the plugin that implements it.
    /// It can also be modified at the runtime using Streamit::ITimers.
    /// As this method is called in a dedicated thread, the timer is suspended
    /// until the thread ends, i.e, until the return of the method timer().
    class ITimer
    {
    public:
        virtual ~ITimer() {}

        /// @brief Called periodically by a timer.
        /// @param name : The name of the timer that called this method.
        virtual void    timer(const QString &name) = 0;
    };
}

Q_DECLARE_INTERFACE(Streamit::ITimer, "cc.lightbird.ITimer");

#endif // ITIMER_H
