#ifndef TIMER_H
# define TIMER_H

# include <QString>
# include <QThread>

class ApiTimers;

/// @brief Manages one timer of a plugin. The events of the timer occure in a
/// specific thread, created by this class. The timers are manage by ApiTimers.
/// @see LightBird::ITimer
class Timer : public QThread
{
    Q_OBJECT

public :
    Timer(QString id, QString name, unsigned timeout, ApiTimers &apiTimers);
    ~Timer();
    Timer(const Timer &timer);
    Timer &operator=(const Timer &timer);

    ///< @brief The main method of the timer thread.
    void        run();
    ///< @brief Returns the interval between each events.
    unsigned    getTimeout();
    ///< @brief Allows to modify the timeout of the timer.
    void        setTimeout(unsigned int timeout);
    ///< @brief Stops the timer.
    void        stop();

private slots:
    ///< @brief This slot is called for each timeout and calls LightBird::ITimer.
    void        _timeout();

private:
    Timer();

    QString     id;         ///< The id of the plugin.
    QString     name;       ///< The name of the timer.
    unsigned    timeout;    ///< The number of milliseconds between each call to _timeout.
    ApiTimers   &apiTimers; ///< Manages all the timers of a plugin.
    bool        stopped;    ///< True if the timer have to stop.
};

#endif // TIMER_H
