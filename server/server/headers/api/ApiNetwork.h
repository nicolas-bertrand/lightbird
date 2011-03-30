#ifndef	APINETWORK_H
# define APINETWORK_H

# include <QObject>

# include "INetwork.h"

/// @brief Implements the interface INetwork that allows to access to the
/// server network features.
class ApiNetwork : public QObject,
                   public Streamit::INetwork
{
    Q_OBJECT

public:
    ApiNetwork(const QString &id, QObject *parent = 0);
    ~ApiNetwork();

    /// @see Streamit::INetwork::addPort
    QSharedPointer<Streamit::IFuture<bool> >    addPort(unsigned short port, const QStringList &protocols = QStringList(),
                                                Streamit::INetwork::Transports transport = Streamit::INetwork::TCP,
                                                unsigned int maxClients = ~0);
    /// @see Streamit::INetwork::removePort
    QSharedPointer<Streamit::IFuture<bool> >    removePort(unsigned short port);
    /// @see Streamit::INetwork::getPort
    bool    getPort(unsigned short port, QStringList &protocols, Streamit::INetwork::Transports &transport, unsigned int &maxClients);
    /// @see Streamit::INetwork::getPorts
    QList<unsigned short>                       getPorts();
    /// @see Streamit::INetwork::getClient
    bool                                        getClient(const QString &id, Streamit::INetwork::Client &client);
    /// @see Streamit::INetwork::getClients
    QStringList                                 getClients(unsigned short port);
    /// @see Streamit::INetwork::disconnect
    QSharedPointer<Streamit::IFuture<bool> >    disconnect(const QString &id);

private:
    ApiNetwork();
    ApiNetwork(const ApiNetwork &);
    ApiNetwork* operator=(const ApiNetwork &);

    QString id; ///< The id of the plugin for which the object has been created.
};

#endif // APINETWORK_H
