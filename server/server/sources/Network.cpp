#include <QSharedPointer>

#include "Future.hpp"
#include "Log.h"
#include "Network.h"
#include "PortTcp.h"
#include "PortUdp.h"
#include "Server.h"
#include "SmartMutex.h"

Network::Network(QObject *parent) : QObject(parent)
{
}

Network::~Network()
{
    this->shutdown();
    Log::trace("Network destroyed!", "Network", "~Network");
}

bool            Network::openPort(unsigned short port, const QStringList &protocols, LightBird::INetwork::Transport transport, unsigned int maxClients)
{
    SmartMutex  mutex(this->mutex, "Network", "openPort");
    Port        *p;

    if (!mutex)
        return (false);
    // The port already exists
    if (this->ports[transport].contains(port))
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
    this->ports[transport][port] = p;
    return (true);
}

bool            Network::closePort(unsigned short port, LightBird::INetwork::Transport transport)
{
    SmartMutex  mutex(this->mutex, SmartMutex::READ, "Network", "closePort");

    if (!mutex)
        return (false);
    // The port doesn't exists
    if (!this->ports[transport].contains(port))
    {
        Log::warning("The port doesn't exists", Properties("port", port), "Network", "closePort");
        return (false);
    }
    // If the port is already removing
    if (!this->ports[transport][port]->isListening())
    {
        Log::warning("The port is already removing", Properties("port", port), "Network", "closePort");
        return (false);
    }
    // Forbids new connections to this port and removes all the remaining connected clients
    this->ports[transport][port]->close();
    return (true);
}

bool            Network::getPort(unsigned short port, QStringList &protocols, unsigned int &maxClients, LightBird::INetwork::Transport transport) const
{
    SmartMutex  mutex(this->mutex, SmartMutex::READ, "Network", "getPort");

    if (!mutex)
        return (false);
    if (!this->ports[transport].contains(port))
        return (false);
    protocols = this->ports[transport][port]->getProtocols();
    maxClients = this->ports[transport][port]->getMaxClients();
    return (true);
}

QList<unsigned short>   Network::getPorts(LightBird::INetwork::Transport transport) const
{
    SmartMutex          mutex(this->mutex, SmartMutex::READ, "Network", "getPorts");

    if (mutex)
        return (this->ports[transport].keys());
    return (QList<unsigned short>());
}

bool            Network::getClient(const QString &id, LightBird::INetwork::Client &client) const
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
    QMapIterator<LightBird::INetwork::Transport, QMap<unsigned short, Port *> > transport(this->ports);
    while (transport.hasNext())
    {
        QMapIterator<unsigned short, Port *> it(transport.next().value());
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
    }
    return (false);
}

QStringList     Network::getClients(int port, LightBird::INetwork::Transport transport) const
{
    SmartMutex  mutex(this->mutex, SmartMutex::READ, "Network", "getClients");
    QStringList result;

    if (!mutex)
        return (QStringList());
    // Gets the clients in CLIENT mode
    if (port < 0)
        result = this->clients.getClients();
    // Gets the clients of the port
    else if (this->ports[transport].contains(port))
        result = this->ports[transport][port]->getClients();
    return (result);
}

Future<QString> Network::connect(const QHostAddress &address, quint16 port, const QStringList &protocols,
                                 LightBird::INetwork::Transport transport, int wait)
{
    return (this->clients.connect(address, port, protocols, transport, wait));
}

bool            Network::disconnect(const QString &id)
{
    SmartMutex  mutex(this->mutex, SmartMutex::READ, "Network", "disconnect");
    bool        found = false;

    if (!mutex)
        return (false);
    found = this->clients.disconnect(id);
    QMapIterator<LightBird::INetwork::Transport, QMap<unsigned short, Port *> > transport(this->ports);
    while (transport.hasNext() && !found)
    {
        QMapIterator<unsigned short, Port *> it(transport.next().value());
        while (it.hasNext() && !found)
            if (it.next().value()->disconnect(id))
                found = true;
    }
    return (found);
}

bool            Network::send(const QString &idClient, const QString &idPlugin, const QString &protocol)
{
    SmartMutex  mutex(this->mutex, SmartMutex::READ, "Network", "send");

    if (!mutex)
        return (false);
    if (this->clients.send(idClient, idPlugin, protocol))
        return (true);
    QMapIterator<LightBird::INetwork::Transport, QMap<unsigned short, Port *> > transport(this->ports);
    while (transport.hasNext())
    {
        QMapIterator<unsigned short, Port *> it(transport.next().value());
        while (it.hasNext())
            if (it.next().value()->send(idClient, protocol))
                return (true);
    }
    return (false);
}

bool            Network::receive(const QString &idClient, const QString &protocol)
{
    return (this->clients.receive(idClient, protocol));
}

void            Network::shutdown()
{
    SmartMutex  mutex(this->mutex, "Network", "shutdown");

    if (!mutex)
        return ;
    // Quits the threads
    QMapIterator<LightBird::INetwork::Transport, QMap<unsigned short, Port *> > transport(this->ports);
    while (transport.hasNext())
    {
        QMapIterator<unsigned short, Port *> it(transport.next().value());
        while (it.hasNext())
            it.next().value()->quit();
    }
    transport.toFront();
    // Waits until the threads are finished
    while (transport.hasNext())
    {
        QMapIterator<unsigned short, Port *> it(transport.next().value());
        while (it.hasNext())
            it.next().value()->wait();
    }
    transport.toFront();
    // Destroys the ports
    while (transport.hasNext())
    {
        QMapIterator<unsigned short, Port *> it(transport.next().value());
        while (it.hasNext())
            delete it.next().value();
    }
    this->ports.clear();
}

void            Network::_finished()
{
    SmartMutex  mutex(this->mutex, "Network", "_finished");

    if (!mutex)
        return ;
    // Searches the ports finished
    QMapIterator<LightBird::INetwork::Transport, QMap<unsigned short, Port *> > transport(this->ports);
    while (transport.hasNext())
    {
        QMutableMapIterator<unsigned short, Port *> it(this->ports[transport.next().key()]);
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
}

Network *Network::instance()
{
    return (Server::instance().getNetwork());
}
