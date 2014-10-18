#ifndef SERVERUDPWINDOWS_H
# define SERVERUDPWINDOWS_H

#include "ServerUdp.h"
#include "SocketUdpWindows.h"

#ifdef Q_OS_WIN
#include <winsock2.h>

/// @brief The Windows implementation of the UDP server.
class ServerUdpWindows : public ServerUdp
{
public:
    ServerUdpWindows(quint16 port, const QHostAddress &address = QHostAddress::Any);
    ~ServerUdpWindows();

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

    int _wsaPollTimeout; ///< The timeout of the WSAPoll.
    QSharedPointer<Socket> _listenSocket; ///< The socket on which the server is listening.
    QVector<WSAPOLLFD> _fds; ///< The FD used by WSAPoll.
    bool _disableWriteBuffer; ///< True to disable the write buffer (less CPU overhead, but more protocol overhead).
    QByteArray _ip; ///< A temporary IP.
};

#endif // Q_OS_WIN
#endif // SERVERUDPWINDOWS_H
