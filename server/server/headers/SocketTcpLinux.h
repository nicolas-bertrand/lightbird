#ifndef SOCKETTCPLINUX_H
# define SOCKETTCPLINUX_H

#include "SocketTcp.h"

#ifdef Q_OS_LINUX
#include <netinet/in.h>
#include <sys/epoll.h>

/// @brief Linux implementation of SocketTcp.
class SocketTcpLinux : public SocketTcp
{
    Q_OBJECT

public:
    /// @brief Creates a new socket (client mode).
    SocketTcpLinux(const QHostAddress &peerAddress, quint16 peerPort);
    /// @brief Builds the TCP socket around an existing socket (server mode).
    SocketTcpLinux(const QHostAddress &peerAddress, quint16 peerPort, quint16 localPort, int socket);
    ~SocketTcpLinux();

    // SocketTcp
    qint64 size() const;
    qint64 read(char *data, qint64 size);
    qint64 write(const char *data, qint64 size);
    void writeAgain();
    void close();

    // Emits signals
    void connected(bool success);
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

    sockaddr_in6 _sockaddr;
    epoll_event *_events; ///< The epoll events of this socket.
    epoll_event _noEvents; ///< Makes _events always point to a value location.
    bool _disableWriteBuffer; ///< True to disable the write buffer (less CPU overhead, but more protocol overhead).
};

#endif // Q_OS_LINUX
#endif // SOCKETTCPLINUX_H
