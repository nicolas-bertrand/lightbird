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
class Network : public QObject
{
    Q_OBJECT

public:
    /// @brief Returns the instance of the Network.
    static Network  *instance(QObject *parent = 0);

    /// @see LightBird::INetwork::openPort
    bool            openPort(unsigned short port, const QStringList &protocols = QStringList(),
                            LightBird::INetwork::Transport transport = LightBird::INetwork::TCP,
                            unsigned int maxClients = ~0);
    /// @see LightBird::INetwork::closePort
    bool            closePort(unsigned short port);
    /// @see LightBird::INetwork::getPort
    bool            getPort(unsigned short port, QStringList &protocols, LightBird::INetwork::Transport &transport, unsigned int &maxClients);
    /// @see LightBird::INetwork::getPorts
    QList<unsigned short>   getPorts();
    /// @see LightBird::INetwork::getClient
    bool            getClient(const QString &id, LightBird::INetwork::Client &client);
    /// @brief Returns the list of the clients connected to a particular port.
    /// A negative number returns the clients connected in CLIENT mode.
    /// @see LightBird::INetwork::getClients
    QStringList     getClients(int port = -1);
    /// @see LightBird::INetwork::connect
    Future<QString> connect(const QHostAddress &address, quint16 port,
                            const QStringList &protocols = QStringList(),
                            LightBird::INetwork::Transport transport = LightBird::INetwork::TCP,
                            int wait = -1);
    /// @see LightBird::INetwork::disconnect
    bool            disconnect(const QString &id);
    /// @see LightBird::INetwork::send
    bool            send(const QString &idClient, const QString &idPlugin, const QString &protocol = "");

private:
    Network(QObject *parent = 0);
    ~Network();
    Network(const Network &);
    Network         *operator=(const Network &);

    QMap<unsigned short, Port *>    ports;      ///< The list of the open ports
    Clients                         clients;    ///< Manages the clients in CLIENT mode.
    QReadWriteLock                  mutex;      ///< Makes the class thread safe.
    static Network                  *_instance; ///< The instance of the singleton.

private slots:
    /// @brief Called when a port thread is finished in order to delete it.
    void            _finished();
};

#endif // NETWORK_H
