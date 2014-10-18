#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include "Log.h"
#include "ServerUdpWindows.h"
#ifdef Q_OS_WIN
#include <Ws2tcpip.h>

ServerUdpWindows::ServerUdpWindows(quint16 port, const QHostAddress &address)
    : ServerUdp(port, address)
    , _disableWriteBuffer(true)
    , _wsaPollTimeout(1000)
{
    _ip.resize(46);

    // Creates the socket to listen to
    SOCKET s;
    if ((s = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP)) == INVALID_SOCKET)
    {
        LOG_ERROR("Server socket() failed", Properties("error", WSAGetLastError()), "ServerUdpWindows", "ServerUdpWindows");
        return ;
    }

    _listenSocket = QSharedPointer<Socket>(new SocketUdpWindows(_port, (qintptr)socket));
    //_listenSocket = QSharedPointer<Socket>(new SocketUdpWindows(_port, socket));

    // Turns IPV6_V6ONLY off to allow dual stack socket (IPv4 + IPv6)
    int ipv6only = 0;
    if (setsockopt(s, IPPROTO_IPV6, IPV6_V6ONLY, (char *)&ipv6only, sizeof(ipv6only)) == SOCKET_ERROR)
    {
        LOG_ERROR("setsockopt IPV6_V6ONLY failed", Properties("error", WSAGetLastError()), "ServerUdpWindows", "ServerUdpWindows");
        return ;
    }

    // Resolves the local address and port to be used by the server
    QByteArray portArray(QByteArray::number(_port));
    struct addrinfo *addrInfo = NULL, hints;
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET6;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_protocol = IPPROTO_UDP;
    hints.ai_flags = AI_PASSIVE;
    int addrinfoResult;
    if ((addrinfoResult = getaddrinfo(NULL, portArray.data(), &hints, &addrInfo)))
    {
        LOG_ERROR("Server getaddrinfo failed", Properties("error", addrinfoResult), "ServerUdpWindows", "ServerUdpWindows");
        return ;
    }

    // Binds the UDP listening socket to the address and port
    if (bind(s, addrInfo->ai_addr, (int)addrInfo->ai_addrlen) == SOCKET_ERROR)
    {
        LOG_ERROR("Unable to find a valid port to bind the server interrupt socket", Properties("error", WSAGetLastError()), "ServerUdpWindows", "ServerUdpWindows");
        freeaddrinfo(addrInfo);
        return ;
    }
    freeaddrinfo(addrInfo);

    // Disables the write buffer
    int sndbuff = 0;
    if (_disableWriteBuffer && setsockopt(s, SOL_SOCKET, SO_SNDBUF, (char *)&sndbuff, sizeof(sndbuff)) == SOCKET_ERROR)
    {
        LOG_ERROR("setsockopt SO_SNDBUF failed", Properties("error", WSAGetLastError()).add("port", _port).add("address", _address.toString()), "ServerUdpWindows", "ServerUdpWindows");
        return ;
    }

    // Non blocking mode
    u_long nonBlockingMode = 1;
    if ((ioctlsocket(s, FIONBIO, &nonBlockingMode)) == SOCKET_ERROR)
    {
        LOG_DEBUG("Failed to set the socket in non blocking mode", Properties("error", WSAGetLastError()).add("port", _port).add("address", _address.toString()), "ServerUdpWindows", "ServerUdpWindows");
        return ;
    }

    WSAPOLLFD fd;
    fd.fd = s;
    fd.events = POLLRDNORM;
    _fds.append(fd);
}

ServerUdpWindows::~ServerUdpWindows()
{
    close();
}

void ServerUdpWindows::execute()
{
    SocketUdpWindows *socket = static_cast<SocketUdpWindows *>(_listenSocket.data());

    while (_listening)
    {
        if (WSAPoll(_fds.data(), _fds.size(), _wsaPollTimeout) == SOCKET_ERROR)
        {
            LOG_ERROR("An error occured while executing the UDP server: WSAPoll failed", Properties("error", WSAGetLastError()).add("port", _port).add("address", _address.toString()), "ServerUdpWindows", "execute");
            break;
        }

        // Checks if something appenned on the listen socket
        if (_fds[0].revents)
        {
            if (_fds[0].revents & POLLRDNORM)
                socket->readyRead();
            if (_fds[0].revents & POLLWRNORM)
                socket->readyWrite();
            if (_fds[0].revents & (POLLERR | POLLHUP | POLLNVAL))
                break;
        }
    }
    _disconnectAll();
}

qint64 ServerUdpWindows::hasPendingDatagrams() const
{
    qint64 size = _listenSocket->size();
    if (size > 0)
        return size;
    return 0;
}

qint64 ServerUdpWindows::readDatagram(char *data, qint64 size, QHostAddress &address, unsigned short &port)
{
    sockaddr_in6 addr;
    int addrSize = sizeof(addr);
    int result = recvfrom(_listenSocket->descriptor(), data, size, 0, (sockaddr *)&addr, &addrSize);

    // Gets the address of the peer
    if (result >= 0)
    {
        if (!InetNtopA(AF_INET6, &addr.sin6_addr, _ip.data(), _ip.size()))
            LOG_ERROR("WSAAddressToString failed", Properties("error", WSAGetLastError()).add("port", _port).add("address", _address.toString()), "ServerUdpWindows", "readDatagram");
        address = QHostAddress(QString(_ip));
        port = ntohs(addr.sin6_port);
    }
    return result;
}

QSharedPointer<Socket> ServerUdpWindows::getListenSocket()
{
    return _listenSocket;
}

void ServerUdpWindows::close()
{
    _listenSocket->close();
    _listening = false;
}

void ServerUdpWindows::_disconnectAll()
{
    close();
    _fds.clear();
    LOG_DEBUG("Listen socket disconnected", Properties("port", _port).add("address", _address.toString()), "ServerUdpWindows", "_disconnectAll");
}

#endif // Q_OS_WIN
