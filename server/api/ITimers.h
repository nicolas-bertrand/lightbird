#ifndef ITIMERS_H
# define ITIMERS_H

# include <QMap>
# include <QString>

namespace LightBird
{
    /// @brief With this interface, the plugins can manage their timers.
    ///
    /// A timer is a method that is called periodically in a dedicated thread.
    /// This allows plugins to process asynchronous tasks easily.
    class ITimers
    {
    public:
        virtual ~ITimers() {}

        /// @brief Add a new timer. If the timer already exists, its timeout is modified.
        /// @param name : The name of the timer.
        /// @param timeout : The time between each calls in milliseconds.
        virtual void                            setTimer(const QString &name, unsigned int timeout) = 0;
        /// @brief Returns the timeout of a timer, or zero if it doesn't exists.
        /// @param name : The name of the timer.
        virtual unsigned int                    getTimer(const QString &name) = 0;
        /// @brief Returns a map of all the timers and their timeout.
        virtual QMap<QString, unsigned int>     getTimers() = 0;
        /// @brief Remove a timer.
        /// @param name : The name of the timer.
        /// @return False if the timer doesn't exists.
        virtual bool                            removeTimer(const QString &name) = 0;
    };
}

Q_DECLARE_INTERFACE(LightBird::ITimers, "cc.lightbird.ITimers");

#endif // ITIMERS_H
