#include "Log.h"
#include "SocketTcpLinux.h"
#ifdef Q_OS_LINUX
#include <sys/socket.h>
#include <netdb.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <unistd.h>

SocketTcpLinux::SocketTcpLinux(const QHostAddress &peerAddress, quint16 peerPort)
    : SocketTcp(peerAddress, peerPort, 0, -1)
    , _events(&_noEvents)
    , _disableWriteBuffer(true)
{
    _socket = -1;
    _connected = false;
    memset(&_noEvents, 0, sizeof (_noEvents));

    // Resolves the peer address and port
    QByteArray address(_peerAddress.toString().toLatin1());
    QByteArray port = QByteArray::number(_peerPort);
    int addrinfoResult;
    struct addrinfo *addrInfo = NULL, hints;
    memset(&hints, 0, sizeof (hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    if ((addrinfoResult = getaddrinfo(address, port.data(), &hints, &addrInfo)))
    {
        LOG_ERROR("Unable to connect to the client: getaddrinfo failed", Properties("error", addrinfoResult).add("peerPort", port).add("peerAddress", address), "SocketTcpLinux", "SocketTcpLinux");
        return ;
    }

    // Creates the socket
    _socket = _descriptor = socket(addrInfo->ai_family, addrInfo->ai_socktype, addrInfo->ai_protocol);
    if (_socket == -1)
    {
        LOG_ERROR("Unable to connect to the client: socket() failed", Properties("error", errno).add("peerPort", port).add("peerAddress", address), "SocketTcpLinux", "SocketTcpLinux");
        freeaddrinfo(addrInfo);
        return ;
    }

    // Disables the write buffer
    int sndbuff = 0;
    if (_disableWriteBuffer && setsockopt(_socket, SOL_SOCKET, SO_SNDBUF, (char *)&sndbuff, sizeof(sndbuff)) == -1)
    {
        LOG_ERROR("Client setsockopt SO_SNDBUF failed", Properties("error", errno), "SocketTcpLinux", "SocketTcpLinux");
        return ;
    }

    // Non blocking mode
    u_long nonBlockingMode = 1;
    if ((ioctl(_socket, FIONBIO, &nonBlockingMode)) == -1)
    {
        LOG_DEBUG("Failed to set the socket in non blocking mode", Properties("error", errno).add("peerPort", port).add("peerAddress", address).add("socket", _socket), "SocketTcpLinux", "SocketTcpLinux");
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
    socklen_t addrlen = sizeof(addr);
    if (getsockname(_socket, (struct sockaddr *)&addr, &addrlen) != -1)
    {
        if (addrlen == sizeof(addr) && addr.sin6_family == AF_INET6)
            _localPort = ntohs(addr.sin6_port);
        else if (addrlen == sizeof(sockaddr_in) && ((sockaddr_in*)&addr)->sin_family == AF_INET)
            _localPort = ntohs(((sockaddr_in*)&addr)->sin_port);
    }
    if (_localPort == 0)
        LOG_ERROR("Unable to get the local port of the socket: getsockname() failed", Properties("error", errno).add("peerPort", port).add("peerAddress", address), "SocketTcpLinux", "SocketTcpLinux");

}

SocketTcpLinux::SocketTcpLinux(const QHostAddress &peerAddress, quint16 peerPort, quint16 localPort, int socket)
    : SocketTcp(peerAddress, peerPort, localPort, socket)
    , _events(&_noEvents)
    , _disableWriteBuffer(true)
{
    // Non blocking mode
    u_long nonBlockingMode = 1;
    if ((ioctl(_socket, FIONBIO, &nonBlockingMode)) == -1)
        LOG_DEBUG("Failed to set the socket in non blocking mode", Properties("error", errno).add("socket", _socket), "SocketTcpLinux", "SocketTcpLinux");
}

SocketTcpLinux::~SocketTcpLinux()
{
    _close();
}

qint64 SocketTcpLinux::size() const
{
    u_long size = 0;
    if ((ioctl(_socket, FIONREAD, &size)) == -1)
    {
        LOG_DEBUG("ioctl error", Properties("error", errno).add("socket", _socket), "SocketTcpLinux", "size");
        return 0;
    }
    return size;
}

qint64 SocketTcpLinux::read(char *data, qint64 size)
{
    return recv(_socket, data, size, 0);
}

qint64 SocketTcpLinux::write(const char *data, qint64 size)
{
    qint64 result = send(_socket, data, size, 0);
    if (result <= 0)
    {
        if (result < 0 && errno == EAGAIN)
            result = 0;
        if (!(_events->events & EPOLLOUT))
        {
            _events->events |= EPOLLOUT;
            emit setEpollEvents(*_events);
        }
    }
    return result;
}

void SocketTcpLinux::close()
{
    if (_connected)
    {
        _close();
        _connected = false;
        emit Socket::disconnected(this);
    }
}

void SocketTcpLinux::connected(bool success)
{
    if (_connecting)
    {
        _connecting = false;
        _connected = success;
        if (success)
        {
            _events->events = EPOLLIN | EPOLLRDHUP;
            emit setEpollEvents(*_events);
        }
        emit Socket::connected(this, success);
    }
}

void SocketTcpLinux::readyRead()
{
    emit Socket::readyRead();
}

void SocketTcpLinux::readyWrite()
{
    if ((_events->events & EPOLLOUT))
    {
        _events->events &= (~EPOLLOUT);
        emit setEpollEvents(*_events);
    }
    emit Socket::readyWrite();
}

void SocketTcpLinux::disconnected()
{
    _connected = false;
    emit Socket::disconnected(this);
}

void SocketTcpLinux::_close()
{
    if (_socket != -1)
    {
        emit closed(*_events);
        ::close(_socket);
        _socket = -1;
    }
}

void SocketTcpLinux::setEpollEvents(epoll_event *events)
{
    if (events)
        _events = events;
    else
        _events = &_noEvents;
}

#endif // Q_OS_LINUX
