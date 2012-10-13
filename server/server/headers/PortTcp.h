#ifndef PORTTCP_H
# define PORTTCP_H

# include <QPair>
# include <QTcpServer>

# include "Future.hpp"
# include "Port.h"

/// @brief Manages a TCP port of the network. The TCP implementation uses
/// QTcpServer to detect new connection, and QTcpSocket to send or receive data.
class PortTcp : public Port
{
    Q_OBJECT

public:
    PortTcp(unsigned short port, const QStringList &protocols, unsigned int maxClients = ~0);
    ~PortTcp();

    void    read(Client *client);
    bool    write(QByteArray *data, Client *client);
    /// @brief Closes the TCP server. No new connections will be accepted.
    void    close();

signals:
    /// @brief Emited when the Client is ready to read data. They will be read
    /// in the port thread via the _read method.
    void    readSignal(Client *client);
    /// @brief Emited when new data have to be write on the network, in order to
    /// write these data from the port thread (where the tcp sockets live).
    void    writeSignal();

private:
    PortTcp(const PortTcp &);
    PortTcp &operator=(const PortTcp &);

    /// @brief The main method of the thread.
    void    run();
    /// @brief Returns true if the port is currently listening the network.
    bool    _isListening() const;
    /// @brief Returns true if the writeBuffer contains the client.
    bool    _containsClient(Client *client);

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
    /// @brief Stores the data to write to a client.
    struct WriteBuffer
    {
        WriteBuffer(Client *c, QByteArray *d) : client(c), data(d), offset(0) {}
        Client     *client; ///< The client to which the data will be written.
        QByteArray *data;   ///< The data to write.
        int        offset;  ///< The amount of data already written.
    };

    QTcpServer                        tcpServer;     ///< The TCP server that listens on the network and waits new connections.
    QMap<QAbstractSocket *, Client *> sockets;       ///< Associates the socket with its client.
    QList<WriteBuffer>                writeBuffer;   ///< The list of the data that are going to be send from the thread.
    Future<bool>                      threadStarted; ///< This future is unlocked when the thread is started.
};

#endif // PORTTCP_H
