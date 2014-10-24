#ifndef SOCKETUDP_H
# define SOCKETUDP_H

#include "Socket.h"

/// @brief Represents a platform independent UDP socket.
class SocketUdp : public Socket
{
    Q_OBJECT

public:
    /// @brief Creates an instance of SocketUdp for the current platform.
    /// NULL is returned if no implementation is available for the current platform.
    static SocketUdp *create(const QHostAddress &peerAddress, quint16 peerPort);

    SocketUdp(const QHostAddress &peerAddress, quint16 peerPort, quint16 localPort = 0, qintptr socket = -1);
    virtual ~SocketUdp();

    // Socket
    virtual qint64 size() const = 0;
    virtual qint64 read(char *data, qint64 size) = 0;
    virtual qint64 write(const char *data, qint64 size) = 0;
    virtual void close() = 0;
};

#endif // SOCKETUDP_H
