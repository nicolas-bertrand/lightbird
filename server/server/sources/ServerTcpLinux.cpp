#include "Log.h"
#include "ServerTcpLinux.h"
#include "SocketTcpLinux.h"
#include "Mutex.h"
#ifdef Q_OS_LINUX
#include <sys/socket.h>
#include <netdb.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <arpa/inet.h>

ServerTcpLinux::ServerTcpLinux(quint16 p, const QHostAddress &address)
    : ServerTcp(p, address)
    , _listenSocket(-1)
    , _disableWriteBuffer(true)
    , _maxPendingConnections(1000)
    , _epoll(-1)
{
    // Resolves the local address and port to be used by the server
    QByteArray port = QByteArray::number(_port);
    int addrinfoResult;
    struct addrinfo *addrInfo = NULL, hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET6;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;
    if ((addrinfoResult = getaddrinfo(NULL, port.data(), &hints, &addrInfo)))
    {
        LOG_ERROR("Can't listen the to the TCP port: getaddrinfo failed", Properties("error", addrinfoResult).add("port", _port).add("address", _address.toString()), "ServerTcpLinux", "ServerTcpLinux");
        return ;
    }

    // Creates the socket to listen to
    if ((_listenSocket = socket(addrInfo->ai_family, addrInfo->ai_socktype, addrInfo->ai_protocol)) == -1)
    {
        LOG_ERROR("Failed to create the listen socket", Properties("error", errno).add("port", _port).add("address", _address.toString()), "ServerTcpLinux", "ServerTcpLinux");
        freeaddrinfo(addrInfo);
        return ;
    }

    // Turns IPV6_V6ONLY off to allow dual stack socket (IPv4 + IPv6)
    int ipv6only = 0;
    if (setsockopt(_listenSocket, IPPROTO_IPV6, IPV6_V6ONLY, (char *)&ipv6only, sizeof(ipv6only)) == -1)
    {
        LOG_ERROR("Can't listen the to the TCP port: setsockopt IPV6_V6ONLY failed", Properties("error", errno).add("port", _port).add("address", _address.toString()), "ServerTcpLinux", "ServerTcpLinux");
        freeaddrinfo(addrInfo);
        close();
        return ;
    }

    // Binds the TCP listening socket to the address and port
    if (bind(_listenSocket, addrInfo->ai_addr, (int)addrInfo->ai_addrlen) == -1)
    {
        LOG_ERROR("Can't listen the to the TCP port: bind failed", Properties("error", errno).add("port", _port).add("address", _address.toString()), "ServerTcpLinux", "ServerTcpLinux");
        freeaddrinfo(addrInfo);
        close();
        return ;
    }
    freeaddrinfo(addrInfo);

    // Starts to listen to the port
    if (::listen(_listenSocket, SOMAXCONN) == -1)
    {
        LOG_ERROR("Can't listen the to the TCP port: listen failed", Properties("error", errno).add("port", _port).add("address", _address.toString()), "ServerTcpLinux", "ServerTcpLinux");
        close();
        return ;
    }

    // Disables the write buffer
    if (_disableWriteBuffer)
    {
        int sndbuff = 0;
        if (setsockopt(_listenSocket, SOL_SOCKET, SO_SNDBUF, (char *)&sndbuff, sizeof(sndbuff)) == -1)
            LOG_ERROR("setsockopt SO_SNDBUF failed", Properties("error", errno).add("port", _port).add("address", _address.toString()), "ServerTcpLinux", "ServerTcpLinux");
    }

    // Non blocking mode
    u_long nonBlockingMode = 1;
    if ((ioctl(_listenSocket, FIONBIO, &nonBlockingMode)) == -1)
    {
        LOG_ERROR("Failed to set the socket in non blocking mode", Properties("error", errno).add("port", _port).add("address", _address.toString()), "ServerTcpLinux", "ServerTcpLinux");
        close();
        return ;
    }

    // Creates the epoll
    if ((_epoll = epoll_create1(0)) == -1)
    {
        LOG_ERROR("Failed to create the epoll: epoll_create1 failed", Properties("error", errno).add("port", _port).add("address", _address.toString()), "ServerTcpLinux", "ServerTcpLinux");
        close();
        return ;
    }

    // Adds the listen socket to the list
    epoll_event e;
    memset(&e, 0, sizeof(e));
    e.events = EPOLLIN | EPOLLRDHUP;
    _sockets.append(e);
    if (epoll_ctl(_epoll, EPOLL_CTL_ADD, _listenSocket, &e) == -1)
    {
        LOG_ERROR("Failed to add the socket to epoll: epoll_ctl EPOLL_CTL_ADD failed", Properties("error", errno).add("port", _port).add("address", _address.toString()), "ServerTcpLinux", "ServerTcpLinux");
        close();
        return ;
    }
    _listening = true;
}

ServerTcpLinux::~ServerTcpLinux()
{
    close();
    if (_epoll != -1)
    {
        ::close(_epoll);
        _epoll = -1;
    }
}

void ServerTcpLinux::execute()
{
    int result;
    bool execute = true;
    QVector<epoll_event> events;
    events.reserve(200);

    while (execute)
    {
        if (events.size() != _sockets.size())
            events.resize(_sockets.size());
        if ((result = epoll_wait(_epoll, events.data(), events.size(), -1)) == -1 && errno != EINTR)
        {
            LOG_ERROR("An error occured while executing the TCP server: epoll_wait failed", Properties("error", errno).add("port", _port).add("address", _address.toString()), "ServerTcpLinux", "execute");
            break;
        }

        // Checks the events
        for (int i = 0; i < result; ++i)
        {
            // Checks the listen socket
            if (events[i].data.ptr == NULL)
            {
                if (events[i].events & EPOLLIN)
                    _newConnection();
                if (events[i].events & (EPOLLERR | EPOLLHUP | EPOLLRDHUP))
                    execute = false;
            }
            // Checks the other sockets
            else
            {
                SocketTcpLinux *socketTcp = static_cast<SocketTcpLinux *>(static_cast<QSharedPointer<Socket> *>(events[i].data.ptr)->data());
                if (events[i].events & EPOLLIN)
                    socketTcp->readyRead();
                if (events[i].events & EPOLLOUT)
                    socketTcp->readyWrite();
                if (events[i].events & (EPOLLERR | EPOLLHUP | EPOLLRDHUP))
                    _disconnect(events[i]);
            }
        }

        // Checks if there is any sockets to close
        if (!_socketsToClose.isEmpty())
            _closeSockets();
    }
    _disconnectAll();
}

void ServerTcpLinux::close()
{
    if (_listenSocket != -1)
    {
        ::close(_listenSocket);
        _listenSocket = -1;
    }
    _listening = false;
}

void ServerTcpLinux::_newConnection()
{
    int socket;
    sockaddr_in6 addr;
    socklen_t addrSize = sizeof(addr);

    // Accepts the new client
    if ((socket = accept(_listenSocket, (sockaddr *)&addr, &addrSize)) == -1)
    {
        LOG_DEBUG("Error with accept", Properties("error", errno).add("port", _port).add("address", _address.toString()), "ServerTcpLinux", "_newConnection");
        return ;
    }

    // Too many pending connection, client is dropped
    if (_pendingConnections.size() > _maxPendingConnections)
    {
        ::close(socket);
        LOG_DEBUG("Too many pending connection, new socket closed", Properties("error", errno).add("port", _port).add("address", _address.toString()), "ServerTcpLinux", "_newConnection");
        return ;
    }

    // Disables the write buffer
    int sndbuff = 0;
    if (_disableWriteBuffer && setsockopt(socket, SOL_SOCKET, SO_SNDBUF, (char *)&sndbuff, sizeof(sndbuff)) == -1)
    {
        LOG_ERROR("setsockopt SO_SNDBUF failed", Properties("error", errno).add("port", _port).add("address", _address.toString()), "ServerTcpLinux", "_newConnection");
        ::close(socket);
        return ;
    }

    // Gets its IP and port
    QByteArray ip = "::1";
    ip.resize(46);
    if (!inet_ntop(AF_INET6, &addr.sin6_addr, ip.data(), ip.size()))
        LOG_ERROR("inet_ntop failed", Properties("error", errno).add("port", _port).add("address", _address.toString()), "ServerTcpLinux", "_newConnection");
    QHostAddress peerAddress = QHostAddress(QString(ip.data()));
    quint16 peerPort = ntohs(addr.sin6_port);

    // Creates the TCP socket
    QSharedPointer<Socket> socketTcp(new SocketTcpLinux(peerAddress, peerPort, _port, socket));

    // Adds it to epoll
    epoll_event e;
    memset(&e, 0, sizeof(e));
    e.data.ptr = new QSharedPointer<Socket>(socketTcp);
    e.events = EPOLLIN | EPOLLRDHUP;
    if (epoll_ctl(_epoll, EPOLL_CTL_ADD, socket, &e) == -1)
    {
        LOG_ERROR("Failed to add the socket to epoll: epoll_ctl EPOLL_CTL_ADD failed", Properties("error", errno).add("port", _port).add("address", _address.toString()), "ServerTcpLinux", "_newConnection");
        ::close(socket);
        return ;
    }
    _sockets.append(e);
    SocketTcpLinux *socketTcpLinux = static_cast<SocketTcpLinux *>(socketTcp.data());
    socketTcpLinux->setEpollEvents(&_sockets.last());
    QObject::connect(socketTcpLinux, SIGNAL(setEpollEvents(epoll_event&)), this, SLOT(_setEpollEvents(epoll_event&)), Qt::DirectConnection);
    QObject::connect(socketTcpLinux, SIGNAL(closed(epoll_event&)), this, SLOT(_socketClosed(epoll_event&)), Qt::DirectConnection);
    _pendingConnections.append(socketTcp);
    emit newConnection();
}

void ServerTcpLinux::_disconnect(epoll_event &event)
{
    for (int i = 0; i < _sockets.size(); ++i)
    {
        if (_sockets[i].data.ptr == event.data.ptr)
        {
            QSharedPointer<Socket> *socketShared = static_cast<QSharedPointer<Socket> *>(event.data.ptr);
            SocketTcpLinux *socketTcp = static_cast<SocketTcpLinux *>(socketShared->data());
            epoll_event e;
            if (epoll_ctl(_epoll, EPOLL_CTL_DEL, socketTcp->descriptor(), &e) == -1)
                LOG_WARNING("Failed to delete the socket from epoll: epoll_ctl EPOLL_CTL_DEL failed", Properties("error", errno).add("port", _port).add("address", _address.toString()), "ServerTcpLinux", "_disconnect");
            if (!_pendingConnections.removeOne(*socketShared))
                socketTcp->disconnected();
            QObject::disconnect(socketTcp, SIGNAL(setEpollEvents(epoll_event&)), this, SLOT(_setEpollEvents(epoll_event&)));
            QObject::disconnect(socketTcp, SIGNAL(closed(epoll_event&)), this, SLOT(_socketClosed(epoll_event&)));
            socketTcp->setEpollEvents(NULL);
            delete socketShared;
            _sockets.removeAt(i);
            break;
        }
    }
}

void ServerTcpLinux::_disconnectAll()
{
    close();
    _pendingConnections.clear();
    for (int i = 0; i < _sockets.size(); ++i)
        if (_sockets[i].data.ptr)
        {
            QSharedPointer<Socket> *socketShared = static_cast<QSharedPointer<Socket> *>(_sockets[i].data.ptr);
            SocketTcpLinux *socketTcp = static_cast<SocketTcpLinux *>(socketShared->data());
            epoll_event e;
            if (epoll_ctl(_epoll, EPOLL_CTL_DEL, socketTcp->descriptor(), &e) == -1)
                LOG_WARNING("Failed to delete the socket from epoll: epoll_ctl EPOLL_CTL_DEL failed", Properties("error", errno).add("port", _port).add("address", _address.toString()), "ServerTcpLinux", "_disconnectAll");
            socketTcp->disconnected();
            QObject::disconnect(socketTcp, SIGNAL(setEpollEvents(epoll_event&)), this, SLOT(_setEpollEvents(epoll_event&)));
            QObject::disconnect(socketTcp, SIGNAL(closed(epoll_event&)), this, SLOT(_socketClosed(epoll_event&)));
            socketTcp->setEpollEvents(NULL);
            delete socketShared;
        }
    _sockets.clear();
    LOG_DEBUG("Listen socket disconnected", Properties("port", _port).add("address", _address.toString()), "ServerTcpLinux", "_disconnectAll");
}

void ServerTcpLinux::_setEpollEvents(epoll_event &events)
{
    if (epoll_ctl(_epoll, EPOLL_CTL_MOD, static_cast<QSharedPointer<Socket> *>(events.data.ptr)->data()->descriptor(), &events) == -1)
    {
        LOG_ERROR("Failed to modify the socket from epoll: epoll_ctl EPOLL_CTL_MOD failed", Properties("error", errno).add("port", _port).add("address", _address.toString()), "ServerTcpLinux", "setEvents");
        ::close(static_cast<QSharedPointer<Socket> *>(events.data.ptr)->data()->descriptor());
        return ;
    }
}

void ServerTcpLinux::_socketClosed(epoll_event &events)
{
    Mutex mutex(_socketsToCloseMutex, "ServerTcpLinux", "_socketClosed");
    if (!mutex)
        return ;

    if (events.data.ptr)
        _socketsToClose.append(events.data.ptr);
    mutex.unlock();
}

void ServerTcpLinux::_closeSockets()
{
    Mutex mutex(_socketsToCloseMutex, "ServerTcpLinux", "_closeSockets");
    if (!mutex)
        return ;

    for (QListIterator<void *> it(_socketsToClose); it.hasNext(); it.next())
    {
        for (int i = 0; i < _sockets.size(); ++i)
            if (_sockets[i].data.ptr == it.peekNext())
            {
                QSharedPointer<Socket> *socketShared = static_cast<QSharedPointer<Socket> *>(it.peekNext());
                SocketTcpLinux *socketTcp = static_cast<SocketTcpLinux *>(socketShared->data());
                socketTcp->disconnected();
                QObject::disconnect(socketTcp, SIGNAL(setEpollEvents(epoll_event&)), this, SLOT(_setEpollEvents(epoll_event&)));
                QObject::disconnect(socketTcp, SIGNAL(closed(epoll_event&)), this, SLOT(_socketClosed(epoll_event&)));
                socketTcp->setEpollEvents(NULL);
                delete socketShared;
                _sockets.removeAt(i);
                break;
            }
    }
    _socketsToClose.clear();
}

#endif // Q_OS_LINUX
