#include "ApiNetwork.h"
#include "Log.h"
#include "Network.h"

ApiNetwork::ApiNetwork(const QString &id)
{
    this->id = id;
}

ApiNetwork::~ApiNetwork()
{
}

bool    ApiNetwork::openPort(unsigned short port, const QStringList &protocols,
                            LightBird::INetwork::Transport transport, unsigned int maxClients)
{
    return (Network::instance()->openPort(port, protocols, transport, maxClients));
}

bool    ApiNetwork::closePort(unsigned short port, LightBird::INetwork::Transport transport)
{
    return (Network::instance()->closePort(port, transport));
}

bool    ApiNetwork::getPort(unsigned short port, QStringList &protocols, unsigned int &maxClients, LightBird::INetwork::Transport transport) const
{
    return (Network::instance()->getPort(port, protocols, maxClients, transport));
}

QList<unsigned short>   ApiNetwork::getPorts(LightBird::INetwork::Transport transport) const
{
    return (Network::instance()->getPorts(transport));
}

bool    ApiNetwork::getClient(const QString &id, LightBird::INetwork::Client &client) const
{
    return (Network::instance()->getClient(id, client));
}

QStringList ApiNetwork::getClients() const
{
    return (Network::instance()->getClients());
}

QStringList ApiNetwork::getClients(unsigned short port, LightBird::INetwork::Transport transport) const
{
    return (Network::instance()->getClients(port, transport));
}

QSharedPointer<LightBird::IFuture<QString> > ApiNetwork::connect(const QHostAddress &address,
                                                                 quint16 port,
                                                                 const QStringList &p,
                                                                 LightBird::INetwork::Transport transport,
                                                                 int wait)
{
    QStringList protocols = p;

    // If protocols is empty all the protocols are accepted
    if (protocols.isEmpty())
        protocols << "all";
    return (QSharedPointer<LightBird::IFuture<QString> >(new Future<QString>(Network::instance()->connect(address, port, protocols, transport, wait))));
}

bool    ApiNetwork::disconnect(const QString &id)
{
    return (Network::instance()->disconnect(id));
}

bool    ApiNetwork::send(const QString &idClient, const QString &protocol)
{
    return (Network::instance()->send(idClient, this->id, protocol));
}
