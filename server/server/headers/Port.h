#ifndef PORT_H
# define PORT_H

# include <QAbstractSocket>
# include <QReadWriteLock>
# include <QString>
# include <QStringList>
# include <QThread>

# include "INetwork.h"
# include "IReadWrite.h"

# include "Client.h"
# include "Future.hpp"

/// @brief Manages a port of the network. Each port is listening on its own thread.
class Port : public QThread,
             public IReadWrite
{
    Q_OBJECT

public:
    Port(unsigned short port, LightBird::INetwork::Transport transport,
         const QStringList &protocols = QStringList(), unsigned int maxClients = ~0);
    virtual ~Port();

    /// @see LightBird::INetwork::getClient
    /// @param found : True if the client has been found on the port.
    Future<bool>    getClient(const QString &id, LightBird::INetwork::Client &client, bool &found) const;
    /// @see LightBird::INetwork::getClients
    QStringList     getClients() const;
    /// @brief Disconnects a client from this port.
    /// @param id : The id of the client to disconnect.
    /// @return True is the client exists.
    bool            disconnect(const QString &id);
    /// @brief Tries to send a response without waiting for a request.
    /// @see LightBird::INetwork::send
    bool            send(const QString &id, const QString &protocol, const QVariantMap &informations);
    /// @brief Closes the port. Denies all the new connections to the port and
    /// remove all the clients currently connected.
    virtual void    close();
    /// @brief Returns the port number.
    unsigned short  getPort() const;
    /// @brief Returns the transport protocol used by this port.
    LightBird::INetwork::Transport getTransport() const;
    /// @brief Returns the names of the protocols used to communicate with the
    /// clients connected to this port.
    const QStringList &getProtocols() const;
    /// @brief Returns the maximum number of clients connected to this port.
    unsigned int    getMaxClients() const;
    /// @brief Returns true if the port is listening the network.
    bool            isListening() const;
    // IReadWrite
    virtual bool    read(QByteArray &data, Client *client) = 0;
    virtual bool    write(QByteArray *data, Client *client) = 0;

protected:
    /// @brief The main method of the thread.
    void            run() = 0;
    /// @brief Adds a new client.
    /// @param socket : The socket through which the client is connected.
    /// @param peerAddress : The address of the peer.
    /// @param peerPort : The port of the host from which the client is connected.
    /// @return The new client.
    Client          *_addClient(QAbstractSocket *socket, const QHostAddress &peerAddress, unsigned short peerPort);
    /// @brief Removes a client and disconnect it if it is connected.
    /// @param client : The client to disconnect and remove.
    void            _removeClient(Client *client);
    /// @brief Returns true if the port is listening.
    virtual bool    _isListening() const;
    /// @brief Sets if the port is listening the network.
    void            _isListening(bool listening);

    QList<Client *>         clients; ///< Contains the list of all the clients connected to this port.
    mutable QReadWriteLock  mutex;   ///< Makes this class is thread safe.

protected slots:
    /// @brief Called when a client is finished.
    /// @param client : The finished client. If not provided, the first finished
    /// client will be used.
    /// @return True if a client has been finished.
    virtual bool    _finished(Client *client = NULL);

private:
    Port(const Port &);
    Port &operator=(const Port &);

    LightBird::INetwork::Transport  transport;  ///< The transport protocol used by the port.
    QStringList                     protocols;  ///< The names of the protocols used to communicate with the clients connected to this port.
    bool                            listening;  ///< If the port is listening the network.
    unsigned short                  port;       ///< The number of the port.
    unsigned int                    maxClients; ///< The maximum number of clients simultaneously connected.
};

#endif // PORT_H
