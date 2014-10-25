#ifndef CLIENTDISCONNECTTIMERS_H
# define CLIENTDISCONNECTTIMERS_H

#include <QTimer>
#include <QMutex>
#include <QDateTime>

class Client;

/// @brief Manages the disconnection timers of a client.
/// The timers are executed in the QCoreApplication thread.
/// @see LightBird::IClient::setDisconnectIdle
/// @see LightBird::IClient::setDisconnectTime
class ClientDisconnectTimers : public QObject
{
    Q_OBJECT

public:
    ClientDisconnectTimers(Client *client);
    ~ClientDisconnectTimers();

    // LightBird::IClient
    /// @see LightBird::IClient::setDisconnectIdle
    void setDisconnectIdle(qint64 msec = -1, bool fatal = false);
    /// @see LightBird::IClient::getDisconnectIdle
    qint64 getDisconnectIdle(bool *fatal = NULL) const;
    /// @see LightBird::IClient::setDisconnectTime
    void setDisconnectTime(const QDateTime &time = QDateTime(), bool fatal = false);
    /// @see LightBird::IClient::getDisconnectTime
    const QDateTime &getDisconnectTime(bool *fatal = NULL) const;

    // These methods are called by the client
    /// @brief The client is running.
    inline void clientRunning() { _idleNextTimeout = 0; }
    /// @brief The client is now idle.
    inline void clientIdle() { if (_idleMsec > 0) { _idleNextTimeout = QDateTime::currentMSecsSinceEpoch() + _idleMsec; emit startIdleTimer(); } }
    /// @brief The has been destroyed, and this instance can be deleted later.
    void clientDestroyed();

signals:
    /// @brief Calls _startIdleTimer in the main thread.
    void startIdleTimer();
    /// @brief Calls _startTimeTimer in the main thread.
    void startTimeTimer();

private slots:
    /// @brief Starts the idle timer from the main thread.
    void _startIdleTimer();
    /// @brief Starts the time timer from the main thread.
    void _startTimeTimer();
    /// @brief The client have to be disconnected.
    void _disconnectIdle();
    /// @brief The client have to be disconnected.
    void _disconnectTime();
    /// @brief Must be called when the client is disconnected.
    void _clientDisconnected();

private:
    Client *_client;
    QMutex _mutex;
    static const int _errorMargin = 100; ///< The number of milliseconds a timer can be fired in advance for the disconnection to occur (depends on the accuracy of QTimer).

    QTimer _idleTimer;
    bool _idleFatal;
    qint64 _idleMsec;
    qint64 _idleNextTimeout; ///< The time at which the client be disconnected.

    QTimer _timeTimer;
    bool _timeFatal;
    qint64 _timeMsec;
    QDateTime _timeNextTimeout; ///< The time at which the client will be disconnected.
};

#endif // CLIENTDISCONNECTTIMERS_H
