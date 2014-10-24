#ifndef SERVERUDPLINUX_H
# define SERVERUDPLINUX_H

#include <QMutex>

#include "ServerUdp.h"
#include "SocketUdpLinux.h"

#ifdef Q_OS_LINUX
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>

/// @brief The Linux implementation of the UDP server.
class ServerUdpLinux : public ServerUdp
{
    Q_OBJECT

public:
    ServerUdpLinux(quint16 port, const QHostAddress &address = QHostAddress::Any);
    ~ServerUdpLinux();

    /// @brief Starts to listen to the network. This method only returns when
    /// the server is closed.
    void execute();
    /// @brief Returns the size of the next pending datagram or 0 if there is none.
    qint64 hasPendingDatagrams() const;
    /// @brief Reads the next datagram.
    /// @return The number of bytes read, or -1 if an error occured.
    qint64 readDatagram(char *data, qint64 size, QHostAddress &address, unsigned short &port);
    /// @brief Returns the UDP listen socket.
    QSharedPointer<Socket> getListenSocket();
    /// @brief Closes the server, making execute() return.
    void close();

private:
    /// @brief Disconnects all the sockets.
    void _disconnectAll();

    QSharedPointer<Socket> _listenSocket; ///< The socket on which the server is listening.
    bool _disableWriteBuffer; ///< True to disable the write buffer (less CPU overhead, but more protocol overhead).
    QByteArray _ip; ///< A temporary IP.
    QList<void *> _socketsToClose; ///< The list of the sockets so close. This allows _socketClosed() to be thread safe.
    QMutex _socketsToCloseMutex; ///< Allows _socketsToAdd to be thread safe.
    int _epoll; ///< The epoll file descriptor.
    epoll_event _epollEvents; ///< The events monitored by the epoll.

private slots:
    /// @brief Modifies the epoll events of a socket.
    void _setEpollEvents(epoll_event &events);
};

#endif // Q_OS_LINUX
#endif // SERVERUDPLINUX_H
