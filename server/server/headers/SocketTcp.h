#ifndef SOCKETTCP_H
# define SOCKETTCP_H

#include "Socket.h"

/// @brief Represents a platform independent TCP socket.
class SocketTcp : public Socket
{
    Q_OBJECT

public:
    /// @brief Creates an instance of SocketTcp for the current platform.
    /// NULL is returned if no implementation is available for the current platform.
    static SocketTcp *create(const QHostAddress &peerAddress, quint16 peerPort);

    SocketTcp(const QHostAddress &peerAddress, quint16 peerPort, quint16 localPort = 0, qintptr socket = -1);
    virtual ~SocketTcp();

    /// @brief True while the socket is connecting.
    inline bool isConnecting() const { return _connecting; }

    // Socket
    virtual qint64 size() const = 0;
    virtual qint64 read(char *data, qint64 size) = 0;
    virtual qint64 write(const char *data, qint64 size) = 0;
    virtual void close() = 0;

protected:
    bool _connecting;
};

#endif // SOCKETTCP_H
