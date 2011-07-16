#include <QSharedPointer>

#include "Future.hpp"
#include "Log.h"
#include "Network.h"
#include "PortTcp.h"
#include "PortUdp.h"
#include "SmartMutex.h"

Network     *Network::_instance = NULL;

Network     *Network::instance(QObject *parent)
{
    if (Network::_instance == NULL)
        Network::_instance = new Network(parent);
    return (Network::_instance);
}

Network::Network(QObject *parent) : QObject(parent)
{
}

Network::~Network()
{
    Log::trace("Network destroyed!", "Network", "~Network");
}

bool            Network::openPort(unsigned short port, const QStringList &protocols, LightBird::INetwork::Transport transport, unsigned int maxClients)
{
    SmartMutex  mutex(this->mutex, "Network", "openPort");
    Port        *p;

    if (!mutex)
        return (false);
    // The port already exists
    if (this->ports.contains(port))
    {
        Log::error("The port is already listening", Properties("port", port).add("protocols", protocols.join(" "))
                   .add("transport", (transport == LightBird::INetwork::TCP ? "TCP" : "UDP")), "Network", "openPort");
        return (false);
    }
    // Creates the port
    if (transport == LightBird::INetwork::TCP)
        p = new PortTcp(port, protocols, maxClients);
    else
        p = new PortUdp(port, protocols, maxClients);
    // If the port is not listening an error occured
    if (!p->isListening())
    {
        delete p;
        return (false);
    }
    // Signals when the thread is finished, in order do delete it
    QObject::connect(p, SIGNAL(finished()), this, SLOT(_finished()), Qt::QueuedConnection);
    // The port is listening the network and we add it to the list
    this->ports[port] = p;
    return (true);
}

bool            Network::closePort(unsigned short port)
{
    SmartMutex  mutex(this->mutex, SmartMutex::READ, "Network", "closePort");
    Port        *p;

    if (!mutex)
        return (false);
    // The port doesn't exists
    if (!this->ports.contains(port))
    {
        Log::warning("The port doesn't exists", Properties("port", port), "Network", "closePort");
        return (false);
    }
    // If the port is already removing
    if (!this->ports[port]->isListening())
    {
        Log::warning("The port is already removing", Properties("port", port), "Network", "closePort");
        return (false);
    }
    p = this->ports[port];
    // Forbids new connections to this port and removes all the remaining connected clients
    p->close();
    return (true);
}

bool            Network::getPort(unsigned short port, QStringList &protocols, LightBird::INetwork::Transport &transport, unsigned int &maxClients)
{
    SmartMutex  mutex(this->mutex, SmartMutex::READ, "Network", "getPort");

    if (!mutex)
        return (false);
    if (!this->ports.contains(port))
        return (false);
    transport = this->ports[port]->getTransport();
    protocols = this->ports[port]->getProtocols();
    maxClients = this->ports[port]->getMaxClients();
    return (true);
}

QList<unsigned short>   Network::getPorts()
{
    SmartMutex          mutex(this->mutex, SmartMutex::READ, "Network", "getPorts");

    if (mutex)
        return (this->ports.keys());
    return (QList<unsigned short>());
}

bool            Network::getClient(const QString &id, LightBird::INetwork::Client &client)
{
    SmartMutex  mutex(this->mutex, SmartMutex::READ, "Network", "getClient");
    bool        found;

    if (!mutex)
        return (false);
    // The client is connected in CLIENT mode
    Future<bool> clients(this->clients.getClient(id, client, found));
    if (found)
    {
        mutex.unlock();
        return (clients.getResult());
    }
    // Otherwise it is connected to a port
    QMapIterator<unsigned short, Port *> it(this->ports);
    while (it.hasNext())
    {
        // If the client has been found, we wait its informations outside the mutex
        Future<bool> future(it.next().value()->getClient(id, client, found));
        if (found)
        {
            mutex.unlock();
            return (future.getResult());
        }
    }
    return (false);
}

QStringList     Network::getClients(int port)
{
    SmartMutex  mutex(this->mutex, SmartMutex::READ, "Network", "getClients");
    QStringList result;

    if (!mutex)
        return (QStringList());
    // Gets the clients in CLIENT mode
    if (port < 0)
        result = this->clients.getClients();
    // Gets the clients of the port
    else if (this->ports.contains(port))
        result = this->ports[port]->getClients();
    return (result);
}

Future<QString>     Network::connect(const QHostAddress &address, quint16 port, const QStringList &protocols,
                                     LightBird::INetwork::Transport transport, int wait)
{
    return (this->clients.connect(address, port, protocols, transport, wait));
}

bool                Network::disconnect(const QString &id)
{
    SmartMutex      mutex(this->mutex, SmartMutex::READ, "Network", "disconnect");
    bool            found = false;

    if (!mutex)
        return (false);
    found = this->clients.disconnect(id);
    QMapIterator<unsigned short, Port *> it(this->ports);
    while (it.hasNext() && !found)
        if (it.next().value()->disconnect(id))
            found = true;
    return (found);
}

bool            Network::send(const QString &idClient, const QString &idPlugin, const QString &protocol)
{
    return (this->clients.send(idClient, idPlugin, protocol));
}

void            Network::_finished()
{
    SmartMutex  mutex(this->mutex, "Network", "_finished");

    if (!mutex)
        return ;
    // Searches the ports finished
    QMutableMapIterator<unsigned short, Port *> it(this->ports);
    while (it.hasNext())
    {
        it.next();
        if (it.value()->isFinished())
        {
            delete it.value();
            it.remove();
        }
    }
}
