#include "Log.h"
#include "SocketTcpWindows.h"

#ifdef Q_OS_WIN

SocketTcpWindows::SocketTcpWindows(const QHostAddress &peerAddress, quint16 peerPort)
    : SocketTcp(peerAddress, peerPort, 0, INVALID_SOCKET)
    , _events(&_noEvents)
{
    _socket = INVALID_SOCKET;
    _connected = false;

    // Resolves the peer address and port
    QByteArray address(_peerAddress.toString().toLatin1());
    QByteArray port = QByteArray::number(_peerPort);
    int addrinfoResult;
    struct addrinfo *addrInfo = NULL, hints;
    ZeroMemory(&hints, sizeof (hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    if ((addrinfoResult = getaddrinfo(address, port.data(), &hints, &addrInfo)))
    {
        LOG_ERROR("Unable to connect to the client: getaddrinfo failed", Properties("error", addrinfoResult).add("peerPort", port).add("peerAddress", address), "SocketTcpWindows", "SocketTcpWindows");
        return ;
    }

    // Creates the socket
    _socket = _descriptor = socket(addrInfo->ai_family, addrInfo->ai_socktype, addrInfo->ai_protocol);
    if (_socket == INVALID_SOCKET)
    {
        LOG_ERROR("Unable to connect to the client: socket() failed", Properties("error", WSAGetLastError()).add("peerPort", port).add("peerAddress", address), "SocketTcpWindows", "SocketTcpWindows");
        freeaddrinfo(addrInfo);
        return ;
    }

    // Non blocking mode
    u_long nonBlockingMode = 1;
    if ((ioctlsocket(_socket, FIONBIO, &nonBlockingMode)) == SOCKET_ERROR)
    {
        LOG_DEBUG("Failed to set the socket in non blocking mode", Properties("error", WSAGetLastError()).add("peerPort", port).add("peerAddress", address).add("socket", _socket), "SocketTcpWindows", "SocketTcpWindows");
        freeaddrinfo(addrInfo);
        _close();
        return ;
    }

    // Starts the connection to the server
    ::connect(_socket, addrInfo->ai_addr, (int)addrInfo->ai_addrlen);
    _connecting = true;
    freeaddrinfo(addrInfo);

    // Gets the local port of the socket
    sockaddr_in6 addr;
    int addrlen = sizeof(addr);
    if (getsockname(_socket, (struct sockaddr *)&addr, &addrlen) != SOCKET_ERROR)
    {
        if (addrlen == sizeof(addr) && addr.sin6_family == AF_INET6)
            _localPort = ntohs(addr.sin6_port);
        else if (addrlen == sizeof(sockaddr_in) && ((sockaddr_in*)&addr)->sin_family == AF_INET)
            _localPort = ntohs(((sockaddr_in*)&addr)->sin_port);
    }
    if (_localPort == 0)
        LOG_ERROR("Unable to get the local port of the socket: getsockname() failed", Properties("error", WSAGetLastError()).add("peerPort", port).add("peerAddress", address), "SocketTcpWindows", "SocketTcpWindows");

}

SocketTcpWindows::SocketTcpWindows(const QHostAddress &peerAddress, quint16 peerPort, quint16 localPort, SOCKET socket)
    : SocketTcp(peerAddress, peerPort, localPort, socket)
    , _events(&_noEvents)
{
    // Non blocking mode
    u_long nonBlockingMode = 1;
    if ((ioctlsocket(_socket, FIONBIO, &nonBlockingMode)) == SOCKET_ERROR)
        LOG_DEBUG("Failed to set the socket in non blocking mode", Properties("error", WSAGetLastError()).add("socket", _socket), "SocketTcpWindows", "SocketTcpWindows");
}

SocketTcpWindows::~SocketTcpWindows()
{
    _close();
}

qint64 SocketTcpWindows::size() const
{
    u_long size = 0;
    if ((ioctlsocket(_socket, FIONREAD, &size)) == SOCKET_ERROR)
    {
        LOG_DEBUG("ioctlsocket error", Properties("error", WSAGetLastError()).add("socket", _socket), "SocketTcpWindows", "size");
        return 0;
    }
    return size;
}

qint64 SocketTcpWindows::read(char *data, qint64 size)
{
    return recv(_socket, data, size, 0);
}

qint64 SocketTcpWindows::write(const char *data, qint64 size)
{
    qint64 result = send(_socket, data, size, 0);
    if (result == 0)
        *_events |= POLLWRNORM;
    return result;
}

void SocketTcpWindows::close()
{
    if (_connected)
    {
        _close();
        _connected = false;
        emit Socket::disconnected(this);
    }
}

void SocketTcpWindows::connected(bool success)
{
    if (_connecting)
    {
        _connecting = false;
        _connected = success;
        *_events = POLLRDNORM;
        emit Socket::connected(this, success);
    }
}

void SocketTcpWindows::readyRead()
{
    emit Socket::readyRead();
}

void SocketTcpWindows::readyWrite()
{
    *_events &= (~POLLWRNORM);
    emit Socket::readyWrite();
}

void SocketTcpWindows::disconnected()
{
    _connected = false;
    emit Socket::disconnected(this);
}

void SocketTcpWindows::_close()
{
    if (_socket != INVALID_SOCKET)
    {
        closesocket(_socket);
        _socket = INVALID_SOCKET;
    }
}

void SocketTcpWindows::setFdEvents(short *events)
{
    if (events)
        _events = events;
    else
        _events = &_noEvents;
}

#endif // Q_OS_WIN
