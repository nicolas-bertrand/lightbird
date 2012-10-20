#ifndef PORTTCP_H
# define PORTTCP_H

# include <QPair>
# include <QSharedPointer>
# include <QTcpServer>

# include "Future.hpp"
# include "Port.h"
# include "WriteBuffer.h"

/// @brief Manages a TCP port of the network. The TCP implementation uses
/// QTcpServer to detect new connection, and QTcpSocket to send or receive data.
class PortTcp : public Port
{
    Q_OBJECT

public:
    PortTcp(unsigned short port, const QStringList &protocols, unsigned int maxClients = ~0);
    ~PortTcp();

    /// @brief Closes the TCP server. No new connections will be accepted.
    void    close();

    // IReadWrite
    void    read(Client *client);
    bool    write(QByteArray *data, Client *client);

signals:
    /// @brief Emitted when the Client is ready to read data. They will be read
    /// in the port thread via the _read method.
    void    readSignal(Client *client);
    /// @brief Emitted when new data have to be write on the network, in order to
    /// write these data from the port thread (where the tcp sockets live).
    void    writeSignal();

private slots:
    /// @brief This slot is called when a new connection is pending on the port of the tcpServer.
    void    _newConnection();
    /// @brief Reads the data on the network from the port thread, then notifies
    /// the Client that they are ready to be processed.
    void    _read(Client *client);
    /// @brief Writes the data stored in writeBuffer on the network, from the port thread.
    void    _write();
    /// @brief Called when a QTcpSocket is disconnected, to remove its client.
    void    _disconnected();
    /// @brief Called when a client is finished.
    Client  *_finished(Client *client = NULL);

private:
    PortTcp(const PortTcp &);
    PortTcp &operator=(const PortTcp &);

    /// @brief The main method of the thread.
    void    run();
    /// @brief Returns true if the port is currently listening the network.
    bool    _isListening() const;

    QTcpServer                          tcpServer;     ///< The TCP server that listens on the network and waits new connections.
    QMap<QAbstractSocket *, Client *>   sockets;       ///< Associates the socket with its client.
    QHash<Client *, QSharedPointer<WriteBuffer> > writeBuffer; ///< The data that are being written from the thread.
    Future<bool>                        threadStarted; ///< This future is unlocked when the thread is started.
};

#endif // PORTTCP_H
