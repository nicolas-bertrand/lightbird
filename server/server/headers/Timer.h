#ifndef TIMER_H
# define TIMER_H

# include <QString>
# include <QThread>
# include <QTimer>

class ApiTimers;

/// @brief Manages one timer of a plugin. The events of the timer occure in a
/// specific thread, created by this class. The timers are manage by ApiTimers.
/// @see LightBird::ITimer
class Timer : public QThread
{
    Q_OBJECT

public :
    Timer(QString id, QString name, unsigned int interval, ApiTimers &apiTimers);
    ~Timer();

    ///< @brief The main method of the timer thread.
    void        run();
    ///< @brief Returns the interval between each events.
    unsigned    getInterval() const;
    ///< @brief Allows to modify the interval of the timer.
    void        setInterval(unsigned int interval);
    ///< @brief Stops the timer.
    void        stop();

signals:
    ///< @brief Called by setInterval in order to call the method _setInterval
    /// in the Timer thread.
    void        setIntervalSignal();

private slots:
    ///< @brief This slot is called for each timeout and calls LightBird::ITimer.
    void        _timeout();
    ///< @brief Modify the interval of the timer. This method is called in the Timer
    /// thread since QTimer::setInterval requires it.
    void        _setInterval();

private:
    Timer();
    Timer(const Timer &timer);
    Timer &operator=(const Timer &timer);

    QString     id;         ///< The id of the plugin.
    QString     name;       ///< The name of the timer.
    QTimer      timer;      ///< Calls regularly the slot _timeout.
    unsigned    interval;   ///< The interval between each call to _timeout.
    ApiTimers   &apiTimers; ///< Manages all the timers of a plugin.
    bool        stopped;    ///< True if the timer have to stop.
};

#endif // TIMER_H
