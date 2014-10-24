#include "Log.h"
#include "Mutex.h"
#include "ClientsNetworkLinux.h"
#include <limits>
#ifdef Q_OS_LINUX
#include <errno.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <sys/eventfd.h>

ClientsNetworkLinux::ClientsNetworkLinux()
    : _disableWriteBuffer(true)
    , _epoll(-1)
    , _eventfd(-1)
{
    // Creates the epoll
    if ((_epoll = epoll_create1(0)) == -1)
    {
        LOG_ERROR("Failed to create the epoll: epoll_create1 failed", Properties("error", errno), "ClientsNetworkLinux", "ClientsNetworkLinux");
        return ;
    }

    // Creates the eventfd that allows to interrupt epoll_wait
    if ((_eventfd = eventfd(0, 0)) == -1)
    {
        LOG_ERROR("Error with eventfd", Properties("error", errno), "ClientsNetworkLinux", "ClientsNetworkLinux");
        return ;
    }
    u_long nonBlockingMode = 1;
    if ((ioctl(_eventfd, FIONBIO, &nonBlockingMode)) == -1)
    {
        LOG_DEBUG("Failed to set eventFd in non blocking mode", Properties("error", errno), "ClientsNetworkLinux", "ClientsNetworkLinux");
        return ;
    }
    epoll_event e;
    memset(&e, 0, sizeof(e));
    e.events = EPOLLIN;
    if (epoll_ctl(_epoll, EPOLL_CTL_ADD, _eventfd, &e) == -1)
    {
        LOG_ERROR("Failed to add the eventFd to epoll: epoll_ctl EPOLL_CTL_ADD failed", Properties("error", errno), "ClientsNetworkLinux", "ClientsNetworkLinux");
        return ;
    }

    isInitialized(true);
}

ClientsNetworkLinux::~ClientsNetworkLinux()
{
    _close();
}

void ClientsNetworkLinux::execute()
{
    int result;
    QVector<epoll_event> events(1);
    events.reserve(200);
    SocketTcpLinux *socketTcp;
    SocketUdpLinux *socketUdp;
    int timeout = -1;

    _listening = true;
    while (_listening)
    {
        if (events.size() != _sockets.size() && _sockets.size() > 0)
            events.resize(_sockets.size());
        if ((result = epoll_wait(_epoll, events.data(), events.size(), timeout)) == -1 && errno != EINTR)
        {
            LOG_ERROR("Error with epoll_wait", Properties("error", errno), "ClientsNetworkLinux", "execute");
            break;
        }
        timeout = -1;

        // Checks the events
        for (int i = 0; i < result; ++i)
        {
            QSharedPointer<Socket> *socket = static_cast<QSharedPointer<Socket> *>(events[i].data.ptr);

            // epoll_wait was interrupted by the eventfd
            if (!socket)
            {
                if (events[i].events & EPOLLIN)
                {
                    quint64 buf;
                    read(_eventfd, &buf, sizeof(buf));
                    continue;
                }
                else
                {
                    LOG_ERROR("Error with the eventfd", Properties("error", events[i].events), "ClientsNetworkLinux", "execute");
                    _listening = false;
                }
            }

            // The TCP socket
            if (socket->data()->transport() == LightBird::INetwork::TCP)
            {
                socketTcp = static_cast<SocketTcpLinux *>(socket->data());
                if (socketTcp->isConnecting())
                    _connected(socketTcp, events[i]);
                else
                {
                    if (events[i].events & EPOLLIN)
                        socketTcp->readyRead();
                    if (events[i].events & EPOLLOUT)
                        socketTcp->readyWrite();
                    if (events[i].events & (EPOLLERR | EPOLLHUP | EPOLLRDHUP))
                        _disconnect(socketTcp, events[i]);
                }
            }

            // The UDP socket
            else
            {
                socketUdp = static_cast<SocketUdpLinux *>(socket->data());
                if (events[i].events & EPOLLIN)
                    socketUdp->readyRead();
                if (events[i].events & EPOLLOUT)
                    socketUdp->readyWrite();
                if (events[i].events & (EPOLLERR | EPOLLHUP | EPOLLRDHUP))
                    _disconnect(socketUdp, events[i]);
            }
        }

        // Checks if there is any sockets to close
        if (!_socketsToClose.isEmpty())
            _closeSockets();

        // Checks if there is any sockets to add
        if (!_socketsToAdd.isEmpty())
            _addSockets();

        // Checks the timeout of the connections
        if (!_connections.isEmpty())
        {
            qint64 currentTime = QDateTime::currentMSecsSinceEpoch();
            // The socket could not connect to the client in the given time
            while (_connections.firstKey() - currentTime <= 0)
            {
                _connectionFailed(_connections.first());
                QMutableMapIterator<qint64, void *> it(_connections);
                it.next();
                it.remove();
                if (_connections.isEmpty())
                    break;
            }
            // Sets the next timeout
            if (!_connections.isEmpty())
                timeout = int(_connections.firstKey() - currentTime);
        }
    }
    _disconnectAll();
}

void ClientsNetworkLinux::addSocket(QSharedPointer<Socket> socket, int wait)
{
    Mutex mutex(_socketsToAddMutex, "ClientsNetworkLinux", "addSocket");
    if (!mutex)
        return ;
    _socketsToAdd.append(qMakePair(socket, wait));
    mutex.unlock();
    _interruptEpoll();
}

void ClientsNetworkLinux::_addSockets()
{
    Mutex mutex(_socketsToAddMutex, "ClientsNetworkLinux", "_addSockets");
    if (!mutex)
        return ;

    for (QMutableListIterator<QPair<QSharedPointer<Socket>, int> > it(_socketsToAdd); it.hasNext(); it.next())
    {
        QSharedPointer<Socket> &socket = it.peekNext().first;
        qint64 wait = it.peekNext().second;
        epoll_event e;
        memset(&e, 0, sizeof(e));

        // Adds the socket to epoll
        e.data.ptr = new QSharedPointer<Socket>(socket);
        e.events = EPOLLIN | EPOLLRDHUP;
        if (socket->transport() == LightBird::INetwork::TCP)
            e.events |= EPOLLOUT;
        if (epoll_ctl(_epoll, EPOLL_CTL_ADD, socket->descriptor(), &e) == -1)
        {
            LOG_ERROR("Failed to add the socket to epoll: epoll_ctl EPOLL_CTL_ADD failed", Properties("error", errno).add("socket", socket->descriptor()), "ClientsNetworkLinux", "_addSockets");
            _connectionFailed(e.data.ptr);
            return ;
        }
        _sockets.append(e);

        // Perpares the TCP connection#include <signal.h>
        if (socket->transport() == LightBird::INetwork::TCP)
        {
            if (wait < 0)
                wait = std::numeric_limits<int>::max();
            _connections.insert(QDateTime::currentMSecsSinceEpoch() + wait, e.data.ptr);
            SocketTcpLinux *socketTcpLinux = static_cast<SocketTcpLinux *>(socket.data());
            socketTcpLinux->setEpollEvents(&_sockets.last());
            QObject::connect(socketTcpLinux, SIGNAL(setEpollEvents(epoll_event&)), this, SLOT(_setEpollEvents(epoll_event&)), Qt::DirectConnection);
            QObject::connect(socketTcpLinux, SIGNAL(closed(epoll_event&)), this, SLOT(_socketClosed(epoll_event&)), Qt::DirectConnection);
        }
        else
        {
            SocketUdpLinux *socketUdpLinux = static_cast<SocketUdpLinux *>(socket.data());
            socketUdpLinux->setEpollEvents(&_sockets.last());
            QObject::connect(socketUdpLinux, SIGNAL(setEpollEvents(epoll_event&)), this, SLOT(_setEpollEvents(epoll_event&)), Qt::DirectConnection);
            QObject::connect(socketUdpLinux, SIGNAL(closed(epoll_event&)), this, SLOT(_socketClosed(epoll_event&)), Qt::DirectConnection);
        }
    }
    _socketsToAdd.clear();
}

void ClientsNetworkLinux::close()
{
    if (_listening)
    {
        _listening = false;
        if (_eventfd != -1)
            _interruptEpoll();
    }
}

void ClientsNetworkLinux::_connected(SocketTcpLinux *socketTcp, epoll_event &e)
{
    bool result = ((e.events & (EPOLLOUT | EPOLLIN)) != 0 || ((e.events & (EPOLLERR | EPOLLHUP | EPOLLRDHUP)) == 0));

    // Sets the result of the connection
    if (result)
        socketTcp->connected(true);
    else
        _connectionFailed(e.data.ptr);

    // Removes the socket from the connections list
    QMutableMapIterator<qint64, void *> it(_connections);
    while (it.hasNext())
    {
        it.next();
        if (it.value() == e.data.ptr)
        {
            it.remove();
            break;
        }
    }
}

void ClientsNetworkLinux::_connectionFailed(void *s)
{
    for (int i = 0; i < _sockets.size(); ++i)
        if (_sockets[i].data.ptr == s)
        {
            QSharedPointer<Socket> *socketShared = static_cast<QSharedPointer<Socket> *>(s);
            if (epoll_ctl(_epoll, EPOLL_CTL_DEL, socketShared->data()->descriptor(), &_sockets[i]) == -1)
                LOG_WARNING("Failed to delete the socket from epoll: epoll_ctl EPOLL_CTL_DEL failed", Properties("error", errno).add("socket", socketShared->data()->descriptor()), "ClientsNetworkLinux", "_connectionFailed");
            SocketTcpLinux *socketTcp = static_cast<SocketTcpLinux *>(socketShared->data());
            socketTcp->connected(false);
            QObject::disconnect(socketTcp, SIGNAL(setEpollEvents(epoll_event&)), this, SLOT(_setEpollEvents(epoll_event&)));
            QObject::disconnect(socketTcp, SIGNAL(closed(epoll_event&)), this, SLOT(_socketClosed(epoll_event&)));
            socketTcp->setEpollEvents(NULL);
            delete socketShared;
            _sockets.removeAt(i);
            break;
        }
}

void ClientsNetworkLinux::_disconnect(SocketTcpLinux *socketTcp, epoll_event &e)
{
    for (int i = 0; i < _sockets.size(); ++i)
        if (_sockets[i].data.ptr == e.data.ptr)
        {
            QSharedPointer<Socket> *socketShared = static_cast<QSharedPointer<Socket> *>(e.data.ptr);
            if (epoll_ctl(_epoll, EPOLL_CTL_DEL, socketTcp->descriptor(), &e) == -1)
                LOG_WARNING("Failed to delete the TCP socket from epoll: epoll_ctl EPOLL_CTL_DEL failed", Properties("error", errno).add("socket", socketTcp->descriptor()), "ClientsNetworkLinux", "_disconnect");
            socketTcp->disconnected();
            QObject::disconnect(socketTcp, SIGNAL(setEpollEvents(epoll_event&)), this, SLOT(_setEpollEvents(epoll_event&)));
            QObject::disconnect(socketTcp, SIGNAL(closed(epoll_event&)), this, SLOT(_socketClosed(epoll_event&)));
            socketTcp->setEpollEvents(NULL);
            delete socketShared;
            _sockets.removeAt(i);
            break;
        }
}

void ClientsNetworkLinux::_disconnect(SocketUdpLinux *socketUdp, epoll_event &e)
{
    for (int i = 0; i < _sockets.size(); ++i)
        if (_sockets[i].data.ptr == e.data.ptr)
        {
            QSharedPointer<Socket> *socketShared = static_cast<QSharedPointer<Socket> *>(e.data.ptr);
            if (epoll_ctl(_epoll, EPOLL_CTL_DEL, socketUdp->descriptor(), &e) == -1)
                LOG_WARNING("Failed to delete the UDP socket from epoll: epoll_ctl EPOLL_CTL_DEL failed", Properties("error", errno).add("socket", socketUdp->descriptor()), "ClientsNetworkLinux", "_disconnect");
            socketUdp->disconnected();
            QObject::disconnect(socketUdp, SIGNAL(setEpollEvents(epoll_event&)), this, SLOT(_setEpollEvents(epoll_event&)));
            QObject::disconnect(socketUdp, SIGNAL(closed(epoll_event&)), this, SLOT(_socketClosed(epoll_event&)));
            socketUdp->setEpollEvents(NULL);
            delete socketShared;
            _sockets.removeAt(i);
            break;
        }
}

void ClientsNetworkLinux::_disconnectAll()
{
    _close();
    for (int i = 1; i < _sockets.size(); ++i)
    {
        QSharedPointer<Socket> *socketShared = static_cast<QSharedPointer<Socket> *>(_sockets[i].data.ptr);
        if (epoll_ctl(_epoll, EPOLL_CTL_DEL, socketShared->data()->descriptor(), &_sockets[i]) == -1)
            LOG_WARNING("Failed to delete the socket from epoll: epoll_ctl EPOLL_CTL_DEL failed", Properties("error", errno).add("socket", socketShared->data()->descriptor()), "ClientsNetworkLinux", "_disconnectAll");
        if (socketShared->data()->transport() == LightBird::INetwork::TCP)
        {
            SocketTcpLinux *socketTcp = static_cast<SocketTcpLinux *>(socketShared->data());
            if (!_connections.keys(_sockets[i].data.ptr).isEmpty())
                socketTcp->connected(false);
            else
                socketTcp->disconnected();
            QObject::disconnect(socketTcp, SIGNAL(setEpollEvents(epoll_event&)), this, SLOT(_setEpollEvents(epoll_event&)));
            QObject::disconnect(socketTcp, SIGNAL(closed(epoll_event&)), this, SLOT(_socketClosed(epoll_event&)));
            socketTcp->setEpollEvents(NULL);
        }
        else
        {
            SocketUdpLinux *socketUdp = static_cast<SocketUdpLinux *>(socketShared->data());
            socketUdp->disconnected();
            QObject::disconnect(socketUdp, SIGNAL(setEpollEvents(epoll_event&)), this, SLOT(_setEpollEvents(epoll_event&)));
            QObject::disconnect(socketUdp, SIGNAL(closed(epoll_event&)), this, SLOT(_socketClosed(epoll_event&)));
            socketUdp->setEpollEvents(NULL);
        }
        delete socketShared;
    }
    _sockets.clear();
    _connections.clear();
    _listening = false;
    LOG_DEBUG("Clients network disconnected", "ClientsNetworkLinux", "_disconnectAll");
}

void ClientsNetworkLinux::_setEpollEvents(epoll_event &events)
{
    if (epoll_ctl(_epoll, EPOLL_CTL_MOD, static_cast<QSharedPointer<Socket> *>(events.data.ptr)->data()->descriptor(), &events) == -1)
    {
        LOG_ERROR("Failed to modify the socket from epoll: epoll_ctl EPOLL_CTL_MOD failed", Properties("error", errno).add("socket", static_cast<QSharedPointer<Socket> *>(events.data.ptr)->data()->descriptor()), "ClientsNetworkLinux", "setEvents");
        ::close(static_cast<QSharedPointer<Socket> *>(events.data.ptr)->data()->descriptor());
        return ;
    }
}

void ClientsNetworkLinux::_socketClosed(epoll_event &events)
{
    Mutex mutex(_socketsToCloseMutex, "ClientsNetworkLinux", "_socketClosed");
    if (!mutex)
        return ;

    if (events.data.ptr)
        _socketsToClose.append(events.data.ptr);
    mutex.unlock();
    _interruptEpoll();
}

void ClientsNetworkLinux::_closeSockets()
{
    Mutex mutex(_socketsToCloseMutex, "ClientsNetworkLinux", "_closeSockets");
    if (!mutex)
        return ;

    for (QListIterator<void *> it(_socketsToClose); it.hasNext(); it.next())
    {
        for (int i = 0; i < _sockets.size(); ++i)
            if (_sockets[i].data.ptr == it.peekNext())
            {
                QSharedPointer<Socket> *socketShared = static_cast<QSharedPointer<Socket> *>(it.peekNext());
                if (socketShared->data()->transport() == LightBird::INetwork::TCP)
                {
                    SocketTcpLinux *socketTcp = static_cast<SocketTcpLinux *>(socketShared->data());
                    socketTcp->disconnected();
                    QObject::disconnect(socketTcp, SIGNAL(setEpollEvents(epoll_event&)), this, SLOT(_setEpollEvents(epoll_event&)));
                    QObject::disconnect(socketTcp, SIGNAL(closed(epoll_event&)), this, SLOT(_socketClosed(epoll_event&)));
                    socketTcp->setEpollEvents(NULL);
                }
                else
                {
                    SocketUdpLinux *socketUdp = static_cast<SocketUdpLinux *>(socketShared->data());
                    socketUdp->disconnected();
                    QObject::disconnect(socketUdp, SIGNAL(setEpollEvents(epoll_event&)), this, SLOT(_setEpollEvents(epoll_event&)));
                    QObject::disconnect(socketUdp, SIGNAL(closed(epoll_event&)), this, SLOT(_socketClosed(epoll_event&)));
                    socketUdp->setEpollEvents(NULL);
                }
                delete socketShared;
                _sockets.removeAt(i);
                break;
            }
    }
    _socketsToClose.clear();
}

void ClientsNetworkLinux::_interruptEpoll()
{
    quint64 buf = 1;
    write(_eventfd, &buf, sizeof(buf));
}

void ClientsNetworkLinux::_close()
{
    _listening = false;
    if (_epoll != -1)
    {
        ::close(_epoll);
        _epoll = -1;
    }
    if (_eventfd != -1)
    {
        ::close(_eventfd);
        _eventfd = -1;
    }
}

#endif // Q_OS_LINUX
