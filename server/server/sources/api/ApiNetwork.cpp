#include "Log.h"
#include "Network.h"
#include "ApiNetwork.h"

ApiNetwork::ApiNetwork(const QString &id)
{
    this->id = id;
}

ApiNetwork::~ApiNetwork()
{
}

QSharedPointer<LightBird::IFuture<bool> >   ApiNetwork::addPort(unsigned short port, const QStringList &protocols,
                                            LightBird::INetwork::Transports transport, unsigned int maxClients)
{
    return (QSharedPointer<LightBird::IFuture<bool> >(new Future<bool>(Network::instance()->addPort(port, protocols, transport, maxClients))));
}

QSharedPointer<LightBird::IFuture<bool> >   ApiNetwork::removePort(unsigned short port)
{
    return (QSharedPointer<LightBird::IFuture<bool> >(new Future<bool>(Network::instance()->removePort(port))));
}

bool                                        ApiNetwork::getPort(unsigned short port, QStringList &protocols, LightBird::INetwork::Transports &transport, unsigned int &maxClients)
{
    return (Network::instance()->getPort(port, protocols, transport, maxClients));
}

QList<unsigned short>                       ApiNetwork::getPorts()
{
    return (Network::instance()->getPorts());
}

bool                                        ApiNetwork::getClient(const QString &id, LightBird::INetwork::Client &client)
{
    return (Network::instance()->getClient(id, client));
}

QStringList                                 ApiNetwork::getClients(unsigned short port)
{
    return (Network::instance()->getClients(port));
}

QSharedPointer<LightBird::IFuture<bool> >   ApiNetwork::disconnect(const QString &id)
{
    return (QSharedPointer<LightBird::IFuture<bool> >(new Future<bool>(Network::instance()->disconnect(id))));
}
