#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include "Log.h"
#include "ServerTcpWindows.h"
#ifdef Q_OS_WIN
#include <Ws2tcpip.h>

ServerTcpWindows::ServerTcpWindows(quint16 p, const QHostAddress &address)
    : ServerTcp(p, address)
    , _listenSocket(INVALID_SOCKET)
    , _disableWriteBuffer(true)
    , _wsaPollTimeout(1000)
    , _maxPendingConnections(1000)
    , _regexIPv4("^\\[.*fff:(.*)\\]:(\\d+)$")
    , _regexIPv6("^\\[(.*)\\]:(\\d+)$")
{
    // Resolves the local address and port to be used by the server
    QByteArray port = QByteArray::number(_port);
    int addrinfoResult;
    struct addrinfo *addrInfo = NULL, hints;
    ZeroMemory(&hints, sizeof (hints));
    hints.ai_family = AF_INET6;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;
    if ((addrinfoResult = getaddrinfo(NULL, port.data(), &hints, &addrInfo)))
    {
        LOG_ERROR("Can't listen the to the TCP port: getaddrinfo failed", Properties("error", addrinfoResult).add("port", _port).add("address", _address.toString()), "ServerTcpWindows", "ServerTcpWindows");
        return ;
    }

    // Creates the socket to listen to
    _listenSocket = socket(addrInfo->ai_family, addrInfo->ai_socktype, addrInfo->ai_protocol);
    if (_listenSocket == INVALID_SOCKET)
    {
        LOG_ERROR("Failed to create the listen socket", Properties("error", WSAGetLastError()).add("port", _port).add("address", _address.toString()), "ServerTcpWindows", "ServerTcpWindows");
        freeaddrinfo(addrInfo);
        return ;
    }

    // Turns IPV6_V6ONLY off to allow dual stack socket (IPv4 + IPv6)
    int ipv6only = 0;
    if (setsockopt(_listenSocket, IPPROTO_IPV6, IPV6_V6ONLY, (char *)&ipv6only, sizeof(ipv6only)) == SOCKET_ERROR)
    {
        LOG_ERROR("Can't listen the to the TCP port: setsockopt IPV6_V6ONLY failed", Properties("error", WSAGetLastError()).add("port", _port).add("address", _address.toString()), "ServerTcpWindows", "ServerTcpWindows");
        freeaddrinfo(addrInfo);
        this->close();
        return ;
    }

    // Binds the TCP listening socket to the address and port
    if (bind(_listenSocket, addrInfo->ai_addr, (int)addrInfo->ai_addrlen) == SOCKET_ERROR)
    {
        LOG_ERROR("Can't listen the to the TCP port: bind failed", Properties("error", WSAGetLastError()).add("port", _port).add("address", _address.toString()), "ServerTcpWindows", "ServerTcpWindows");
        freeaddrinfo(addrInfo);
        this->close();
        return ;
    }
    freeaddrinfo(addrInfo);

    // Starts to listen to the port
    if (::listen(_listenSocket, SOMAXCONN) == SOCKET_ERROR)
    {
        LOG_ERROR("Can't listen the to the TCP port: listen failed", Properties("error", WSAGetLastError()).add("port", _port).add("address", _address.toString()), "ServerTcpWindows", "ServerTcpWindows");
        this->close();
        return ;
    }
    _listening = true;

    // Disables the write buffer
    if (_disableWriteBuffer)
    {
        int sndbuff = 0;
        if (setsockopt(_listenSocket, SOL_SOCKET, SO_SNDBUF, (char *)&sndbuff, sizeof(sndbuff)) == SOCKET_ERROR)
            LOG_ERROR("setsockopt SO_SNDBUF failed", Properties("error", WSAGetLastError()).add("port", _port).add("address", _address.toString()), "ServerTcpWindows", "ServerTcpWindows");
    }

    // Non blocking mode
    u_long nonBlockingMode = 1;
    if ((ioctlsocket(_listenSocket, FIONBIO, &nonBlockingMode)) == SOCKET_ERROR)
    {
        LOG_ERROR("Failed to set the socket in non blocking mode", Properties("error", WSAGetLastError()).add("port", _port).add("address", _address.toString()), "ServerTcpWindows", "ServerTcpWindows");
        this->close();
        return ;
    }

    WSAPOLLFD fd;
    fd.fd = _listenSocket;
    fd.events = POLLRDNORM;
    _fds.append(fd);
}

ServerTcpWindows::~ServerTcpWindows()
{
    close();
}

void ServerTcpWindows::execute()
{
    int result;

    while (true)
    {
        if ((result = WSAPoll(_fds.data(), _fds.size(), _wsaPollTimeout)) == SOCKET_ERROR)
        {
            LOG_ERROR("An error occured while executing the TCP server: WSAPoll failed", Properties("error", WSAGetLastError()).add("port", _port).add("address", _address.toString()), "ServerTcpWindows", "execute");
            break;
        }

        // Checks if something appenned on the listen socket
        if (_fds[0].revents)
        {
            // New connection
            if (_fds[0].revents & POLLRDNORM)
                _newConnection();
            // Listen socket disconnected
            if (_fds[0].revents & (POLLERR | POLLHUP | POLLNVAL))
                break;
            --result;
        }

        // Checks the other clients
        if (result > 0)
            for (int i = 1; i < _fds.size(); ++i)
                if (_fds[i].revents)
                {
                    SocketTcpWindows *socketTcp = static_cast<SocketTcpWindows *>(_sockets[i - 1].data());
                    if (_fds[i].revents & POLLRDNORM)
                        socketTcp->readyRead();
                    if (_fds[i].revents & POLLWRNORM)
                        socketTcp->readyWrite();
                    if (_fds[i].revents & (POLLERR | POLLHUP | POLLNVAL))
                        _disconnect(socketTcp, i);
                    if (--result <= 0)
                        break;
                }
    }
    _disconnectAll();
}

void ServerTcpWindows::close()
{
    if (_listenSocket != INVALID_SOCKET)
    {
        closesocket(_listenSocket);
        _listenSocket = INVALID_SOCKET;
    }
    _listening = false;
}

void ServerTcpWindows::_newConnection()
{
    SOCKET socket;
    sockaddr_in6 addr;
    int addrSize = sizeof(addr);

    // Accepts the new client
    if ((socket = accept(_listenSocket, (sockaddr *)&addr, &addrSize)) == INVALID_SOCKET)
    {
        LOG_DEBUG("Error with accept", Properties("error", WSAGetLastError()).add("port", _port).add("address", _address.toString()), "ServerTcpWindows", "_newConnection");
        return ;
    }

    // Too many pending connection, client is dropped
    if (_pendingConnections.size() > _maxPendingConnections)
    {
        closesocket(socket);
        LOG_DEBUG("Too many pending connection, new socket closed", Properties("error", WSAGetLastError()).add("port", _port).add("address", _address.toString()), "ServerTcpWindows", "_newConnection");
        return ;
    }

    // Disables the write buffer
    int sndbuff = 0;
    if (_disableWriteBuffer && setsockopt(socket, SOL_SOCKET, SO_SNDBUF, (char *)&sndbuff, sizeof(sndbuff)) == SOCKET_ERROR)
    {
        LOG_ERROR("setsockopt SO_SNDBUF failed", Properties("error", WSAGetLastError()).add("port", _port).add("address", _address.toString()), "ServerTcpWindows", "_newConnection");
        closesocket(socket);
        return ;
    }

    // Gets its IP and port
    QByteArray ip = "::1";
    ip.reserve(46);
    if (!InetNtopA(AF_INET6, &addr.sin6_addr, ip.data(), ip.size()))
        LOG_ERROR("WSAAddressToString failed", Properties("error", WSAGetLastError()).add("port", _port).add("address", _address.toString()), "ServerTcpWindows", "_newConnection");
    QHostAddress peerAddress = QHostAddress(QString(ip.data()));
    quint16 peerPort = ntohs(addr.sin6_port);

    // Adds the socket to the lists
    WSAPOLLFD fd;
    fd.fd = socket;
    fd.events = POLLRDNORM;
    _fds.append(fd);
    QSharedPointer<Socket> socketTcp(new SocketTcpWindows(peerAddress, peerPort, _port, socket));
    static_cast<SocketTcpWindows *>(socketTcp.data())->setFdEvents(&_fds.last().events);
    _sockets.append(socketTcp);
    _pendingConnections.append(socketTcp);
    emit newConnection();
}

void ServerTcpWindows::_disconnect(SocketTcpWindows *socketTcp, int &i)
{
    if (!_pendingConnections.removeOne(_sockets[i - 1]))
        socketTcp->disconnected();
    socketTcp->setFdEvents(NULL);
    _sockets.removeAt(i - 1);
    _fds.remove(i--);
}

void ServerTcpWindows::_disconnectAll()
{
    close();
    _pendingConnections.clear();
    for (int i = 0; i < _sockets.size(); ++i)
    {
        SocketTcpWindows *socketTcp = static_cast<SocketTcpWindows *>(_sockets[i].data());
        socketTcp->disconnected();
        socketTcp->setFdEvents(NULL);
    }
    _fds.clear();
    _sockets.clear();
    LOG_DEBUG("Listen socket disconnected", Properties("port", _port).add("address", _address.toString()), "ServerTcpWindows", "_disconnectAll");
}

void ServerTcpWindows::_printAddrinfo(addrinfo *addrInfo)
{
    INT iRetval;
    int i = 1;
    addrinfo *ptr = NULL;
    sockaddr_in  *sockaddr_ipv4;
    LPSOCKADDR sockaddr_ip;
    char ipstringbuffer[46];
    DWORD ipbufferlength = 46;

    // Retrieve each address and print out the hex bytes
    for (ptr = addrInfo; ptr != NULL; ptr = ptr->ai_next)
    {
        printf("getaddrinfo response %d\n", i++);
        printf("\tFlags: 0x%x\n", ptr->ai_flags);
        printf("\tFamily: ");
        switch (ptr->ai_family)
        {
            case AF_UNSPEC:
                printf("Unspecified\n");
                break;
            case AF_INET:
                printf("AF_INET (IPv4)\n");
                sockaddr_ipv4 = (struct sockaddr_in *) ptr->ai_addr;
                printf("\tIPv4 address %s\n", inet_ntoa(sockaddr_ipv4->sin_addr));
                break;
            case AF_INET6:
                printf("AF_INET6 (IPv6)\n");
                // the InetNtop function is available on Windows Vista and later
                //sockaddr_in6 *sockaddr_ipv6;
                //sockaddr_ipv6 = (struct sockaddr_in6 *) ptr->ai_addr;
                //printf("\tIPv6 address %s\n", InetNtopA(AF_INET6, &sockaddr_ipv6->sin6_addr, ipstringbuffer, 46));

                // We use WSAAddressToString since it is supported on Windows XP and later
                sockaddr_ip = (LPSOCKADDR) ptr->ai_addr;
                // The buffer length is changed by each call to WSAAddresstoString
                // So we need to set it for each iteration through the loop for safety
                ipbufferlength = 46;
                iRetval = WSAAddressToStringA(sockaddr_ip, (DWORD)ptr->ai_addrlen, NULL, ipstringbuffer, &ipbufferlength);
                if (iRetval)
                    printf("WSAAddressToString failed with %u\n", WSAGetLastError());
                else
                    printf("\tIPv6 address %s\n", ipstringbuffer);
                break;
            case AF_NETBIOS:
                printf("AF_NETBIOS (NetBIOS)\n");
                break;
            default:
                printf("Other %ld\n", ptr->ai_family);
                break;
        }
        printf("\tSocket type: ");
        switch (ptr->ai_socktype)
        {
            case 0:
                printf("Unspecified\n");
                break;
            case SOCK_STREAM:
                printf("SOCK_STREAM (stream)\n");
                break;
            case SOCK_DGRAM:
                printf("SOCK_DGRAM (datagram) \n");
                break;
            case SOCK_RAW:
                printf("SOCK_RAW (raw) \n");
                break;
            case SOCK_RDM:
                printf("SOCK_RDM (reliable message datagram)\n");
                break;
            case SOCK_SEQPACKET:
                printf("SOCK_SEQPACKET (pseudo-stream packet)\n");
                break;
            default:
                printf("Other %ld\n", ptr->ai_socktype);
                break;
        }
        printf("\tProtocol: ");
        switch (ptr->ai_protocol)
        {
            case 0:
                printf("Unspecified\n");
                break;
            case IPPROTO_TCP:
                printf("IPPROTO_TCP (TCP)\n");
                break;
            case IPPROTO_UDP:
                printf("IPPROTO_UDP (UDP) \n");
                break;
            default:
                printf("Other %ld\n", ptr->ai_protocol);
                break;
        }
        printf("\tLength of this sockaddr: %d\n", ptr->ai_addrlen);
        printf("\tCanonical name: %s\n", ptr->ai_canonname);
    }
}

#endif // Q_OS_WIN
