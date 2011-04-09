#ifndef	APINETWORK_H
# define APINETWORK_H

# include <QObject>

# include "INetwork.h"

/// @brief Implements the interface INetwork that allows to access to the
/// server network features.
class ApiNetwork : public QObject,
                   public LightBird::INetwork
{
    Q_OBJECT

public:
    ApiNetwork(const QString &id, QObject *parent = 0);
    ~ApiNetwork();

    /// @see LightBird::INetwork::addPort
    QSharedPointer<LightBird::IFuture<bool> >   addPort(unsigned short port, const QStringList &protocols = QStringList(),
                                                LightBird::INetwork::Transports transport = LightBird::INetwork::TCP,
                                                unsigned int maxClients = ~0);
    /// @see LightBird::INetwork::removePort
    QSharedPointer<LightBird::IFuture<bool> >   removePort(unsigned short port);
    /// @see LightBird::INetwork::getPort
    bool    getPort(unsigned short port, QStringList &protocols, LightBird::INetwork::Transports &transport, unsigned int &maxClients);
    /// @see LightBird::INetwork::getPorts
    QList<unsigned short>                       getPorts();
    /// @see LightBird::INetwork::getClient
    bool                                        getClient(const QString &id, LightBird::INetwork::Client &client);
    /// @see LightBird::INetwork::getClients
    QStringList                                 getClients(unsigned short port);
    /// @see LightBird::INetwork::disconnect
    QSharedPointer<LightBird::IFuture<bool> >   disconnect(const QString &id);

private:
    ApiNetwork();
    ApiNetwork(const ApiNetwork &);
    ApiNetwork* operator=(const ApiNetwork &);

    QString id; ///< The id of the plugin for which the object has been created.
};

#endif // APINETWORK_H
