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

bool    ApiNetwork::openPort(unsigned short port, const QStringList &protocols, LightBird::INetwork::Transport transport, unsigned int maxClients)
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

QSharedPointer<LightBird::IFuture<QString> > ApiNetwork::connect(const QHostAddress &address, quint16 port, const QStringList &p, LightBird::INetwork::Transport transport, const QVariantMap &informations, const QStringList &contexts, int wait)
{
    QStringList protocols = p;

    // If protocols is empty all the protocols are accepted
    if (protocols.isEmpty())
        protocols << "all";
    return (QSharedPointer<LightBird::IFuture<QString> >(new Future<QString>(Network::instance()->connect(address, port, protocols, transport, informations, contexts, wait))));
}

bool    ApiNetwork::disconnect(const QString &id, bool fatal)
{
    return (Network::instance()->disconnect(id, fatal));
}

bool    ApiNetwork::send(const QString &idClient, const QString &protocol, const QVariantMap &informations)
{
    return (Network::instance()->send(idClient, this->id, protocol, informations));
}

bool    ApiNetwork::receive(const QString &idClient, const QString &protocol, const QVariantMap &informations)
{
    return (Network::instance()->receive(idClient, protocol, informations));
}

bool    ApiNetwork::pause(const QString &idClient, int msec)
{
    return (Network::instance()->pause(idClient, msec));
}

bool    ApiNetwork::resume(const QString &idClient)
{
    return (Network::instance()->resume(idClient));
}

void    ApiNetwork::setDisconnectIdle(const QString &id, qint64 msec, bool fatal)
{
    Network::instance()->setDisconnectIdle(id, msec, fatal);
}

qint64  ApiNetwork::getDisconnectIdle(const QString &id, bool *fatal)
{
    return (Network::instance()->getDisconnectIdle(id, fatal));
}

void    ApiNetwork::setDisconnectTime(const QString &id, const QDateTime &time, bool fatal)
{
    Network::instance()->setDisconnectTime(id, time, fatal);
}

QDateTime ApiNetwork::getDisconnectTime(const QString &id, bool *fatal)
{
    return (Network::instance()->getDisconnectTime(id, fatal));
}
