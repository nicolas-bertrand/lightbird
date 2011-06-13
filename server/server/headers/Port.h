#ifndef PORT_H
# define PORT_H

# include <QAbstractSocket>
# include <QObject>
# include <QString>
# include <QStringList>

# include "INetwork.h"
# include "IReadWrite.h"

# include "Client.h"
# include "Future.hpp"

/// @brief Manage a port of the network.
///
/// This class listen a server port (TCP or UDP), welcome the new connections,
/// read, and write the data on the network. When a client connect to this
/// port, a new thread is created (by the Client object). When the port receives data,
/// they are transmited to the client thread, which will process the request, and
/// genrerates its response. The request is then sent by Port to the client.
/// Actually, the class Port is abstract, and is independent of TCP/UDP considerations.
/// All the transport specific operations are done in the classes that inherits Port,
/// e.g PortTcp and PortUdp.
class Port : public QObject,
             public IReadWrite
{
    Q_OBJECT

public:
    virtual ~Port();

    /// @brief Returns the number of the port.
    unsigned short  getPort();
    /// @brief The transport protocol used by this port.
    LightBird::INetwork::Transports getTransport();
    /// @brief Returns the names of the protocols used to communicate with the
    /// clients connected to this port.
    const QStringList &getProtocols();
    /// @brief Returns the maximum number of connected clients at the same time.
    unsigned int    getMaxClients();
    /// @brief If the port is listening the network.
    virtual bool    isListening();
    /// @brief No new connections will be accepted after this call, and all the
    /// connected clients are removed.
    virtual void    stopListening();
    /// @brief Allows to get the informations of a client.
    /// @param id : The id of the client to find.
    /// @param client : The information of the client to fill.
    /// @param thread : The address of the thread that requires the informations.
    /// Used to avoid dead locks.
    /// @param future : Used to unlock the thread that is waiting the informations.
    /// @return True if the searched client is in the current port.
    bool            getClient(const QString &id, LightBird::INetwork::Client *client, void *thread, Future<bool> *future);
    /// @brief Returns the list of the id of the clients connected to the current port.
    QStringList     getClients();
    /// @brief Disconnect a client from this port.
    /// @param id : The id of the client to disconnect.
    /// @return If the client is connected to this port.
    bool            disconnect(const QString &id);

protected:
    Port(unsigned short port, LightBird::INetwork::Transports transport, const QStringList &protocols = QStringList(),
         unsigned int maxClients = ~0, QObject *object = 0);

    /// @brief Set if the port is listening the network.
    void            _isListening(bool listening);
    /// @brief Add a new connected client. Creates a new Client, and insert
    /// it in the clients list.
    /// @param socket : The socket through which the client is connected.
    /// This can be a QTcpSocket or a QUdpSocket.
    /// @param peerAddress : The address of the peer, e.g the connected client
    /// to this port.
    /// @param peerPort : The port of the host from which the client is connected
    /// (this is different from the local port).
    /// @return The new client.
    Client          *_addClient(QAbstractSocket *socket, const QHostAddress &peerAddress, unsigned short peerPort);
    /// @brief Remove a client and disconnect it if it is connected.
    /// @param client : The client to disconnect and remove.
    void            _removeClient(Client *client);


    virtual bool    read(QByteArray &data, Client *client) = 0;
    virtual bool    write(QByteArray &data, Client *client) = 0;

    QList<Client *> clients;    ///< Contains the list of all the connected clients on the current port.

protected slots:
    /// @brief Removes a client from the client list when its thread is finished.
    /// @return : The client's thread.
    virtual Client  *_finished();

private:
    Port(const Port &);
    Port    *operator=(const Port &);

    LightBird::INetwork::Transports transport;  ///< The transport protocol used by the port.
    QStringList                     protocols;  ///< The names of the protocols used to communicate with the clients connected to this port.
    bool                            listening;  ///< If the port is listening the network.
    unsigned short                  port;       ///< The number of the port.
    unsigned int                    maxClients; ///< The maximum number of clients simultaneously connected.

signals:
    /// @brief This signal is emited when all the clients of the port are destroyed,
    /// and the port is no longer listening on the network.
    /// @param port : The port concerned.
    void    allClientsRemoved(unsigned short port);
};

#endif // PORT_H
