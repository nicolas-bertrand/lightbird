#ifndef SOCKET_H
# define SOCKET_H

# include <QObject>

# include "ISocket.h"
# include "INetwork.h"

/// @brief An abstract representation of a socket.
class Socket
        : public QObject
        , public LightBird::ISocket
{
    Q_OBJECT

public:
    Socket(LightBird::INetwork::Transport transport, const QHostAddress &peerAddress, quint16 peerPort, quint16 localPort = 0, qintptr socket = -1);
    virtual ~Socket();

    virtual qint64 size() const = 0;
    virtual qint64 read(char *data, qint64 size) = 0;
    virtual qint64 write(const char *data, qint64 size) = 0;
    virtual void close() = 0;

    inline bool isConnected() const { return _connected; }
    inline const QHostAddress &peerAddress() const { return _peerAddress; }
    inline const QString &peerName() const { return _peerName; }
    inline quint16 peerPort() const { return _peerPort; }
    inline quint16 localPort() const { return _localPort; }
    inline qintptr descriptor() const { return _descriptor; }
    inline LightBird::INetwork::Transport transport() const { return _transport; }

signals:
    /// @brief The socket is connected.
    /// @param success : True if the connection succeeded, false otherwise.
    void connected(Socket *socket, bool success);
    /// @brief New data is available for read.
    void readyRead();
    /// @brief The socket is ready to write data.
    void readyWrite();
    /// @brief This signal is emitted when the socket has been disconnected.
    void disconnected(Socket *socket);

protected:
    qintptr _socket; ///< The socket file descriptor.
    qintptr _descriptor; ///< Holds the same value as socket, but keep it even when the socket is closed.
    bool _connected; ///< True while the socket is connected to a client.
    QHostAddress _peerAddress;
    QString _peerName;
    quint16 _peerPort;
    quint16 _localPort;
    LightBird::INetwork::Transport _transport;
};

#endif // SOCKET_H
