#include "Log.h"
#include "Network.h"
#include "ApiNetwork.h"

ApiNetwork::ApiNetwork(const QString &id, QObject *parent) : QObject(parent)
{
    this->id = id;
}

ApiNetwork::~ApiNetwork()
{
    Log::trace("ApiNetwork destroyed!", Properties("id", this->id), "ApiNetwork", "~ApiNetwork");
}

QSharedPointer<Streamit::IFuture<bool> >    ApiNetwork::addPort(unsigned short port, const QStringList &protocols,
                                            Streamit::INetwork::Transports transport, unsigned int maxClients)
{
    return (QSharedPointer<Streamit::IFuture<bool> >(new Future<bool>(Network::instance()->addPort(port, protocols, transport, maxClients))));
}

QSharedPointer<Streamit::IFuture<bool> >    ApiNetwork::removePort(unsigned short port)
{
    return (QSharedPointer<Streamit::IFuture<bool> >(new Future<bool>(Network::instance()->removePort(port))));
}

bool                                        ApiNetwork::getPort(unsigned short port, QStringList &protocols, Streamit::INetwork::Transports &transport, unsigned int &maxClients)
{
    return (Network::instance()->getPort(port, protocols, transport, maxClients));
}

QList<unsigned short>                       ApiNetwork::getPorts()
{
    return (Network::instance()->getPorts());
}

bool                                        ApiNetwork::getClient(const QString &id, Streamit::INetwork::Client &client)
{
    return (Network::instance()->getClient(id, client));
}

QStringList                                 ApiNetwork::getClients(unsigned short port)
{
    return (Network::instance()->getClients(port));
}

QSharedPointer<Streamit::IFuture<bool> >    ApiNetwork::disconnect(const QString &id)
{
    return (QSharedPointer<Streamit::IFuture<bool> >(new Future<bool>(Network::instance()->disconnect(id))));
}
