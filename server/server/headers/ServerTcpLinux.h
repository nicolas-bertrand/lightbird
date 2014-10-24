#ifndef SERVERTCPLINUX_H
# define SERVERTCPLINUX_H

#include <QMutex>

#include "ServerTcp.h"

#ifdef Q_OS_LINUX
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>

/// @brief The Linux implementation of the TCP server.
class ServerTcpLinux : public ServerTcp
{
    Q_OBJECT

public:
    ServerTcpLinux(quint16 port, const QHostAddress &address = QHostAddress::Any);
    ~ServerTcpLinux();

    /// @brief Starts to listen to the network. This method only returns when
    /// the server is closed.
    void execute();
    /// @brief Closes the server, making execute() return.
    void close();

private:
    /// @brief A new client is connected to the server.
    void _newConnection();
    /// @brief A client has been disconnected.
    void _disconnect(epoll_event &event);
    /// @brief Disconnects all the sockets.
    void _disconnectAll();
    /// @brief Closes the sockets in _socketsToClose.
    void _closeSockets();

    int _listenSocket; ///< The socket on which the server is listening.
    bool _disableWriteBuffer; ///< True to disable the write buffer (less CPU overhead, but more protocol overhead).
    int _maxPendingConnections; ///< The maximum number of pending connections. Beyond that, the new sockets are closed just after the accept.
    QList<epoll_event> _sockets; ///< The list of the sockets connected to the server.
    QList<void *> _socketsToClose; ///< The list of the sockets so close. This allows _socketClosed() to be thread safe.
    QMutex _socketsToCloseMutex; ///< Allows _socketsToAdd to be thread safe.
    int _epoll; ///< The epoll file descriptor.

private slots:
    /// @brief Modifies the epoll events of a socket.
    void _setEpollEvents(epoll_event &events);
    /// @brief A socket was closed.
    void _socketClosed(epoll_event &events);
};

#endif // Q_OS_LINUX
#endif // SERVERTCPLINUX_H
