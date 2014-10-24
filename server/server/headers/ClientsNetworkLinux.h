#ifndef CLIENTSNETWORKLINUX_H
# define CLIENTSNETWORKLINUX_H

#include <QMutex>

#include "ClientsNetwork.h"
#include "SocketTcpLinux.h"
#include "SocketUdpLinux.h"

#ifdef Q_OS_LINUX
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>

/// @brief Linux implementation of ClientsNetwork.
/// This class is thread safe as long as execute() is called once.
class ClientsNetworkLinux : public ClientsNetwork
{
    Q_OBJECT

public:
    ClientsNetworkLinux();
    ~ClientsNetworkLinux();

    /// @brief This method only returns when the Clients is closed.
    void execute();
    /// @brief Adds a socket to manage.
    void addSocket(QSharedPointer<Socket> socket, int wait = -1);
    /// @brief Closes all the clients, making execute() return.
    void close();

private:
    /// @brief Adds a socket to manage.
    void _addSockets();
    /// @brief The connection to the socket is done (success or not).
    void _connected(SocketTcpLinux *socketTcp, epoll_event &e);
    /// @brief The connection to the socket failed.
    void _connectionFailed(void *socket);
    /// @brief A client has been disconnected.
    void _disconnect(SocketTcpLinux *socketTcp, epoll_event &e);
    void _disconnect(SocketUdpLinux *socketUdp, epoll_event &e);
    /// @brief Disconnects all the sockets.
    void _disconnectAll();
    /// @brief Closes the sockets in _socketsToClose.
    void _closeSockets();
    /// @brief Interrupts epoll_wait by sending a signal to its thread.
    void _interruptEpoll();
    /// @brief Closes the files descriptors.
    void _close();

    bool _disableWriteBuffer; ///< True to disable the write buffer (less CPU overhead, but more protocol overhead).
    QList<epoll_event> _sockets; ///< The list of the sockets connected to the clients.
    QMap<qint64, void *> _connections; ///< The list of the connecting sockets, sorted by their timeout.
    QList<QPair<QSharedPointer<Socket>, int> > _socketsToAdd; ///< The list of the sockets so add. This allows addSocket() to be thread safe.
    QMutex _socketsToAddMutex; ///< Allows _socketsToAdd to be thread safe.
    QList<void *> _socketsToClose; ///< The list of the sockets so close. This allows _socketClosed() to be thread safe.
    QMutex _socketsToCloseMutex; ///< Allows _socketsToAdd to be thread safe.
    int _epoll; ///< The epoll file descriptor.
    int _eventfd; ///< Allows to interrupt epoll_wait.

private slots:
    /// @brief Modifies the epoll events of a socket.
    void _setEpollEvents(epoll_event &events);
    /// @brief A socket was closed.
    void _socketClosed(epoll_event &events);
};

#endif // Q_OS_LINUX
#endif // CLIENTSNETWORKLINUX_H
