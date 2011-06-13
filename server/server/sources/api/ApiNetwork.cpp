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

QSharedPointer<LightBird::IFuture<bool> >   ApiNetwork::addPort(unsigned short port,
                                                                const QStringList &protocols,
                                                                LightBird::INetwork::Transports transport,
                                                                unsigned int maxClients)
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

QStringList                                 ApiNetwork::getClients()
{
    return (Network::instance()->getClients());
}

QStringList                                 ApiNetwork::getClients(unsigned short port)
{
    return (Network::instance()->getClients(port));
}

QSharedPointer<LightBird::IFuture<QString> > ApiNetwork::connect(const QHostAddress &address,
                                                                 quint16 port,
                                                                 const QStringList &p,
                                                                 LightBird::INetwork::Transports transport,
                                                                 int wait)
{
    QStringList protocols = p;

    // If protocols is empty all the protocols are accepted
    if (protocols.isEmpty())
        protocols << "all";
    return (QSharedPointer<LightBird::IFuture<QString> >(new Future<QString>(Network::instance()->connect(address, port, protocols, transport, wait))));
}

QSharedPointer<LightBird::IFuture<bool> >   ApiNetwork::disconnect(const QString &id)
{
    return (QSharedPointer<LightBird::IFuture<bool> >(new Future<bool>(Network::instance()->disconnect(id))));
}

bool                                        ApiNetwork::send(const QString &idClient, const QString &protocol)
{
    return (Network::instance()->send(idClient, this->id, protocol));
}
