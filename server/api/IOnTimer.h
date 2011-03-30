#ifndef ITIMER_H
# define ITIMER_H

# include <QString>

namespace StreamIt
{
    /**
     * @brief This method is called periodically. The delay between each calls
     * is defined in the configuration file of the plugin that implements it.
     * As this method is called in a dedicated thread, the timer is suspended
     * until the thread ends, i.e, until the return of timer.
     */
    class ITimer
    {
    public:
        virtual ~ITimer() {}
        /**
         * @brief Called periodically by a timer.
         * @param name : The name of the timer that called this method.
         */
        virtual void    timer(const QString &name) = 0;
    };
}

#endif // ITIMER_H
