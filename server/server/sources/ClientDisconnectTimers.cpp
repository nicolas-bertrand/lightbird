#include <QCoreApplication>

#include "Client.h"
#include "ClientDisconnectTimers.h"
#include "Log.h"
#include "Mutex.h"

ClientDisconnectTimers::ClientDisconnectTimers(Client *client)
    : _client(client)
    , _idleFatal(false)
    , _idleMsec(-1)
    , _idleNextTimeout(0)
    , _timeFatal(false)
    , _timeMsec(-1)
{
    moveToThread(QCoreApplication::instance()->thread());
    _idleTimer.moveToThread(this->thread());
    _timeTimer.moveToThread(this->thread());
    QObject::connect(this, SIGNAL(startIdleTimer()), this, SLOT(_startIdleTimer()), Qt::QueuedConnection);
    QObject::connect(this, SIGNAL(startTimeTimer()), this, SLOT(_startTimeTimer()), Qt::QueuedConnection);
    QObject::connect(&_idleTimer, SIGNAL(timeout()), this, SLOT(_disconnectIdle()), Qt::DirectConnection);
    QObject::connect(&_timeTimer, SIGNAL(timeout()), this, SLOT(_disconnectTime()), Qt::DirectConnection);
}

ClientDisconnectTimers::~ClientDisconnectTimers()
{
}

void ClientDisconnectTimers::setDisconnectIdle(qint64 msec, bool fatal)
{
    Mutex mutex(_mutex, "ClientDisconnectTimers", "setDisconnectIdle");
    if (!mutex)
        return ;

    if (msec > 0)
    {
        _idleMsec = msec;
        _idleFatal = fatal;
        _idleNextTimeout = 0;
        if (!_client->isRunning())
            clientIdle();
    }
    else
        _idleMsec = -1;
}

qint64 ClientDisconnectTimers::getDisconnectIdle(bool *fatal) const
{
    if (fatal)
        *fatal = _idleFatal;
    return _idleMsec;
}

void ClientDisconnectTimers::setDisconnectTime(const QDateTime &time, bool fatal)
{
    Mutex mutex(_mutex, "ClientDisconnectTimers", "setDisconnectTime");
    if (!mutex)
        return ;

    _timeMsec = time.toMSecsSinceEpoch() - QDateTime::currentMSecsSinceEpoch();
    if (_timeMsec > 0)
    {
        _timeNextTimeout = time;
        _timeFatal = fatal;
        emit startTimeTimer();
    }
    else
    {
        _timeNextTimeout = QDateTime();
        _timeMsec = -1;
    }
}

const QDateTime &ClientDisconnectTimers::getDisconnectTime(bool *fatal) const
{
    if (fatal)
        *fatal = _timeFatal;
    return _timeNextTimeout;
}

void ClientDisconnectTimers::_startIdleTimer()
{
    Mutex mutex(_mutex, "ClientDisconnectTimers", "_startIdleTimer");
    if (!mutex)
        return ;

    if (_client && _idleMsec > 0 && _idleNextTimeout)
        _idleTimer.start(qMax(_idleNextTimeout - QDateTime::currentMSecsSinceEpoch(), qint64(0)));
}

void ClientDisconnectTimers::_startTimeTimer()
{
    Mutex mutex(_mutex, "ClientDisconnectTimers", "_startTimeTimer");
    if (!mutex)
        return ;

    if (_client && _timeMsec > 0)
        _timeTimer.start(qMax(_timeNextTimeout.toMSecsSinceEpoch() - QDateTime::currentMSecsSinceEpoch(), qint64(0)));
}

void ClientDisconnectTimers::_disconnectIdle()
{
    Mutex mutex(_mutex, "ClientDisconnectTimers", "_disconnectIdle");
    if (!mutex)
        return ;

    _idleTimer.stop();
    if (_client && _idleMsec > 0 && _idleNextTimeout)
    {
        if (_idleNextTimeout - QDateTime::currentMSecsSinceEpoch() < _errorMargin)
        {
            LOG_DEBUG("Client disconnected after being idle for too long", Properties("client id", _client->getId()).add("idle time", _idleMsec), "ClientDisconnectTimers", "_disconnectIdle");
            _client->disconnect(_idleFatal);
            _clientDisconnected();
        }
        // The timer was fired too soon
        else
            _idleTimer.start(qMax(_idleNextTimeout - QDateTime::currentMSecsSinceEpoch(), qint64(0)));
    }
}

void ClientDisconnectTimers::_disconnectTime()
{
    Mutex mutex(_mutex, "ClientDisconnectTimers", "_disconnectTime");
    if (!mutex)
        return ;

    _timeTimer.stop();
    if (_client && _timeMsec > 0)
    {
        if (_timeNextTimeout.toMSecsSinceEpoch() - QDateTime::currentMSecsSinceEpoch() < _errorMargin)
        {
            LOG_DEBUG("Client disconnected at the scheduled time", Properties("client id", _client->getId()).add("scheduled duration", _timeMsec), "ClientDisconnectTimers", "_disconnectTime");
            _client->disconnect(_timeFatal);
            _clientDisconnected();
        }
        // The timer was fired too soon
        else
            _timeTimer.start(qMax(_timeNextTimeout.toMSecsSinceEpoch() - QDateTime::currentMSecsSinceEpoch(), qint64(0)));
    }
}

void ClientDisconnectTimers::clientDestroyed()
{
    Mutex mutex(_mutex, "ClientDisconnectTimers", "clientDestroyed");
    if (!mutex)
        return ;

    _client = NULL;
    deleteLater();
}

void ClientDisconnectTimers::_clientDisconnected()
{
    QObject::disconnect(this, SIGNAL(startIdleTimer()), this, SLOT(_startIdleTimer()));
    QObject::disconnect(this, SIGNAL(startTimeTimer()), this, SLOT(_startTimeTimer()));
    QObject::disconnect(&_idleTimer, SIGNAL(timeout()), this, SLOT(_disconnectIdle()));
    QObject::disconnect(&_timeTimer, SIGNAL(timeout()), this, SLOT(_disconnectTime()));
    _client = NULL;
}
