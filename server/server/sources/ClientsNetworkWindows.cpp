#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include "Log.h"
#include "Mutex.h"
#include "ClientsNetworkWindows.h"
#include <limits>
#ifdef Q_OS_WIN
#include <Ws2tcpip.h>
#undef max

ClientsNetworkWindows::ClientsNetworkWindows()
    : _wsaPollTimeout(1000)
    , _disableWriteBuffer(true)
{
    // Creates the interrupt sockets
    _interrupt.serverSocket = INVALID_SOCKET;
    _interrupt.clientSocket = INVALID_SOCKET;
    isInitialized(_createInterruptSockets());
}

ClientsNetworkWindows::~ClientsNetworkWindows()
{
    close();
}

void ClientsNetworkWindows::execute()
{
    int result;
    QVector<char> buf(64 * 1024);
    int sockAddrLen;
    SocketTcpWindows *socketTcp;
    SocketUdpWindows *socketUdp;
    int timeout = _wsaPollTimeout;

    _listening = true;
    while (_listening)
    {
        if ((result = WSAPoll(_fds.data(), _fds.size(), timeout)) == SOCKET_ERROR && WSAGetLastError() != WSAEINVAL && WSAGetLastError() != WSAENOTSOCK)
        {
            LOG_ERROR("An error occured while executing the TCP server: WSAPoll failed", Properties("error", WSAGetLastError()), "ClientsNetworkWindows", "execute");
            break;
        }
        timeout = _wsaPollTimeout;

        // WSAPoll was interrupted
        if (_fds[0].revents)
        {
            // Removes the datagrams
            if (_fds[0].revents & POLLRDNORM)
            {
                sockAddrLen = buf.size();
                while (recvfrom(_fds[0].fd, buf.data(), buf.size(), 0, (sockaddr *)buf.data(), &sockAddrLen) > 0)
                    ;
            }
            // There is a problem with the server interrupt socket
            if (_fds[0].revents & (POLLERR | POLLHUP | POLLNVAL))
            {
                if (_listening)
                    Log::error("An error occured with the interrupt socket", "ClientsNetworkWindows", "execute");
                break;
            }
            --result;
        }

        // Checks the state of the connection of each connecting sockets (WSAPoll won't tell us if a connection failed)
        if (result == 0 && !_connections.isEmpty())
        {
            int error = 0;
            int errorSize = sizeof(error);
            QMutableMapIterator<qint64, int> it(_connections);
            while (it.hasNext())
            {
                it.next();
                if ((getsockopt(_fds[it.value()].fd, SOL_SOCKET, SO_ERROR, (char *)&error, &errorSize)) == SOCKET_ERROR || error)
                {
                    _connectionFailed(it.value());
                    it.remove();
                }
            }
        }

        // Checks the other clients
        if (result > 0)
            for (int i = 1; i < _fds.size(); ++i)
                if (_fds[i].revents)
                {
                    if (_sockets[i - 1]->transport() == LightBird::INetwork::TCP)
                    {
                        socketTcp = static_cast<SocketTcpWindows *>(_sockets[i - 1].data());
                        if (socketTcp->isConnecting())
                            _connected(socketTcp, i);
                        else
                        {
                            if (_fds[i].revents & POLLRDNORM)
                                socketTcp->readyRead();
                            if (_fds[i].revents & POLLWRNORM)
                                socketTcp->readyWrite();
                            if (_fds[i].revents & (POLLERR | POLLHUP | POLLNVAL))
                                _disconnect(socketTcp, i);
                        }
                    }
                    else
                    {
                        socketUdp = static_cast<SocketUdpWindows *>(_sockets[i - 1].data());
                        if (_fds[i].revents & POLLRDNORM)
                            socketUdp->readyRead();
                        if (_fds[i].revents & POLLWRNORM)
                            socketUdp->readyWrite();
                        if (_fds[i].revents & (POLLERR | POLLHUP | POLLNVAL))
                            _disconnect(socketUdp, i);
                    }
                    if (--result <= 0)
                        break;
                }

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
                QMutableMapIterator<qint64, int> it(_connections);
                it.next();
                it.remove();
                if (_connections.isEmpty())
                    break;
            }
            // Sets the next timeout
            if (!_connections.isEmpty())
                timeout = qMin(_wsaPollTimeout, int(_connections.firstKey() - currentTime));
        }
    }
    _disconnectAll();
}

void ClientsNetworkWindows::addSocket(QSharedPointer<Socket> socket, int wait)
{
    Mutex mutex(_socketsToAddMutex, "ClientsNetworkWindows", "addSocket");
    if (!mutex)
        return ;
    _socketsToAdd.append(qMakePair(socket, wait));
    mutex.unlock();
    _interruptPoll();
}

void ClientsNetworkWindows::_addSockets()
{
    Mutex mutex(_socketsToAddMutex, "ClientsNetworkWindows", "_addSockets");
    if (!mutex)
        return ;

    for (QMutableListIterator<QPair<QSharedPointer<Socket>, int> > it(_socketsToAdd); it.hasNext(); it.next())
    {
        WSAPOLLFD fd;
        QSharedPointer<Socket> &socket = it.peekNext().first;
        qint64 wait = it.peekNext().second;

        // In TCP we need to connect check if the socket is connected first
        if (socket->transport() == LightBird::INetwork::TCP)
            fd.events = POLLRDNORM | POLLWRNORM;
        else
            fd.events = POLLRDNORM;
        fd.fd = socket->descriptor();
        _fds.append(fd);
        _sockets.append(socket);

        // Perpares the TCP connection
        if (socket->transport() == LightBird::INetwork::TCP)
        {
            if (wait < 0)
                wait = std::numeric_limits<int>::max();
            _connections.insert(QDateTime::currentMSecsSinceEpoch() + wait, _fds.size() - 1);
            static_cast<SocketTcpWindows *>(socket.data())->setFdEvents(&_fds.last().events);
        }
        else
            static_cast<SocketUdpWindows *>(socket.data())->setFdEvents(&_fds.last().events);
    }
    _socketsToAdd.clear();
}

void ClientsNetworkWindows::close()
{
    if (_listening && _interrupt.clientSocket != INVALID_SOCKET)
        _interruptPoll();
    _listening = false;
    if (_interrupt.serverSocket != INVALID_SOCKET)
        closesocket(_interrupt.serverSocket);
    _interrupt.serverSocket = INVALID_SOCKET;
    if (_interrupt.clientSocket != INVALID_SOCKET)
        closesocket(_interrupt.clientSocket);
    _interrupt.clientSocket = INVALID_SOCKET;
}

void ClientsNetworkWindows::_connected(SocketTcpWindows *socketTcp, int i)
{
    bool result = ((_fds[i].revents & (POLLRDNORM | POLLWRNORM)) != 0 || ((_fds[i].revents & (POLLERR | POLLHUP | POLLNVAL)) == 0));

    // Sets the result of the connection
    if (result)
        socketTcp->connected(true);
    else
        _connectionFailed(i);

    // Removes the socket from the connections list
    QMutableMapIterator<qint64, int> it(_connections);
    while (it.hasNext())
    {
        it.next();
        if (it.value() == i)
        {
            it.remove();
            break;
        }
    }
}

void ClientsNetworkWindows::_connectionFailed(int i)
{
    SocketTcpWindows *socketTcp = static_cast<SocketTcpWindows *>(_sockets[i - 1].data());
    socketTcp->connected(false);
    socketTcp->setFdEvents(NULL);
    _fds.remove(i);
    _sockets.removeAt(i - 1);
}

void ClientsNetworkWindows::_disconnect(SocketTcpWindows *socketTcp, int &i)
{
    socketTcp->disconnected();
    socketTcp->setFdEvents(NULL);
    _sockets.removeAt(i - 1);
    _fds.remove(i--);
}

void ClientsNetworkWindows::_disconnect(SocketUdpWindows *socketUdp, int &i)
{
    socketUdp->disconnected();
    socketUdp->setFdEvents(NULL);
    _sockets.removeAt(i - 1);
    _fds.remove(i--);
}

void ClientsNetworkWindows::_disconnectAll()
{
    close();
    for (int i = 1; i < _fds.size(); ++i)
    {
        if (_sockets[i - 1]->transport() == LightBird::INetwork::TCP)
        {
            SocketTcpWindows *socketTcp = static_cast<SocketTcpWindows *>(_sockets[i - 1].data());
            if (!_connections.keys(i).isEmpty())
                socketTcp->connected(false);
            else
                socketTcp->disconnected();
            socketTcp->setFdEvents(NULL);
        }
        else
        {
            SocketUdpWindows *socketUdp = static_cast<SocketUdpWindows *>(_sockets[i - 1].data());
            socketUdp->disconnected();
            socketUdp->setFdEvents(NULL);
        }
    }
    _fds.clear();
    _sockets.clear();
    _connections.clear();
    _listening = false;
    LOG_DEBUG("Clients network disconnected", "ClientsNetworkWindows", "_disconnectAll");
}

bool ClientsNetworkWindows::_createInterruptSockets()
{
    // Creates the server interrupt socket
    {
        // Creates the socket to listen to
        if ((_interrupt.serverSocket = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP)) == INVALID_SOCKET)
        {
            LOG_ERROR("Server socket() failed", Properties("error", WSAGetLastError()), "ClientsNetworkWindows", "_createInterruptSockets");
            return false;
        }

        // Turns IPV6_V6ONLY off to allow dual stack socket (IPv4 + IPv6)
        int ipv6only = 0;
        if (setsockopt(_interrupt.serverSocket, IPPROTO_IPV6, IPV6_V6ONLY, (char *)&ipv6only, sizeof(ipv6only)) == SOCKET_ERROR)
        {
            LOG_ERROR("setsockopt IPV6_V6ONLY failed", Properties("error", WSAGetLastError()), "ClientsNetworkWindows", "_createInterruptSockets");
            return false;
        }

        // Resolves the local address and port to be used by the server
        struct addrinfo *addrInfo = NULL, hints;
        ZeroMemory(&hints, sizeof(hints));
        hints.ai_family = AF_INET6;
        hints.ai_socktype = SOCK_DGRAM;
        hints.ai_protocol = IPPROTO_UDP;
        hints.ai_flags = AI_PASSIVE;
        int addrinfoResult;
        if ((addrinfoResult = getaddrinfo("::1", "0", &hints, &addrInfo)))
        {
            LOG_ERROR("Server getaddrinfo failed", Properties("error", addrinfoResult), "ClientsNetworkWindows", "_createInterruptSockets");
            return false;
        }

        // Binds the TCP listening socket to the address and port
        if (bind(_interrupt.serverSocket, addrInfo->ai_addr, (int)addrInfo->ai_addrlen) == SOCKET_ERROR)
        {
            LOG_ERROR("Unable to find a valid port to bind the server interrupt socket", Properties("error", WSAGetLastError()), "ClientsNetworkWindows", "_createInterruptSockets");
            freeaddrinfo(addrInfo);
            return false;
        }
        freeaddrinfo(addrInfo);

        // Gets the address of the server socket
        int addrlen = sizeof(_interrupt.serverAddr);
        if(getsockname(_interrupt.serverSocket, (struct sockaddr *)&_interrupt.serverAddr, &addrlen) == SOCKET_ERROR)
        {
            LOG_ERROR("Server getsockname failed", Properties("error", WSAGetLastError()), "ClientsNetworkWindows", "_createInterruptSockets");
            return false;
        }
    }

    // Creates the client interrupt socket
    {
        // Creates the socket
        if ((_interrupt.clientSocket = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP)) == INVALID_SOCKET)
        {
            LOG_ERROR("Client socket() failed", Properties("error", WSAGetLastError()), "ClientsNetworkWindows", "_createInterruptSockets");
            return false;
        }

        // Disables the write buffer
        int sndbuff = 0;
        if (_disableWriteBuffer && setsockopt(_interrupt.clientSocket, SOL_SOCKET, SO_SNDBUF, (char *)&sndbuff, sizeof(sndbuff)) == SOCKET_ERROR)
        {
            LOG_ERROR("Client setsockopt SO_SNDBUF failed", Properties("error", WSAGetLastError()), "ServerTcpWindows", "_createInterruptSockets");
            return false;
        }
    }

    // Non blocking mode
    u_long nonBlockingMode = 1;
    if ((ioctlsocket(_interrupt.serverSocket, FIONBIO, &nonBlockingMode)) == SOCKET_ERROR ||
        (ioctlsocket(_interrupt.clientSocket, FIONBIO, &nonBlockingMode)) == SOCKET_ERROR)
    {
        LOG_ERROR("Failed to set the sockets in non blocking mode", Properties("error", WSAGetLastError()), "ClientsNetworkWindows", "_createInterruptSockets");
        return false;
    }

    // Adds the server interrupt socket to the FDs
    WSAPOLLFD fd;
    fd.events = POLLRDNORM;
    fd.fd = _interrupt.serverSocket;
    _fds.append(fd);
    return true;
}

void ClientsNetworkWindows::_interruptPoll()
{
    sendto(_interrupt.clientSocket, " ", 1, 0, (sockaddr *)&_interrupt.serverAddr, sizeof(_interrupt.serverAddr));
}

#endif // Q_OS_WIN
