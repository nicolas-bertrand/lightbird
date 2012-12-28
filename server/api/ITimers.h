#ifndef LIGHTBIRD_ITIMERS_H
# define LIGHTBIRD_ITIMERS_H

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

        /// @brief Add a new timer. If the timer already exists, its interval is modified.
        /// @param name : The name of the timer.
        /// @param interval : The time between each calls in milliseconds.
        /// If null the timer is called immediatly.
        virtual void                            setTimer(const QString &name, unsigned int interval = 0) = 0;
        /// @brief Returns the interval of a timer, or zero if it doesn't exists.
        /// @param name : The name of the timer.
        virtual unsigned int                    getTimer(const QString &name) const = 0;
        /// @brief Returns a map of all the timers and their interval.
        virtual QMap<QString, unsigned int>     getTimers() const = 0;
        /// @brief Remove a timer.
        /// @param name : The name of the timer.
        /// @return False if the timer doesn't exists.
        virtual bool                            removeTimer(const QString &name) = 0;
    };
}

Q_DECLARE_INTERFACE(LightBird::ITimers, "cc.lightbird.ITimers")

#endif // LIGHTBIRD_ITIMERS_H
