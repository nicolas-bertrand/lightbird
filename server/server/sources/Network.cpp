#include <QSharedPointer>

#include "Defines.h"
#include "Log.h"
#include "Network.h"
#include "PortTcp.h"
#include "PortUdp.h"

Network     *Network::_instance = NULL;

Network     *Network::instance(QObject *parent)
{
    if (Network::_instance == NULL)
        Network::_instance = new Network(parent);
    return (Network::_instance);
}

Network::Network(QObject *parent) : QObject(parent)
{
    qRegisterMetaType<QHostAddress>("QHostAddress");
    qRegisterMetaType<LightBird::INetwork::Transport>("LightBird::INetwork::Transport");
    qRegisterMetaType<LightBird::INetwork::Client>("LightBird::INetwork::Client*");
    QObject::connect(this, SIGNAL(addPortSignal(unsigned short,QStringList,LightBird::INetwork::Transport,unsigned int,Future<bool>*)),
                     this, SLOT(_addPort(unsigned short,QStringList,LightBird::INetwork::Transport,unsigned int,Future<bool>*)));
    QObject::connect(this, SIGNAL(removePortSignal(unsigned short,Future<bool>*)), this, SLOT(_removePort(unsigned short,Future<bool>*)));
    QObject::connect(this, SIGNAL(getClientSignal(QString,LightBird::INetwork::Client*,void*,Future<bool>*)),
                     this, SLOT(_getClient(QString,LightBird::INetwork::Client*,void*,Future<bool>*)));
    QObject::connect(this, SIGNAL(getClientsSignal(int,Future<QStringList>*)),
                     this, SLOT(_getClients(int,Future<QStringList>*)));
    QObject::connect(this, SIGNAL(connectSignal(QHostAddress,quint16,QStringList,LightBird::INetwork::Transport,int,Future<QString>*)),
                     this, SLOT(_connect(QHostAddress,quint16,QStringList,LightBird::INetwork::Transport,int,Future<QString>*)));
    QObject::connect(this, SIGNAL(disconnectSignal(QString,Future<bool>*)), this, SLOT(_disconnect(QString,Future<bool>*)));
}

Network::~Network()
{
    Log::trace("Network destroyed!", "Network", "~Network");
}

Future<bool>        Network::addPort(unsigned short port, const QStringList &protocols, LightBird::INetwork::Transport transport, unsigned int maxClients)
{
    Future<bool>    *future = new Future<bool>(false);
    Future<bool>    result(*future);

    emit addPortSignal(port, protocols, transport, maxClients, future);
    return (result);
}

Future<bool>        Network::removePort(unsigned short port)
{
    Future<bool>    *future = new Future<bool>(false);
    Future<bool>    result(*future);

    emit removePortSignal(port, future);
    return (result);
}

bool                Network::getPort(unsigned short port, QStringList &protocols, LightBird::INetwork::Transport &transport, unsigned int &maxClients)
{
    bool            result = false;

    if (!this->lockPorts.tryLockForRead(MAXTRYLOCK))
    {
        Log::error("Deadlock", "Network", "getPort");
        return (false);
    }
    // If the port exists
    if (this->ports.contains(port))
    {
        // Stores its informations in the parameters
        protocols = this->ports[port]->getProtocols();
        maxClients = this->ports[port]->getMaxClients();
        transport = LightBird::INetwork::TCP;
        if (qobject_cast<PortUdp *>(this->ports[port]))
            transport = LightBird::INetwork::UDP;
        result = true;
    }
    this->lockPorts.unlock();
    return (result);
}

QList<unsigned short>   Network::getPorts()
{
    QList<unsigned short>                   ports;

    if (!this->lockPorts.tryLockForRead(MAXTRYLOCK))
    {
        Log::error("Deadlock", "Network", "getPorts");
        return (ports);
    }
    ports = this->ports.keys();
    this->lockPorts.unlock();
    return (ports);
}

bool    Network::getClient(const QString &id, LightBird::INetwork::Client &client)
{
    Future<bool> *future = new Future<bool>(false);
    Future<bool> result(*future);

    emit getClientSignal(id, &client, QThread::currentThread(), future);
    return (result.getResult());
}

QStringList             Network::getClients(int port)
{
    Future<QStringList> *future = new Future<QStringList>();
    Future<QStringList> result(*future);

    emit getClientsSignal(port, future);
    return (result.getResult());
}

Future<QString>     Network::connect(const QHostAddress &address, quint16 port, const QStringList &protocols,
                                     LightBird::INetwork::Transport transport, int wait)
{
    Future<QString> *future = new Future<QString>();
    Future<QString> result(*future);

    emit connectSignal(address, port, protocols, transport, wait, future);
    return (result);
}

Future<bool>        Network::disconnect(const QString &id)
{
    Future<bool>    *future = new Future<bool>(false);
    Future<bool>    result(*future);

    emit disconnectSignal(id, future);
    return (result);
}

bool        Network::send(const QString &idClient, const QString &idPlugin, const QString &protocol)
{
    bool    result;

    if (!this->lockPorts.tryLockForWrite(MAXTRYLOCK))
    {
        Log::error("Deadlock", "Network", "send");
        return (false);
    }
    result = this->clients.send(idClient, idPlugin, protocol);
    this->lockPorts.unlock();
    return (result);
}

void    Network::_addPort(unsigned short port, const QStringList &protocols, LightBird::INetwork::Transport transport, unsigned int maxClients, Future<bool> *future)
{
    Port                            *p;
    QSharedPointer<Future<bool> >   f(future);

    if (!this->lockPorts.tryLockForWrite(MAXTRYLOCK))
    {
        Log::error("Deadlock", "Network", "_addPort");
        return ;
    }
    // If the port already exists, we can't add it
    if (this->ports.contains(port))
    {
        Log::error("The port is already listening", Properties("port", port).add("protocols", protocols.join(" ")).add("transport", (transport == LightBird::INetwork::TCP ? "TCP" : "UDP")), "Network", "_addPort");
        this->lockPorts.unlock();
        return ;
    }
    // Creates the port
    if (transport == LightBird::INetwork::TCP)
        p = new PortTcp(port, protocols, maxClients, this);
    else
        p = new PortUdp(port, protocols, maxClients, this);
    // If the port is not listening, an error occured
    if (!p->isListening())
    {
        delete p;
        this->lockPorts.unlock();
        return ;
    }
    // Otherwise, the port is listening the network, and we add it to the port list
    this->ports[port] = p;
    f->setResult(true);
    this->lockPorts.unlock();
}

void    Network::_removePort(unsigned short port, Future<bool> *future)
{
    Port                            *p;
    QSharedPointer<Future<bool> >   f(future);

    if (!this->lockPorts.tryLockForWrite(MAXTRYLOCK))
    {
        Log::error("Deadlock", "Network", "_removePort");
        return ;
    }
    // If the port doesn't exists
    if (!this->ports.contains(port))
    {
        Log::warning("The port doen't exists", Properties("port", port), "Network", "_removePort");
        this->lockPorts.unlock();
        return ;
    }
    // If the port is already removing
    if (!this->ports[port]->isListening())
    {
        Log::warning("The port is already removing", Properties("port", port), "Network", "_removePort");
        this->lockPorts.unlock();
        return ;
    }
    p = this->ports[port];
    // Since the next operation may take some time since all the clients have to finish their tasks,
    // we connect to a signal that will be emited when all the clients of this port will be destroyed,
    // so that Network can definitely remove this port.
    QObject::connect(p, SIGNAL(allClientsRemoved(unsigned short)), this, SLOT(_destroyPort(unsigned short)), Qt::QueuedConnection);
    // Forbid new connections to this port and removes all the remaining connected clients on the port
    p->stopListening();
    f->setResult(true);
    this->lockPorts.unlock();
}

void        Network::_getClient(const QString &id, LightBird::INetwork::Client *client, void *thread, Future<bool> *future)
{
    bool    found = false;

    if (!this->lockPorts.tryLockForRead(MAXTRYLOCK))
    {
        Log::error("Deadlock", "Network", "_getClient");
        return ;
    }
    QMapIterator<unsigned short, Port *> it(this->ports);
    while (it.hasNext())
        if (it.next().value()->getClient(id, client, thread, future))
            found = true;
    if (!found)
        found = this->clients.getClient(id, client, thread, future);
    this->lockPorts.unlock();
    if (!found)
        delete future;
}

void        Network::_getClients(int port, Future<QStringList> *future)
{
    QSharedPointer<Future<QStringList> >    f(future);

    if (!this->lockPorts.tryLockForRead(MAXTRYLOCK))
    {
        Log::error("Deadlock", "Network", "_getClients");
        return ;
    }
    // Gets the clients in CLIENT mode
    if (port < 0)
        future->setResult(this->clients.getClients());
    // Gets the clients of the port
    else if (this->ports.contains(port))
        future->setResult(this->ports[port]->getClients());
    this->lockPorts.unlock();
}

void        Network::_connect(const QHostAddress &address, quint16 port, const QStringList &protocols,
                              LightBird::INetwork::Transport transport, int wait, Future<QString> *future)
{
    if (!this->lockPorts.tryLockForWrite(MAXTRYLOCK))
    {
        Log::error("Deadlock", "Network", "_connect");
        return ;
    }
    this->clients.connect(address, port, protocols, transport, wait, future);
    this->lockPorts.unlock();
}

void        Network::_disconnect(const QString &id, Future<bool> *future)
{
    QSharedPointer<Future<bool> >   f(future);
    bool                            found = false;

    if (!this->lockPorts.tryLockForWrite(MAXTRYLOCK))
    {
        Log::error("Deadlock", "Network", "_disconnect");
        return ;
    }
    QMapIterator<unsigned short, Port *> it(this->ports);
    // Searches the port that manages the client, and disconnect it
    while (it.hasNext() && !found)
        if (it.next().value()->disconnect(id))
            found = true;
    if (!found)
        found = this->clients.disconnect(id);
    f->setResult(found);
    this->lockPorts.unlock();
}

void        Network::_destroyPort(unsigned short port)
{
    Port    *p;

    if (!this->lockPorts.tryLockForWrite(MAXTRYLOCK))
    {
        Log::error("Deadlock", "Network", "_destroyPort");
        return ;
    }
    // If the port exists and is not listening on the network
    if (this->ports.contains(port) && !(p = this->ports[port])->isListening())
    {
        // Remove it from the list
        this->ports.remove(port);
        // And delete it later
        p->deleteLater();
    }
    this->lockPorts.unlock();
}
