#include "Log.h"
#include "SocketUdpWindows.h"

#ifdef Q_OS_WIN

SocketUdpWindows::SocketUdpWindows(const QHostAddress &peerAddress, quint16 peerPort)
    : SocketUdp(peerAddress, peerPort, 0, INVALID_SOCKET)
    , _events(&_noEvents)
    , _peerAddress(NULL)
    , _peerAddressRecv(NULL)
    , _disableWriteBuffer(true)
{
    _socket = INVALID_SOCKET;
    _connected = false;

    // Resolves the address of the peer
    QByteArray addressArray = peerAddress.toString().toLatin1();
    QByteArray portArray(QByteArray::number(peerPort));
    struct addrinfo *addrInfo = NULL, hints;
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_protocol = IPPROTO_UDP;
    int addrinfoResult;
    if ((addrinfoResult = getaddrinfo(addressArray.data(), portArray.data(), &hints, &addrInfo)))
    {
        LOG_ERROR("Server getaddrinfo failed", Properties("error", addrinfoResult).add("peerPort", peerPort).add("peerAddress", peerAddress.toString()), "SocketUdpWindows", "SocketUdpWindows");
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
        LOG_ERROR("Unsuported ai_family", Properties("ai_family", addrInfo->ai_family).add("peerPort", peerPort).add("peerAddress", peerAddress.toString()), "SocketUdpWindows", "SocketUdpWindows");
        freeaddrinfo(addrInfo);
        return ;
    }
    _peerAddressSize = addrInfo->ai_addrlen;
    memcpy(_peerAddress, addrInfo->ai_addr, addrInfo->ai_addrlen);

    // Creates the socket to listen to
    if ((_socket = _descriptor = socket(addrInfo->ai_family, addrInfo->ai_socktype, addrInfo->ai_protocol)) == INVALID_SOCKET)
    {
        LOG_ERROR("Server socket() failed", Properties("error", WSAGetLastError()).add("peerPort", peerPort).add("peerAddress", peerAddress.toString()), "SocketUdpWindows", "SocketUdpWindows");
        freeaddrinfo(addrInfo);
        return ;
    }
    freeaddrinfo(addrInfo);

    // Disables the write buffer
    int sndbuff = 0;
    if (_disableWriteBuffer && setsockopt(_socket, SOL_SOCKET, SO_SNDBUF, (char *)&sndbuff, sizeof(sndbuff)) == SOCKET_ERROR)
    {
        LOG_ERROR("setsockopt SO_SNDBUF failed", Properties("error", WSAGetLastError()).add("peerPort", peerPort).add("peerAddress", peerAddress.toString()), "ServerUdpWindows", "ServerUdpWindows");
        return ;
    }

    // Non blocking mode
    u_long nonBlockingMode = 1;
    if ((ioctlsocket(_socket, FIONBIO, &nonBlockingMode)) == SOCKET_ERROR)
    {
        LOG_DEBUG("Failed to set the socket in non blocking mode", Properties("error", WSAGetLastError()).add("peerPort", peerPort).add("peerAddress", peerAddress.toString()).add("socket", _socket), "SocketUdpWindows", "SocketUdpWindows");
        return ;
    }
    _connected = true;
}

SocketUdpWindows::SocketUdpWindows(quint16 localPort, qintptr socket)
    : SocketUdp(QHostAddress(), 0, localPort, socket)
    , _events(&_noEvents)
    , _peerAddress(NULL)
    , _peerAddressRecv(NULL)
    , _disableWriteBuffer(true)
{
}

SocketUdpWindows::~SocketUdpWindows()
{
    _close();
    delete _peerAddress;
    delete _peerAddressRecv;
}

qint64 SocketUdpWindows::size() const
{
    u_long size = 0;
    if ((ioctlsocket(_socket, FIONREAD, &size)) == SOCKET_ERROR)
    {
        LOG_DEBUG("ioctlsocket error", Properties("error", WSAGetLastError()).add("socket", _socket), "SocketUdpWindows", "size");
        return 0;
    }
    return size;
}

qint64 SocketUdpWindows::read(char *data, qint64 size)
{
    int addressSize = _peerAddressSize;
    qint64 result = recvfrom(_socket, data, size, 0, _peerAddressRecv, &addressSize);

    // We only read the data from the peer we are connected to
    if (result > 0)
    {
        if (addressSize == sizeof(sockaddr_in6))
        {
            if ((((sockaddr_in6 *)_peerAddressRecv)->sin6_port != ((sockaddr_in6 *)_peerAddress)->sin6_port ||
                memcmp(((sockaddr_in6 *)_peerAddressRecv)->sin6_addr.u.Byte, ((sockaddr_in6 *)_peerAddress)->sin6_addr.u.Byte, sizeof(((sockaddr_in6 *)_peerAddress)->sin6_addr.u.Byte))))
                return 0;
        }
        else if (addressSize == sizeof(sockaddr_in))
        {
            if ((((sockaddr_in *)_peerAddressRecv)->sin_port != ((sockaddr_in *)_peerAddress)->sin_port ||
                ((sockaddr_in *)_peerAddressRecv)->sin_addr.S_un.S_addr != ((sockaddr_in *)_peerAddress)->sin_addr.S_un.S_addr))
                return 0;
        }
    }
    return result;
}

qint64 SocketUdpWindows::write(const char *data, qint64 size)
{
    qint64 result = sendto(_socket, data, size, 0, _peerAddress, _peerAddressSize);
    if (result == 0)
        *_events |= POLLWRNORM;
    return result;
}

void SocketUdpWindows::close()
{
    if (_connected)
    {
        _close();
        _connected = false;
        emit Socket::disconnected(this);
    }
}

void SocketUdpWindows::readyRead()
{
    emit Socket::readyRead();
}

void SocketUdpWindows::readyWrite()
{
    *_events &= (~POLLWRNORM);
    emit Socket::readyWrite();
}

void SocketUdpWindows::disconnected()
{
    _connected = false;
    emit Socket::disconnected(this);
}

void SocketUdpWindows::_close()
{
    if (_socket != INVALID_SOCKET)
    {
        closesocket(_socket);
        _socket = INVALID_SOCKET;
    }
}

void SocketUdpWindows::setFdEvents(short *events)
{
    if (events)
        _events = events;
    else
        _events = &_noEvents;
}

#endif // Q_OS_WIN
