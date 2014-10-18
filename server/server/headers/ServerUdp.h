#ifndef SERVERUDP_H
# define SERVERUDP_H

#include <QObject>
#include <QHostAddress>
#include <QSharedPointer>

#include "SocketUdp.h"

/// @brief A UDP server that can listen on a port and tell when data are
/// available for read, write...
class ServerUdp : public QObject
{
    Q_OBJECT

public:
    /// @brief Creates an instance of ServerUdp for the current platform.
    /// NULL is returned if no implementation is available for the current platform.
    static ServerUdp *create(quint16 port, const QHostAddress &address = QHostAddress::Any);

    virtual ~ServerUdp();

    /// @brief Returns true if the server is currently listening for incoming connections; otherwise returns false.
    inline bool isListening() const { return _listening; }
    /// @brief Blocks until the server is closed.
    virtual void execute() = 0;
    /// @brief Returns the size of the next pending datagram or 0 if there is none.
    virtual qint64 hasPendingDatagrams() const = 0;
    /// @brief Reads the next datagram.
    /// @return The number of bytes read, or -1 if an error occured.
    virtual qint64 readDatagram(char *data, qint64 size, QHostAddress &address, unsigned short &port) = 0;
    /// @brief Returns the UDP listen socket.
    virtual QSharedPointer<Socket> getListenSocket() = 0;
    /// @brief Closes the server. The server will no longer listen for incoming connections.
    virtual void close() = 0;

signals:
    /// @brief This signal is emitted every time a new connection is available.
    void newConnection();

protected:
    ServerUdp(quint16 port, const QHostAddress &address);

    quint16 _port; ///< The port to listen to.
    QHostAddress _address; ///< If address is QHostAddress::Any, the server will listen on all network interfaces.
    bool _listening; ///< True if the server is listening the socket.
};

#endif // SERVERUDP_H
