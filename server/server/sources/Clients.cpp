#include <QTcpSocket>
#include <QUdpSocket>

#include "Clients.h"
#include "Log.h"
#include "SmartMutex.h"
#include "Threads.h"

Clients::Clients()
{
}

Clients::~Clients()
{
    SmartMutex  mutex(this->mutex);

    QMapIterator<QString, QPair<Future<QString> *, int> > it(this->futures);
    while (it.hasNext())
        delete it.next().value().first;
}

void            Clients::connect(const QHostAddress &address, quint16 port, const QStringList &protocols,
                                 LightBird::INetwork::Transport transport, int wait, Future<QString> *future)
{
    SmartMutex  mutex(this->mutex);

    if (transport == LightBird::INetwork::TCP)
    {
        // Creates the socket. The connection is made il the client thread, via IReadWrite
        QTcpSocket *socket = new QTcpSocket(NULL);
        // Creates the client
        Client *client = new Client(socket, transport, protocols, socket->localPort(), socket->socketDescriptor(),
                                    address, port, socket->peerName(), LightBird::IClient::CLIENT, this);
        this->clients.push_back(client);
        // When the client thread is finished, _finished is called
        QObject::connect(client, SIGNAL(finished()), this, SLOT(_finished()), Qt::QueuedConnection);
        // When new data are received on this socket, Client::read is called
        QObject::connect(socket, SIGNAL(readyRead()), client, SLOT(read()), Qt::QueuedConnection);
        // When the client is disconnected, _disconnected is called
        QObject::connect(socket, SIGNAL(disconnected()), this, SLOT(_disconnected()), Qt::QueuedConnection);
        // Keeps the future in order to set its result in IReadWrite::connect
        this->futures.insert(client->getId(), QPair<Future<QString> *, int>(future, wait));
        client->start();
    }
    else
    {
        QSharedPointer<Future<QString> > f(future);
        QListIterator<Client *> it(this->clients);

        // Ensure that the client is not already connected
        while (it.hasNext())
        {
            if (it.peekNext()->getPeerAddress() == address && it.peekNext()->getPeerPort() == port)
                return future->setResult(it.peekNext()->getId());
            it.next();
        }
        // Creates the socket and connects it virtually to the peer
        QUdpSocket *socket = new QUdpSocket(this);
        socket->connectToHost(address, port);
        // Creates the client
        Client *client = new Client(socket, transport, protocols, socket->localPort(), socket->socketDescriptor(),
                                    address, port, socket->peerName(), LightBird::IClient::CLIENT, this);
        this->clients.push_back(client);
        // When the client thread is finished, _finished is called
        QObject::connect(client, SIGNAL(finished()), this, SLOT(_finished()), Qt::QueuedConnection);
        client->start();
    }
}

bool            Clients::send(const QString &idClient, const QString &idPlugin, const QString &p)
{
    SmartMutex  mutex(this->mutex);
    Client      *client = NULL;
    QString     protocol = p;
    QStringList protocols;

    // Search the client
    QListIterator<Client *> it(this->clients);
    while (it.hasNext() && !client)
        if (it.next()->getId() == idClient)
            client = it.peekPrevious();
    // The client doesn't exists
    if (!client)
        return (false);
    protocols = client->getProtocols();
    // If the protocol is defined we check that it is in the protocols handled by the client
    if (!protocol.isEmpty() && !protocols.contains("all") && !protocols.contains(protocol))
        return (false);
    // Otherwise the protocol is the first in the list
    if (protocol.isEmpty() && !protocols.isEmpty() && !protocols.contains("all"))
        protocol = protocols.first();
    // No protocol has been found
    if (protocol.isEmpty())
        return (false);
    client->send(idPlugin, protocol);
    return (true);
}

bool            Clients::getClient(const QString &id, LightBird::INetwork::Client *client, void *thread, Future<bool> *future)
{
    // Search the client
    QListIterator<Client *> it(this->clients);
    while (it.hasNext())
        if (it.next()->getId() == id)
        {
            // The client has been found
            it.peekPrevious()->getInformations(client, thread, future);
            return (true);
        }
    return (false);
}

QStringList     Clients::getClients()
{
    SmartMutex  mutex(this->mutex);
    QStringList result;

    // Stores the id of the clients in a string list
    QListIterator<Client *> it(this->clients);
    while (it.hasNext())
        result << it.next()->getId();
    return (result);
}

bool            Clients::disconnect(const QString &id)
{
    SmartMutex  mutex(this->mutex);

    // Search the client
    QListIterator<Client *> it(this->clients);
    while (it.hasNext())
        if (it.next()->getId() == id)
        {
            // And quit its event loop
            it.peekPrevious()->quit();
            return (true);
        }
    return (false);
}

bool    Clients::read(QByteArray &data, Client *client)
{
    if (client->getTransport() == LightBird::INetwork::TCP)
    {
        data.clear();
        // Calls the IDoRead interface of the plugins
        if (!client->doRead(data))
            //If no plugins implements it, the server read the data itself
            data = client->getSocket().readAll();
    }
    else
    {
    }
    return (!data.isEmpty());
}

bool    Clients::write(QByteArray &data, Client *client)
{
    int wrote;

    if (client->getTransport() == LightBird::INetwork::TCP)
    {
        // Calls the IDoWrite interface of the plugins
        if (!client->doWrite(data))
        {
            //If no plugins implements it, the server write the data itself
            if ((wrote = client->getSocket().write(data)) != data.size())
            {
                Log::warning("All data has not been written", Properties("wrote", wrote).add("toWrite", data.size()).add("id", client->getId()), "PortTcp", "write");
                return (false);
            }
            client->getSocket().waitForBytesWritten();
        }
    }
    else
    {
    }
    return (true);
}

bool            Clients::connect(Client *client)
{
    SmartMutex  mutex(this->mutex);

    if (client->getMode() == LightBird::IClient::CLIENT &&
        client->getTransport() == LightBird::INetwork::TCP)
    {
        // Gets the future that is waiting for the connection
        QSharedPointer<Future<QString> > future(this->futures.value(client->getId()).first);
        int wait = this->futures.value(client->getId()).second;
        // Connects to the client
        client->getSocket().connectToHost(client->getPeerAddress(), client->getPeerPort());
        // Wait until the connection is established
        if (client->getSocket().waitForConnected(wait))
            future->setResult(client->getId());
        // An error occured
        else
        {
            Log::error("Connection failed", Properties("address", client->getPeerAddress().toString())
                       .add("port", client->getPeerPort())
                       .add("transport", (client->getTransport() == LightBird::INetwork::TCP ? "TCP" : "UDP"))
                       .add("error", client->getSocket().errorString(), false), "Clients", "connect");
            return (false);
        }
    }
    return (true);
}

void                Clients::_disconnected()
{
    SmartMutex      mutex(this->mutex);
    Client          *client = NULL;
    QAbstractSocket *socket;

    // If the sender of the signal is a QAbstractSocket
    if ((socket = qobject_cast<QAbstractSocket *>(this->sender())))
    {
        // Search the client associated with this socket
        QListIterator<Client *> it(this->clients);
        while (it.hasNext() && !client)
            if (&(it.next()->getSocket()) == socket)
                client = it.peekPrevious();
        // Quit the client's thread. Its instance will also be deleted after the thread finished.
        if (client)
            Threads::instance()->deleteThread(client);
    }
}

void            Clients::_finished()
{
    SmartMutex  mutex(this->mutex);
    Client      *client = NULL;

    // Search the client that has been finished
    QListIterator<Client *> it(this->clients);
    while (it.hasNext() && !client)
        if (it.next()->isFinished())
            client = it.peekPrevious();
    // Delete the client
    this->clients.removeAll(client);
    delete client;
}
