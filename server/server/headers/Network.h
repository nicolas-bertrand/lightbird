#ifndef NETWORK_H
# define NETWORK_H

# include <QList>
# include <QObject>
# include <QReadWriteLock>
# include <QString>
# include <QStringList>

# include "INetwork.h"

# include "Clients.h"
# include "Future.hpp"
# include "Port.h"

/// @brief Manages the network.
///
/// This class is thread safe. All the publics methods of this class emit
/// a signal that allows to execute their operations in the main thread
/// of the server, not in the thread from which they are called.
/// Users can use the Future returned to wait until the operation is done,
/// and get its result.
class Network : public QObject
{
    Q_OBJECT

public:
    /// @brief Returns the instance of the network.
    static Network  *instance(QObject *parent = 0);

    /// @brief Created a new port from which the server will listen.
    /// @param port : The port to listen.
    /// @param protocols : The protocols used by the server to communicate
    /// with the clients connected to this port.
    /// @param transport : The transport protocol used to route the data through
    /// the network via this port.
    /// @param maxClients : The maximum number of clients simultaneously connected.
    /// When the number of client reach this limit, new connections are waiting
    /// that a connected client disconnect.
    /// @return The future result of the action, e.g true if the port has been created.
    Future<bool>    addPort(unsigned short port, const QStringList &protocols = QStringList(),
                            LightBird::INetwork::Transport transport = LightBird::INetwork::TCP,
                            unsigned int maxClients = ~0);
    /// @brief Remove a port. This may take some time since all the operations
    /// made on the removed port have to be finished and stopped.
    /// @param port : The port to remove.
    /// @return The future result of the action, e.g false if the port is not valid.
    Future<bool>    removePort(unsigned short port);
    /// @brief Allows to get informations on an opened port.
    /// @param port : The port to get.
    /// @param protocols : The names of the protocols used by the port.
    /// @param transport : The transport protocol of the port.
    /// @param maxClients : The maximum number of clients simultaneously connected,
    /// allowed by the port.
    /// @return True if the port exists.
    bool            getPort(unsigned short port, QStringList &protocols, LightBird::INetwork::Transport &transport, unsigned int &maxClients);
    /// @brief Returns the list of the open ports. Users can use getPort() to get
    /// more detailed informations about a specific port.
    /// @return The list of the opened ports on the server.
    QList<unsigned short>   getPorts();
    /// @brief Allows to get the informations of a client.
    bool            getClient(const QString &id, LightBird::INetwork::Client &client);
    /// @brief Returns the list of the clients connected to a particular port.
    /// A negative number returns the clients in CLIENT mode.
    QStringList     getClients(int port = -1);
    /// @brief Connects the server to a new client.
    /// @see LightBird::INetwork::connect
    Future<QString> connect(const QHostAddress &address, quint16 port,
                            const QStringList &protocols = QStringList(),
                            LightBird::INetwork::Transport transport = LightBird::INetwork::TCP,
                            int wait = -1);
    /// @brief Disconnects a client from the server.
    Future<bool>    disconnect(const QString &id);
    /// @brief Asks the engine to call IDoSend in order to generate a request
    /// that will be send to a client.
    bool            send(const QString &idClient, const QString &idPlugin, const QString &protocol = "");

private:
    Network(QObject *parent = 0);
    ~Network();
    Network(const Network &);
    Network         *operator=(const Network &);

    QMap<unsigned short, Port *>    ports;      ///< The list of the listening ports
    QReadWriteLock                  lockPorts;  ///< Serure the ports.
    Clients                         clients;    ///< Manages the clients in CLIENT mode.
    static Network                  *_instance; ///< The instance of the singleton that manage the network.

    // These signals and slots are used to execute the actions on the network
    // from the main thread of the server, not in the thread from which they are called.
private slots:
    void            _addPort(unsigned short port, const QStringList &protocols,
                             LightBird::INetwork::Transport transport,
                             unsigned int maxClients, Future<bool> *future);
    void            _removePort(unsigned short port, Future<bool> *future);
    void            _getClient(const QString &id, LightBird::INetwork::Client *client, void *thread, Future<bool> *future);
    void            _getClients(int port, Future<QStringList> *future);
    void            _connect(const QHostAddress &address, quint16 port, const QStringList &protocols,
                             LightBird::INetwork::Transport transport, int wait, Future<QString> *future);
    void            _disconnect(const QString &id, Future<bool> *future);
    /// @brief Delete and remove a port from the ports list. The port must be close, and all the work made on it finished.
    void            _destroyPort(unsigned short port);

signals:
    void            addPortSignal(unsigned short port, const QStringList &protocols,
                                  LightBird::INetwork::Transport transport,
                                  unsigned int maxClients, Future<bool> *future);
    void            removePortSignal(unsigned short port, Future<bool> *future);
    void            getClientSignal(const QString &id, LightBird::INetwork::Client *client, void *thread, Future<bool> *future);
    void            getClientsSignal(int port, Future<QStringList> *future);
    void            connectSignal(const QHostAddress &address, quint16 port, const QStringList &protocols,
                                  LightBird::INetwork::Transport transport, int wait, Future<QString> *future);
    void            disconnectSignal(const QString &id, Future<bool> *future);
};

#endif // NETWORK_H
