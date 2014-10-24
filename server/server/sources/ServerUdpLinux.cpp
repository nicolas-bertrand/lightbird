#include "Log.h"
#include "ServerUdpLinux.h"
#ifdef Q_OS_LINUX
#include <sys/socket.h>
#include <netdb.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/eventfd.h>

ServerUdpLinux::ServerUdpLinux(quint16 port, const QHostAddress &address)
    : ServerUdp(port, address)
    , _disableWriteBuffer(true)
    , _epoll(-1)
    , _eventfd(-1)
{
    _ip.resize(46);

    // Creates the socket to listen to
    int s;
    if ((s = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP)) == -1)
    {
        LOG_ERROR("Server socket() failed", Properties("error", errno), "ServerUdpLinux", "ServerUdpLinux");
        return ;
    }
    _listenSocket = QSharedPointer<Socket>(new SocketUdpLinux(_port, (qintptr)socket));

    // Turns IPV6_V6ONLY off to allow dual stack socket (IPv4 + IPv6)
    int ipv6only = 0;
    if (setsockopt(s, IPPROTO_IPV6, IPV6_V6ONLY, (char *)&ipv6only, sizeof(ipv6only)) == -1)
    {
        LOG_ERROR("setsockopt IPV6_V6ONLY failed", Properties("error", errno), "ServerUdpLinux", "ServerUdpLinux");
        return ;
    }

    // Resolves the local address and port to be used by the server
    QByteArray portArray(QByteArray::number(_port));
    struct addrinfo *addrInfo = NULL, hints;
    memset(&hints, 0, sizeof (hints));
    hints.ai_family = AF_INET6;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_protocol = IPPROTO_UDP;
    hints.ai_flags = AI_PASSIVE;
    int addrinfoResult;
    if ((addrinfoResult = getaddrinfo(NULL, portArray.data(), &hints, &addrInfo)))
    {
        LOG_ERROR("Server getaddrinfo failed", Properties("error", addrinfoResult), "ServerUdpLinux", "ServerUdpLinux");
        return ;
    }

    // Binds the UDP listening socket to the address and port
    if (bind(s, addrInfo->ai_addr, (int)addrInfo->ai_addrlen) == -1)
    {
        LOG_ERROR("Unable to find a valid port to bind the server interrupt socket", Properties("error", errno), "ServerUdpLinux", "ServerUdpLinux");
        freeaddrinfo(addrInfo);
        return ;
    }
    freeaddrinfo(addrInfo);

    // Disables the write buffer
    int sndbuff = 0;
    if (_disableWriteBuffer && setsockopt(s, SOL_SOCKET, SO_SNDBUF, (char *)&sndbuff, sizeof(sndbuff)) == -1)
    {
        LOG_ERROR("setsockopt SO_SNDBUF failed", Properties("error", errno).add("port", _port).add("address", _address.toString()), "ServerUdpLinux", "ServerUdpLinux");
        return ;
    }

    // Non blocking mode
    u_long nonBlockingMode = 1;
    if ((ioctl(s, FIONBIO, &nonBlockingMode)) == -1)
    {
        LOG_DEBUG("Failed to set the socket in non blocking mode", Properties("error", errno).add("port", _port).add("address", _address.toString()), "ServerUdpLinux", "ServerUdpLinux");
        return ;
    }

    // Creates the epoll
    if ((_epoll = epoll_create1(0)) == -1)
    {
        LOG_ERROR("Failed to create the epoll: epoll_create1 failed", Properties("error", errno).add("port", _port).add("address", _address.toString()), "ServerUdpLinux", "ServerUdpLinux");
        return ;
    }

    // Adds the listen socket to the epoll
    memset(&_epollEvents, 0, sizeof(_epollEvents));
    _epollEvents.events = EPOLLIN | EPOLLRDHUP;
    if (epoll_ctl(_epoll, EPOLL_CTL_ADD, s, &_epollEvents) == -1)
    {
        LOG_ERROR("Failed to add the socket to epoll: epoll_ctl EPOLL_CTL_ADD failed", Properties("error", errno).add("port", _port).add("address", _address.toString()), "ServerUdpLinux", "ServerUdpLinux");
        return ;
    }

    // Creates the eventfd that allows to interrupt epoll_wait
    if ((_eventfd = eventfd(0, 0)) == -1)
    {
        LOG_ERROR("Error with eventfd", Properties("error", errno), "ServerUdpLinux", "ServerUdpLinux");
        return ;
    }
    if ((ioctl(_eventfd, FIONBIO, &nonBlockingMode)) == -1)
    {
        LOG_DEBUG("Failed to set eventFd in non blocking mode", Properties("error", errno), "ServerUdpLinux", "ServerUdpLinux");
        return ;
    }
    epoll_event e;
    memset(&e, 0, sizeof(e));
    e.events = EPOLLIN;
    e.data.ptr = this;
    if (epoll_ctl(_epoll, EPOLL_CTL_ADD, _eventfd, &e) == -1)
    {
        LOG_ERROR("Failed to add the eventFd to epoll: epoll_ctl EPOLL_CTL_ADD failed", Properties("error", errno), "ServerUdpLinux", "ServerUdpLinux");
        return ;
    }

    _listening = true;
}

ServerUdpLinux::~ServerUdpLinux()
{
    close();
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

void ServerUdpLinux::execute()
{
    SocketUdpLinux *socket = static_cast<SocketUdpLinux *>(_listenSocket.data());
    int result;
    epoll_event event;

    while (_listening)
    {
        if ((result = epoll_wait(_epoll, &event, 1, -1)) == -1 && errno != EINTR)
        {
            LOG_ERROR("An error occured while executing the UDP server: epoll_wait failed", Properties("error", errno).add("port", _port).add("address", _address.toString()), "ServerUdpLinux", "execute");
            break;
        }

        if (result > 0)
        {
            // epoll_wait was interrupted by the eventfd
            if (event.data.ptr == this)
            {
                if (event.events & EPOLLIN)
                {
                    quint64 buf;
                    read(_eventfd, &buf, sizeof(buf));
                }
                else
                {
                    LOG_ERROR("Error with the eventfd", Properties("error", event.events), "ServerUdpLinux", "execute");
                    _listening = false;
                }
            }
            // Something appenned on the listen socket
            else
            {
                if (event.events & EPOLLIN)
                    socket->readyRead();
                if (event.events & EPOLLOUT)
                    socket->readyWrite();
                if (event.events & (EPOLLERR | EPOLLHUP | EPOLLRDHUP))
                    break;
            }
        }
    }
    _disconnectAll();
}

qint64 ServerUdpLinux::hasPendingDatagrams() const
{
    qint64 size = _listenSocket->size();
    if (size > 0)
        return size;
    return 0;
}

qint64 ServerUdpLinux::readDatagram(char *data, qint64 size, QHostAddress &address, unsigned short &port)
{
    sockaddr_in6 addr;
    socklen_t addrSize = sizeof(addr);
    int result = recvfrom(_listenSocket->descriptor(), data, size, 0, (sockaddr *)&addr, &addrSize);

    // Gets the address of the peer
    if (result >= 0)
    {
        if (!inet_ntop(AF_INET6, &addr.sin6_addr, _ip.data(), _ip.size()))
            LOG_ERROR("inet_ntop failed", Properties("error", errno).add("port", _port).add("address", _address.toString()), "ServerUdpLinux", "readDatagram");
        address = QHostAddress(QString(_ip));
        port = ntohs(addr.sin6_port);
    }
    return result;
}

QSharedPointer<Socket> ServerUdpLinux::getListenSocket()
{
    return _listenSocket;
}

void ServerUdpLinux::close()
{
    if (_listenSocket)
        _listenSocket->close();
    _listening = false;
    if (_eventfd != -1)
        _interruptEpoll();
}

void ServerUdpLinux::_disconnectAll()
{
    close();
    LOG_DEBUG("Listen socket disconnected", Properties("port", _port).add("address", _address.toString()), "ServerUdpLinux", "_disconnectAll");
}

void ServerUdpLinux::_setEpollEvents(epoll_event &events)
{
    if (epoll_ctl(_epoll, EPOLL_CTL_MOD, static_cast<QSharedPointer<Socket> *>(events.data.ptr)->data()->descriptor(), &events) == -1)
    {
        LOG_ERROR("Failed to modify the socket from epoll: epoll_ctl EPOLL_CTL_MOD failed", Properties("error", errno).add("port", _port).add("address", _address.toString()), "ServerUdpLinux", "setEvents");
        ::close(static_cast<QSharedPointer<Socket> *>(events.data.ptr)->data()->descriptor());
        return ;
    }
}

void ServerUdpLinux::_interruptEpoll()
{
    quint64 buf = 1;
    write(_eventfd, &buf, sizeof(buf));
}

#endif // Q_OS_LINUX
