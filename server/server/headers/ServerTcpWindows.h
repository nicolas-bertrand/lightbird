#ifndef SERVERTCPWINDOWS_H
# define SERVERTCPWINDOWS_H

#include "ServerTcp.h"
#include "SocketTcpWindows.h"

#ifdef Q_OS_WIN
#include <winsock2.h>

/// @brief The Windows implementation of the TCP server.
class ServerTcpWindows : public ServerTcp
{
public:
    ServerTcpWindows(quint16 port, const QHostAddress &address = QHostAddress::Any);
    ~ServerTcpWindows();

    /// @brief Starts to listen to the network. This method only returns when
    /// the server is closed.
    void execute();
    /// @brief Closes the server, making execute() return.
    void close();

private:
    /// @brief A new client is connected to the server.
    void _newConnection();
    /// @brief A client has been disconnected.
    void _disconnect(SocketTcpWindows *socketTcp, int &i);
    /// @brief Disconnects all the sockets.
    void _disconnectAll();
    /// @brief Prints the addrinfo structure.
    void _printAddrinfo(addrinfo *addrInfo);

    bool _disableWriteBuffer; ///< True to disable the write buffer (less CPU overhead, but more protocol overhead).
    int _wsaPollTimeout; ///< The timeout of the WSAPoll.
    int _maxPendingConnections; ///< The maximum number of pending connections. Beyond that, the new sockets are closed just after the accept.
    SOCKET _listenSocket; ///< The socket on which the server is listening.
    QVector<WSAPOLLFD> _fds; ///< The FD used by WSAPoll.
    QList<QSharedPointer<Socket> > _sockets; ///< The list of the sockets connected to the server.
    QRegExp _regexIPv4; ///< Parses IPv4 encoded in IPv6.
    QRegExp _regexIPv6; ///< Parses IPv6.
};

#endif // Q_OS_WIN
#endif // SERVERTCPWINDOWS_H
