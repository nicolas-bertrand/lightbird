#include <QSharedPointer>

#include "Future.hpp"
#include "Log.h"
#include "Network.h"
#include "PortTcp.h"
#include "PortUdp.h"
#include "Server.h"
#include "Mutex.h"

#ifdef Q_OS_WIN
# include <WinSock2.h>
# include <Ws2tcpip.h>
#endif // Q_OS_WIN

Network::Network(QObject *parent)
    : QObject(parent)
    , clients(NULL)
{
#ifdef Q_OS_WIN
    // Initializes Winsock on Windows
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (result != 0)
    {
        LOG_FATAL("WSAStartup failed", Properties("result", result), "ServerTcpWindows", "ServerTcpWindows");
        return ;
    }
#endif // Q_OS_WIN

    this->clients = new Clients();
    isInitialized(*this->clients);
}

Network::~Network()
{
    this->shutdown();
#ifdef Q_OS_WIN
    // Cleans Winsock on Windows
    WSACleanup();
#endif // Q_OS_WIN
    LOG_TRACE("Network destroyed!", "Network", "~Network");
}

bool    Network::openPort(unsigned short port, const QStringList &protocols, LightBird::INetwork::Transport transport, const QStringList &contexts, unsigned int maxClients)
{
    Mutex   mutex(this->mutex, "Network", "openPort");
    Port    *p;

    if (!mutex)
        return (false);
    // The port already exists
    if (this->ports[transport].contains(port))
    {
        LOG_ERROR("The port is already listening", Properties("port", port).add("protocols", protocols.join(" "))
                  .add("transport", (transport == LightBird::INetwork::TCP ? "TCP" : "UDP")), "Network", "openPort");
        return (false);
    }
    // Creates the port
    if (transport == LightBird::INetwork::TCP)
        p = new PortTcp(port, protocols, contexts, maxClients);
    else
        p = new PortUdp(port, protocols, contexts, maxClients);
    // If the port is not listening an error occurred
    if (!p->isListening())
    {
        Threads::instance()->removeThread(p);
        delete p;
        return (false);
    }
    // Signals when the thread is finished, in order do delete it
    QObject::connect(p, SIGNAL(finished()), this, SLOT(_finished()), Qt::QueuedConnection);
    // The port is listening the network and we add it to the list
    this->ports[transport][port] = p;
    return (true);
}

bool    Network::closePort(unsigned short port, LightBird::INetwork::Transport transport)
{
    Mutex   mutex(this->mutex, Mutex::READ, "Network", "closePort");

    if (!mutex)
        return (false);
    // The port does not exist
    if (!this->ports[transport].contains(port))
    {
        LOG_WARNING("The port does not exist", Properties("port", port), "Network", "closePort");
        return (false);
    }
    // If the port is already removing
    if (!this->ports[transport][port]->isListening())
    {
        LOG_WARNING("The port is already removing", Properties("port", port), "Network", "closePort");
        return (false);
    }
    // Forbids new connections to this port and removes all the remaining connected clients
    this->ports[transport][port]->close();
    return (true);
}

bool    Network::getPort(unsigned short port, QStringList &protocols, unsigned int &maxClients, LightBird::INetwork::Transport transport) const
{
    Mutex   mutex(this->mutex, Mutex::READ, "Network", "getPort");

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
    Mutex   mutex(this->mutex, Mutex::READ, "Network", "getPorts");

    if (mutex)
        return (this->ports[transport].keys());
    return (QList<unsigned short>());
}

bool    Network::getClient(const QString &id, LightBird::INetwork::Client &client) const
{
    Mutex   mutex(this->mutex, Mutex::READ, "Network", "getClient");
    bool    found;

    if (!mutex)
        return (false);
    // The client is connected in CLIENT mode
    Future<bool> clients(this->clients->getClient(id, client, found));
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

QStringList Network::getClients(int port, LightBird::INetwork::Transport transport) const
{
    Mutex   mutex(this->mutex, Mutex::READ, "Network", "getClients");
    QStringList result;

    if (!mutex)
        return (QStringList());
    // Gets the clients in CLIENT mode
    if (port < 0)
        result = this->clients->getClients();
    // Gets the clients of the port
    else if (this->ports[transport].contains(port))
        result = this->ports[transport][port]->getClients();
    return (result);
}

Future<QString> Network::connect(const QHostAddress &address, quint16 port, const QStringList &protocols, LightBird::INetwork::Transport transport, const QVariantMap &informations, const QStringList &contexts, int wait)
{
    return (this->clients->connect(address, port, protocols, transport, informations, contexts, wait));
}

bool    Network::disconnect(const QString &id, bool fatal)
{
    Mutex   mutex(this->mutex, Mutex::READ, "Network", "disconnect");
    bool    found = false;

    if (!mutex)
        return (false);
    found = this->clients->disconnect(id, fatal);
    QMapIterator<LightBird::INetwork::Transport, QMap<unsigned short, Port *> > transport(this->ports);
    while (transport.hasNext() && !found)
    {
        QMapIterator<unsigned short, Port *> it(transport.next().value());
        while (it.hasNext() && !found)
            if (it.next().value()->disconnect(id, fatal))
                found = true;
    }
    return (found);
}

bool    Network::send(const QString &idClient, const QString &idPlugin, const QString &protocol, const QVariantMap &informations)
{
    Mutex   mutex(this->mutex, Mutex::READ, "Network", "send");

    if (!mutex)
        return (false);
    if (this->clients->send(idClient, idPlugin, protocol, informations))
        return (true);
    QMapIterator<LightBird::INetwork::Transport, QMap<unsigned short, Port *> > transport(this->ports);
    while (transport.hasNext())
    {
        QMapIterator<unsigned short, Port *> it(transport.next().value());
        while (it.hasNext())
            if (it.next().value()->send(idClient, protocol, informations))
                return (true);
    }
    return (false);
}

bool    Network::receive(const QString &idClient, const QString &protocol, const QVariantMap &informations)
{
    return (this->clients->receive(idClient, protocol, informations));
}

bool    Network::pause(const QString &idClient, int msec)
{
    Mutex   mutex(this->mutex, Mutex::READ, "Network", "pause");

    if (!mutex)
        return (false);
    if (this->clients->pause(idClient, msec))
        return (true);
    QMapIterator<LightBird::INetwork::Transport, QMap<unsigned short, Port *> > transport(this->ports);
    while (transport.hasNext())
    {
        QMapIterator<unsigned short, Port *> it(transport.next().value());
        while (it.hasNext())
            if (it.next().value()->pause(idClient, msec))
                return (true);
    }
    return (false);
}

bool    Network::resume(const QString &idClient)
{
    Mutex   mutex(this->mutex, Mutex::READ, "Network", "resume");

    if (!mutex)
        return (false);
    if (this->clients->resume(idClient))
        return (true);
    QMapIterator<LightBird::INetwork::Transport, QMap<unsigned short, Port *> > transport(this->ports);
    while (transport.hasNext())
    {
        QMapIterator<unsigned short, Port *> it(transport.next().value());
        while (it.hasNext())
            if (it.next().value()->resume(idClient))
                return (true);
    }
    return (false);
}

void    Network::setDisconnectIdle(const QString &idClient, qint64 msec, bool fatal)
{
    Mutex   mutex(this->mutex, Mutex::READ, "Network", "setDisconnectIdle");

    if (!mutex)
        return ;
    if (this->clients->setDisconnectIdle(idClient, msec, fatal))
        return ;
    QMapIterator<LightBird::INetwork::Transport, QMap<unsigned short, Port *> > transport(this->ports);
    while (transport.hasNext())
    {
        QMapIterator<unsigned short, Port *> it(transport.next().value());
        while (it.hasNext())
            if (it.next().value()->setDisconnectIdle(idClient, msec, fatal))
                return ;
    }
}

qint64  Network::getDisconnectIdle(const QString &idClient, bool *fatal)
{
    Mutex mutex(this->mutex, Mutex::READ, "Network", "getDisconnectIdle");
    qint64 result = -1;

    if (!mutex)
        return result;
    if (this->clients->getDisconnectIdle(idClient, fatal, result))
        return result;
    QMapIterator<LightBird::INetwork::Transport, QMap<unsigned short, Port *> > transport(this->ports);
    while (transport.hasNext())
    {
        QMapIterator<unsigned short, Port *> it(transport.next().value());
        while (it.hasNext())
            if (it.next().value()->getDisconnectIdle(idClient, fatal, result))
                return result;
    }
    return result;
}

void    Network::setDisconnectTime(const QString &idClient, const QDateTime &time, bool fatal)
{
    Mutex   mutex(this->mutex, Mutex::READ, "Network", "setDisconnectTime");

    if (!mutex)
        return ;
    if (this->clients->setDisconnectTime(idClient, time, fatal))
        return ;
    QMapIterator<LightBird::INetwork::Transport, QMap<unsigned short, Port *> > transport(this->ports);
    while (transport.hasNext())
    {
        QMapIterator<unsigned short, Port *> it(transport.next().value());
        while (it.hasNext())
            if (it.next().value()->setDisconnectTime(idClient, time, fatal))
                return ;
    }
}

QDateTime Network::getDisconnectTime(const QString &idClient, bool *fatal)
{
    Mutex mutex(this->mutex, Mutex::READ, "Network", "getDisconnectTime");
    QDateTime result;

    if (!mutex)
        return result;
    if (this->clients->getDisconnectTime(idClient, fatal, result))
        return result;
    QMapIterator<LightBird::INetwork::Transport, QMap<unsigned short, Port *> > transport(this->ports);
    while (transport.hasNext())
    {
        QMapIterator<unsigned short, Port *> it(transport.next().value());
        while (it.hasNext())
            if (it.next().value()->getDisconnectTime(idClient, fatal, result))
                return result;
    }
    return result;
}

void    Network::shutdown()
{
    Mutex   mutex(this->mutex, "Network", "shutdown");

    if (!mutex)
        return ;
    // Quits the threads
    QMapIterator<LightBird::INetwork::Transport, QMap<unsigned short, Port *> > transport(this->ports);
    while (transport.hasNext())
    {
        QMapIterator<unsigned short, Port *> it(transport.next().value());
        while (it.hasNext())
            it.next().value()->close();
    }
    if (this->clients)
        this->clients->close();
    transport.toFront();
    // Destroys the ports
    while (transport.hasNext())
    {
        QMapIterator<unsigned short, Port *> it(transport.next().value());
        while (it.hasNext())
            delete it.next().value();
    }
    this->ports.clear();
    delete this->clients;
    this->clients = NULL;
}

void    Network::_finished()
{
    Mutex   mutex(this->mutex, "Network", "_finished");

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
