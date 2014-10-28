#ifndef SOCKETUDPLINUX_H
# define SOCKETUDPLINUX_H

#include "SocketUdp.h"

#ifdef Q_OS_LINUX
#include <netinet/in.h>
#include <sys/epoll.h>

/// @brief Linux implementation of SocketUdp.
class SocketUdpLinux : public SocketUdp
{
    Q_OBJECT

public:
    /// @brief Creates a new socket (client mode).
    SocketUdpLinux(const QHostAddress &peerAddress, quint16 peerPort);
    /// @brief Builds the UDP socket around an existing socket (server mode).
    SocketUdpLinux(quint16 localPort, qintptr socket);
    ~SocketUdpLinux();

    // SocketUdp
    qint64 size() const;
    qint64 read(char *data, qint64 size);
    qint64 write(const char *data, qint64 size);
    void writeAgain();
    void close();

    // Emits signals
    void readyRead();
    void readyWrite();
    void disconnected();

    /// @brief Sets the events monitored by the epoll.
    void setEpollEvents(epoll_event *events = NULL);

signals:
    /// @brief The sockets wants to modify the events monitored by the epoll.
    void setEpollEvents(epoll_event &events);
    /// @brief The socket was closed.
    void closed(epoll_event &events);

private:
    /// @brief Closes the socket.
    inline void _close();

    sockaddr *_peerAddress; ///< The address of the peer the socket is connected to. The actual format can be sockaddr_in or sockaddr_in6.
    sockaddr *_peerAddressRecv; ///< Stores the address of the peer of the last datagram received. The actual format can be sockaddr_in or sockaddr_in6.
    int _peerAddressSize; ///< The size of the peer address (depends on IPv4 or IPv6).
    bool _disableWriteBuffer; ///< True to disable the write buffer (less CPU overhead, but more protocol overhead).
    epoll_event *_events; ///< The epoll events of this socket. Must never point to an invalid address.
    epoll_event _noEvents; ///< The address pointed by _events when it is not set.
};

#endif // Q_OS_LINUX
#endif // SOCKETUDPLINUX_H
