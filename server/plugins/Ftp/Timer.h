#ifndef TIMER_H
# define TIMER_H

# include <QMutex>

# include "IApi.h"
# include "ITimer.h"

# define TIMER_IDENTIFY "identify" // The name of the timer that identifies the files.
# define TIMER_TIMEOUT  "timeout"  // The name of the timer that disconnects the inactive clients.

/// @brief Manages the identification of the uploaded files and the connections timeout.
class Timer : public LightBird::ITimer
{
public:
    Timer(LightBird::IApi &api);
    virtual ~Timer();

    /// @see LightBird::ITimer
    bool    timer(const QString &name);
    /// @brief Adds a file to be identified in the timer thread.
    /// @param idFile : The id of the file to identify.
    void    identify(const QString &idFile);
    /// @brief Starts the timeout of the connection. If the client stays inactive
    /// until the timeout, it is disconnected.
    void    startTimeout(const QString &idClient);
    /// @brief Stops the timeout while the client is active.
    void    stopTimeout(const QString &idClient);

private:
    Timer(const Timer &);
    Timer   &operator=(const Timer &);

    /// @brief Identifies the files from the identifyList in the thread timer.
    bool    _identify();
    /// @brief Disconnects the inactive clients.
    bool    _timeout();

    LightBird::IApi &api;
    QMutex          mutex;             ///< Makes the class thread safe.
    QStringList     identifyList;      ///< The id of the files to identify in the timer thread.
    QDateTime       nextTimeout;       ///< The next timeout of the timer.
    QHash<QString, QDateTime> timeout; ///< The list of the dates
};

#endif // TIMER_H
