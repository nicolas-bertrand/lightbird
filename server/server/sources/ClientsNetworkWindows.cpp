#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include "Log.h"
#include "ClientsNetworkWindows.h"
#ifdef Q_OS_WIN
#include <Ws2tcpip.h>

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
    //SocketUdpWindows *socketUdp;

    _listening = true;
    while (_listening)
    {
        if ((result = WSAPoll(_fds.data(), _fds.size(), _wsaPollTimeout)) == SOCKET_ERROR)
        {
            LOG_ERROR("An error occured while executing the TCP server: WSAPoll failed", Properties("error", WSAGetLastError()), "ClientsNetworkWindows", "execute");
            break;
        }

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
                Log::error("An error occured with the interrupt socket", "ClientsNetworkWindows", "execute");
                break;
            }
            --result;
        }

        // Checks the state of the connection of each connecting sockets (WSAPoll won't tell us if a connection failed)
        if (result == 0 && _connections.size())
        {
            int error = 0;
            int errorSize = sizeof(error);
            for (int i = 0, s = _connections.size(); i < s; ++i)
                if ((getsockopt(_fds[_connections[i]].fd, SOL_SOCKET, SO_ERROR, (char *)&error, &errorSize)) == SOCKET_ERROR || error)
                {
                    int j = _connections[i];
                    socketTcp = static_cast<SocketTcpWindows *>(_sockets[j - 1].data());
                    socketTcp->connected(false);
                    socketTcp->setFdEvents(NULL);
                    _fds.remove(j);
                    _sockets.removeAt(j - 1);
                    _connections.removeAt(i--);
                    s--;
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
                        {
                            socketTcp->connected((_fds[i].revents & (POLLERR | POLLHUP | POLLNVAL)) == 0);
                            _connections.removeOne(i);
                        }
                        else
                        {
                            if (_fds[i].revents & POLLRDNORM)
                            {
                                Log::fatal("READ " + QString::number(socketTcp->size()));
                                socketTcp->readyRead();
                            }
                            if (_fds[i].revents & POLLWRNORM)
                            {
                                Log::fatal("WRITE");
                                socketTcp->readyWrite();
                            }
                            if (_fds[i].revents & (POLLERR | POLLHUP | POLLNVAL))
                            {
                                Log::fatal("DISCONNECT");
                                _disconnect(socketTcp, i);
                            }
                        }
                    }
                    else
                    {

                    }
                    if (--result <= 0)
                        break;
                }
    }
    _disconnectAll();
    return ;
}

void ClientsNetworkWindows::addSocket(QSharedPointer<Socket> socket, int wait)
{
    WSAPOLLFD fd;
    // In TCP we need to connect check if the socket is connected first
    if (socket->transport() == LightBird::INetwork::TCP)
        fd.events = POLLRDNORM | POLLWRNORM;
    else
        fd.events = POLLRDNORM;
    fd.fd = socket->descriptor();
    _fds.append(fd);
    _sockets.append(socket);
    _connections.append(_fds.size() - 1);
    if (socket->transport() == LightBird::INetwork::TCP)
        static_cast<SocketTcpWindows *>(socket.data())->setFdEvents(&_fds.last().events);
    _interruptPoll();
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

void ClientsNetworkWindows::_disconnect(SocketTcpWindows *socketTcp, int &i)
{
    socketTcp->disconnected();
    socketTcp->setFdEvents(NULL);
    _sockets.removeAt(i - 1);
    _fds.remove(i--);
}

void ClientsNetworkWindows::_disconnectAll()
{
    close();
    for (int i = 1; i < _fds.size(); ++i)
    {
        SocketTcpWindows *socketTcp = static_cast<SocketTcpWindows *>(_sockets[i - 1].data());
        if (_connections.removeOne(i))
            socketTcp->connected(false);
        else
            socketTcp->disconnected();
        socketTcp->setFdEvents(NULL);
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
        ZeroMemory(&hints, sizeof (hints));
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
            LOG_ERROR("Unable to find a valid port to bind the server interrupt socket", "ClientsNetworkWindows", "_createInterruptSockets");
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
