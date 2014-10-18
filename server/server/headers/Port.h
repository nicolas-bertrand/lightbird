#ifndef PORT_H
# define PORT_H

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
    Port(unsigned short _port, LightBird::INetwork::Transport transport,
         const QStringList &protocols = QStringList(), unsigned int maxClients = ~0);
    virtual ~Port();

    /// @see LightBird::INetwork::getClient
    /// @param found : True if the client has been found on the port.
    Future<bool>    getClient(const QString &id, LightBird::INetwork::Client &client, bool &found) const;
    /// @see LightBird::INetwork::getClients
    QStringList     getClients() const;
    /// @brief Disconnects a client from this port.
    /// @see LightBird::INetwork::disconnect
    bool            disconnect(const QString &id, bool fatal = false);
    /// @brief Tries to send a response without waiting for a request.
    /// @see LightBird::INetwork::send
    bool            send(const QString &id, const QString &protocol, const QVariantMap &informations);
    /// @see LightBird::INetwork::pause
    bool            pause(const QString &idClient, int msec = -1);
    /// @see LightBird::INetwork::resume
    bool            resume(const QString &idClient);
    /// @brief Returns true if the port is listening the network.
    virtual bool    isListening() const = 0;
    /// @brief Closes the port. Denies all the new connections to the port and
    /// remove all the clients currently connected.
    virtual void    close() = 0;
    /// @brief Returns the port number.
    inline unsigned short getPort() const { return _port; }
    /// @brief Returns the transport protocol used by this port.
    inline LightBird::INetwork::Transport getTransport() const { return _transport; }
    /// @brief Returns the names of the protocols used to communicate with the
    /// clients connected to this port.
    inline const QStringList &getProtocols() const { return _protocols; }
    /// @brief Returns the maximum number of clients connected to this port.
    inline unsigned int getMaxClients() const { return _maxClients; }

    // IReadWrite
    virtual void    read(Client *client) = 0;
    virtual void    write(Client *client, const QByteArray &data) = 0;

protected:
    Port(const Port &);
    Port &operator=(const Port &);

    /// @brief The main method of the thread.
    void            run() = 0;
    /// @brief Returns the shared pointer of a client.
    QSharedPointer<Client> _getClient(Client *client);

    QList<QSharedPointer<Client> > _clients; ///< Contains the list of all the clients connected to this port.
    mutable QReadWriteLock _mutex; ///< Makes this class is thread safe.
    LightBird::INetwork::Transport _transport; ///< The transport protocol used by the port.
    QStringList _protocols; ///< The names of the protocols used to communicate with the clients connected to this port.
    unsigned short _port; ///< The number of the port.
    unsigned int _maxClients; ///< The maximum number of clients simultaneously connected.
};

#endif // PORT_H
