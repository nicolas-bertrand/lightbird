#ifndef SERVERTCP_H
# define SERVERTCP_H

#include <QObject>
#include <QHostAddress>
#include <QSharedPointer>

#include "SocketTcp.h"

/// @brief A TCP server that can listen on a port and tell when data are
/// available for read, write...
class ServerTcp : public QObject
{
    Q_OBJECT

public:
    /// @brief Creates an instance of ServerTcp for the current platform.
    /// NULL is returned if no implementation is available for the current platform.
    static ServerTcp *create(quint16 port, const QHostAddress &address = QHostAddress::Any);

    virtual ~ServerTcp();

    /// @brief Returns true if the server is currently listening for incoming connections; otherwise returns false.
    inline bool isListening() const { return _listening; }
    /// @brief Blocks until the server is closed.
    virtual void execute() = 0;
    /// @brief Returns true if the server has a pending connection; Otherwise returns false.
    inline bool hasPendingConnections() const { return !_pendingConnections.isEmpty(); }
    /// @brief Returns the next pending connection as a connected SocketTcp object.
    /// NULL is returned if this function is called when there are no pending connections.
    QSharedPointer<Socket> nextPendingConnection();
    /// @brief Closes the server. The server will no longer listen for incoming connections.
    virtual void close() = 0;

signals:
    /// @brief This signal is emitted every time a new connection is available.
    void newConnection();

protected:
    ServerTcp(quint16 port, const QHostAddress &address);

    quint16 _port; ///< The port to listen to.
    QHostAddress _address; ///< If address is QHostAddress::Any, the server will listen on all network interfaces.
    bool _listening; ///< True if the server is listening the socket.
    QList<QSharedPointer<Socket> > _pendingConnections; ///< The list of the sockets waiting to be get by nextPendingConnection.
};

#endif // SERVERTCP_H
