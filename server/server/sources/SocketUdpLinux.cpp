#include "Log.h"
#include "SocketUdpLinux.h"
#ifdef Q_OS_LINUX
#include <sys/socket.h>
#include <netdb.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <unistd.h>

SocketUdpLinux::SocketUdpLinux(const QHostAddress &peerAddress, quint16 peerPort)
    : SocketUdp(peerAddress, peerPort, 0, -1)
    , _peerAddress(NULL)
    , _peerAddressRecv(NULL)
    , _disableWriteBuffer(true)
    , _events(&_noEvents)
{
    _socket = -1;
    _connected = false;
    memset(&_noEvents, 0, sizeof(_noEvents));

    // Resolves the address of the peer
    QByteArray addressArray = peerAddress.toString().toLatin1();
    QByteArray portArray(QByteArray::number(peerPort));
    struct addrinfo *addrInfo = NULL, hints;
    memset(&hints, 0, sizeof (hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_protocol = IPPROTO_UDP;
    int addrinfoResult;
    if ((addrinfoResult = getaddrinfo(addressArray.data(), portArray.data(), &hints, &addrInfo)))
    {
        LOG_ERROR("Server getaddrinfo failed", Properties("error", addrinfoResult).add("peerPort", peerPort).add("peerAddress", peerAddress.toString()), "SocketUdpLinux", "SocketUdpLinux");
        return ;
    }
    if (addrInfo->ai_family == AF_INET)
    {
        _peerAddress = (sockaddr *)new sockaddr_in;
        _peerAddressRecv = (sockaddr *)new sockaddr_in;
    }
    else if (addrInfo->ai_family == AF_INET6)
    {
        _peerAddress = (sockaddr *)new sockaddr_in6;
        _peerAddressRecv = (sockaddr *)new sockaddr_in6;
    }
    else
    {
        LOG_ERROR("Unsuported ai_family", Properties("ai_family", addrInfo->ai_family).add("peerPort", peerPort).add("peerAddress", peerAddress.toString()), "SocketUdpLinux", "SocketUdpLinux");
        freeaddrinfo(addrInfo);
        return ;
    }
    _peerAddressSize = addrInfo->ai_addrlen;
    memcpy(_peerAddress, addrInfo->ai_addr, addrInfo->ai_addrlen);

    // Creates the socket to listen to
    if ((_socket = _descriptor = socket(addrInfo->ai_family, addrInfo->ai_socktype, addrInfo->ai_protocol)) == -1)
    {
        LOG_ERROR("Server socket() failed", Properties("error", errno).add("peerPort", peerPort).add("peerAddress", peerAddress.toString()), "SocketUdpLinux", "SocketUdpLinux");
        freeaddrinfo(addrInfo);
        return ;
    }
    freeaddrinfo(addrInfo);

    // Disables the write buffer
    int sndbuff = 0;
    if (_disableWriteBuffer && setsockopt(_socket, SOL_SOCKET, SO_SNDBUF, (char *)&sndbuff, sizeof(sndbuff)) == -1)
    {
        LOG_ERROR("setsockopt SO_SNDBUF failed", Properties("error", errno).add("peerPort", peerPort).add("peerAddress", peerAddress.toString()), "ServerUdpLinux", "ServerUdpLinux");
        return ;
    }

    // Non blocking mode
    u_long nonBlockingMode = 1;
    if ((ioctl(_socket, FIONBIO, &nonBlockingMode)) == -1)
    {
        LOG_DEBUG("Failed to set the socket in non blocking mode", Properties("error", errno).add("peerPort", peerPort).add("peerAddress", peerAddress.toString()).add("socket", _socket), "SocketUdpLinux", "SocketUdpLinux");
        return ;
    }
    _connected = true;
}

SocketUdpLinux::SocketUdpLinux(quint16 localPort, qintptr socket)
    : SocketUdp(QHostAddress(), 0, localPort, socket)
    , _peerAddress(NULL)
    , _peerAddressRecv(NULL)
    , _disableWriteBuffer(true)
    , _events(&_noEvents)
{
}

SocketUdpLinux::~SocketUdpLinux()
{
    _close();
    delete _peerAddress;
    delete _peerAddressRecv;
}

qint64 SocketUdpLinux::size() const
{
    u_long size = 0;
    if ((ioctl(_socket, FIONREAD, &size)) == -1)
    {
        LOG_DEBUG("ioctl error", Properties("error", errno).add("socket", _socket), "SocketUdpLinux", "size");
        return 0;
    }
    return size;
}

qint64 SocketUdpLinux::read(char *data, qint64 size)
{
    socklen_t addressSize = _peerAddressSize;
    qint64 result = recvfrom(_socket, data, size, 0, _peerAddressRecv, &addressSize);

    // We only read the data from the peer we are connected to
    if (result > 0)
    {
        if (addressSize == sizeof(sockaddr_in6))
        {
            if ((((sockaddr_in6 *)_peerAddressRecv)->sin6_port != ((sockaddr_in6 *)_peerAddress)->sin6_port ||
                memcmp(((sockaddr_in6 *)_peerAddressRecv)->sin6_addr.__in6_u.__u6_addr8, ((sockaddr_in6 *)_peerAddress)->sin6_addr.__in6_u.__u6_addr8, sizeof(((sockaddr_in6 *)_peerAddress)->sin6_addr.__in6_u.__u6_addr8))))
                return 0;
        }
        else if (addressSize == sizeof(sockaddr_in))
        {
            if ((((sockaddr_in *)_peerAddressRecv)->sin_port != ((sockaddr_in *)_peerAddress)->sin_port ||
                ((sockaddr_in *)_peerAddressRecv)->sin_addr.s_addr != ((sockaddr_in *)_peerAddress)->sin_addr.s_addr))
                return 0;
        }
    }
    return result;
}

qint64 SocketUdpLinux::write(const char *data, qint64 size)
{
    qint64 result = sendto(_socket, data, size, 0, _peerAddress, _peerAddressSize);
    if (result < 0 && (errno == EAGAIN || errno == EINTR))
        result = 0;
    return result;
}

void SocketUdpLinux::writeAgain()
{
    if (!(_events->events & EPOLLOUT))
    {
        _events->events |= EPOLLOUT;
        emit setEpollEvents(*_events);
    }
}

void SocketUdpLinux::close()
{
    if (_connected)
    {
        _close();
        _connected = false;
        emit Socket::disconnected(this);
    }
}

void SocketUdpLinux::readyRead()
{
    emit Socket::readyRead();
}

void SocketUdpLinux::readyWrite()
{
    if ((_events->events & EPOLLOUT))
    {
        _events->events &= (~EPOLLOUT);
        emit setEpollEvents(*_events);
    }
    emit Socket::readyWrite();
}

void SocketUdpLinux::disconnected()
{
    _connected = false;
    emit Socket::disconnected(this);
}

void SocketUdpLinux::_close()
{
    if (_socket != -1)
    {
        emit closed(*_events);
        ::close(_socket);
        _socket = -1;
    }
}

void SocketUdpLinux::setEpollEvents(epoll_event *events)
{
    if (events)
        _events = events;
    else
        _events = &_noEvents;
}

#endif // Q_OS_LINUX
