#include <QCoreApplication>
#include <QTcpSocket>
#include <QUdpSocket>

#include "Clients.h"
#include "Log.h"
#include "SmartMutex.h"
#include "Threads.h"

Clients::Clients()
{
    this->moveToThread(this);
    // Starts the thread
    this->start();
    // Waits that the thread is started
    Future<bool>(this->threadStarted).getResult();
}

Clients::~Clients()
{
    // Quit the thread if it is still running
    this->quit();
    this->wait();
    // Release the remaining connections requests
    QMapIterator<QString, QPair<Future<QString> *, int> > it(this->connections);
    while (it.hasNext())
        delete it.next().value().first;
    // Clean the write buffer
    while (!this->writeBuffer.isEmpty())
        delete this->writeBuffer.dequeue().second;
}

void    Clients::run()
{
    // Connects the TCP clients
    QObject::connect(this, SIGNAL(connectSignal(QString)), this, SLOT(_connect(QString)), Qt::QueuedConnection);
    // Allows to write the data from this thread
    QObject::connect(this, SIGNAL(writeSignal()), this, SLOT(_write()), Qt::QueuedConnection);
    this->threadStarted.setResult(true);
    this->exec();
    this->moveToThread(QCoreApplication::instance()->thread());
}

Future<QString> Clients::connect(const QHostAddress &address, quint16 port, const QStringList &protocols,
                                 LightBird::INetwork::Transport transport, int wait)
{
    SmartMutex      mutex(this->mutex);
    Future<QString> *future;

    if (!mutex)
        return (Future<QString>());
    if (transport == LightBird::INetwork::TCP)
    {
        // Creates the socket. The connection is made by the client in the ThreadPool, via IReadWrite
        QTcpSocket *socket = new QTcpSocket(NULL);
        // Creates the client
        Client *client = new Client(socket, transport, protocols, socket->localPort(), socket->socketDescriptor(),
                                    address, port, socket->peerName(), LightBird::IClient::CLIENT, this);
        this->clients.push_back(client);
        socket->setParent(client);
        client->moveToThread(this);
        // When the client thread is finished, _finished is called
        QObject::connect(client, SIGNAL(finished()), this, SLOT(_finished()), Qt::QueuedConnection);
        // When new data are received on this socket, Client::read is called
        QObject::connect(socket, SIGNAL(readyRead()), client, SLOT(read()), Qt::QueuedConnection);
        // When the client is disconnected, _disconnected is called
        QObject::connect(socket, SIGNAL(disconnected()), this, SLOT(_disconnected()), Qt::QueuedConnection);
        // Keeps the future in order to set its result in IReadWrite::connect
        future = new Future<QString>();
        this->connections.insert(client->getId(), QPair<Future<QString> *, int>(future, wait));
        return (Future<QString>(*future));
    }
    else
    {
        // Ensures that the client is not already connected
        QListIterator<Client *> it(this->clients);
        while (it.hasNext())
        {
            if (it.peekNext()->getPeerAddress() == address && it.peekNext()->getPeerPort() == port)
                return (Future<QString>(it.peekNext()->getId()));
            it.next();
        }
        // Creates the socket and connects it virtually to the peer
        QUdpSocket *socket = new QUdpSocket(NULL);
        socket->connectToHost(address, port);
        // Creates the client
        Client *client = new Client(socket, transport, protocols, socket->localPort(), socket->socketDescriptor(),
                                    address, port, socket->peerName(), LightBird::IClient::CLIENT, this);
        this->clients.push_back(client);
        socket->setParent(client);
        client->moveToThread(this);
        // When new data are received on this socket, Client::read is called
        QObject::connect(socket, SIGNAL(readyRead()), client, SLOT(read()), Qt::QueuedConnection);
        // When the client thread is finished, _finished is called
        QObject::connect(client, SIGNAL(finished()), this, SLOT(_finished()), Qt::QueuedConnection);
        return (Future<QString>(client->getId()));
    }
    return (Future<QString>());
}

bool            Clients::send(const QString &idClient, const QString &idPlugin, const QString &p)
{
    SmartMutex  mutex(this->mutex, "Clients", "send");
    Client      *client = NULL;
    QString     protocol = p;
    QStringList protocols;

    if (!mutex)
        return (false);
    // Search the client
    QListIterator<Client *> it(this->clients);
    while (it.hasNext() && !client)
        if (it.next()->getId() == idClient)
            client = it.peekPrevious();
    // The client doesn't exists
    if (!client)
    {
        Log::warning("Unknow client id", Properties("idClient", idClient).add("idPlugin", idPlugin), "Clients", "send");
        return (false);
    }
    protocols = client->getProtocols();
    // If the protocol is defined we check that it is in the protocols handled by the client
    if (!protocol.isEmpty() && !protocols.contains("all") && !protocols.contains(protocol))
    {
        Log::warning("The protocol is not handled by the client", Properties("idClient", idClient).add("idPlugin", idPlugin).add("protocol", protocol), "Clients", "send");
        return (false);
    }
    // Otherwise the protocol is the first in the list
    if (protocol.isEmpty() && !protocols.isEmpty() && !protocols.contains("all"))
        protocol = protocols.first();
    // No protocol has been found
    if (protocol.isEmpty())
    {
        Log::warning("No protocol defined for the request", Properties("idClient", idClient).add("idPlugin", idPlugin), "Clients", "send");
        return (false);
    }
    client->send(idPlugin, protocol);
    return (true);
}

Future<bool>    Clients::getClient(const QString &id, LightBird::INetwork::Client &client, bool &found)
{
    SmartMutex  mutex(this->mutex, "Clients", "getClient");

    found = false;
    if (!mutex)
        return (Future<bool>(false));
    QListIterator<Client *> it(this->clients);
    while (it.hasNext())
        if (it.next()->getId() == id)
        {
            Future<bool> *future = new Future<bool>(false);
            Future<bool> result(*future);
            it.peekPrevious()->getInformations(client, future);
            found = true;
            return (result);
        }
    return (Future<bool>(false));
}

QStringList     Clients::getClients()
{
    SmartMutex  mutex(this->mutex, "Clients", "getClients");
    QStringList result;

    if (!mutex)
        return (QStringList());
    // Stores the id of the clients in a string list
    QListIterator<Client *> it(this->clients);
    while (it.hasNext())
        result << it.next()->getId();
    return (result);
}

bool            Clients::disconnect(const QString &id)
{
    SmartMutex  mutex(this->mutex, "Clients", "disconnect");

    if (!mutex)
        return (false);
    QListIterator<Client *> it(this->clients);
    while (it.hasNext())
        if (it.next()->getId() == id)
        {
            it.peekPrevious()->disconnect();
            return (true);
        }
    return (false);
}

bool    Clients::read(QByteArray &data, Client *client)
{
    int read;

    data.clear();
    // Calls the IDoRead interface of the plugins
    if (!client->doRead(data))
    {
        // If no plugins implements it, the server read the data itself
        data.resize(client->getSocket().size());
        if ((read = client->getSocket().read(data.data(), data.size())) != data.size())
        {
            Log::warning("An error occured while reading the data", Properties("id", client->getId())
                         .add("error", read).add("size", data.size()), "Clients", "read");
            data.resize(read);
        }
    }
    return (!data.isEmpty());
}

bool            Clients::write(QByteArray *data, Client *client)
{
    SmartMutex  mutex(this->mutex, "Clients", "write");

    if (!mutex)
        return (false);
    // If a signal has not already be send
    if (this->writeBuffer.isEmpty())
        emit writeSignal();
    this->writeBuffer.enqueue(QPair<Client *, QByteArray *>(client, data));
    return (true);
}

void            Clients::_write()
{
    SmartMutex  mutex(this->mutex, "Clients", "_write");
    Client      *client;
    QByteArray  *data;
    int         wrote;

    if (!mutex)
        return ;
    // While there is data to send
    while (!this->writeBuffer.isEmpty())
    {
        client = this->writeBuffer.head().first;
        data = this->writeBuffer.head().second;
        if (this->clients.contains(client))
        {
            // The API should never be called in a mutex
            mutex.unlock();
            // Calls the IDoWrite interface of the plugins, if the client still exists and is connected
            if(client->getSocket().state() == QAbstractSocket::ConnectedState && !client->doWrite(*data))
                //If no plugins implements IDoWrite, the server write the data itself
                if ((wrote = client->getSocket().write(*data)) != data->size())
                    Log::warning("All data has not been written", Properties("wrote", wrote)
                                 .add("size", data->size()).add("id", client->getId()), "Clients", "write");
            mutex.lock();
        }
        delete data;
        this->writeBuffer.dequeue();
    }
}

bool            Clients::connect(Client *client)
{
    SmartMutex  mutex(this->mutex, "Clients", "connect");
    QString     id;

    if (!mutex)
        return (false);
    // No connection is needed in UDP
    if (client->getTransport() == LightBird::INetwork::UDP)
        return (true);
    id = client->getId();
    // Checks if the client exists
    if (!this->connections.contains(id))
        return (false);
    // Gets the future that will be unlocked when the connection is etablished
    Future<QString> future(*this->connections.value(id).first);
    mutex.unlock();
    // The connection to the client is made in the thread
    emit this->connectSignal(id);
    // Waits the result of the connection
    return (!future.getResult().isEmpty());
}

void            Clients::_connect(QString id)
{
    SmartMutex  mutex(this->mutex, "Clients", "_connect");
    Client      *client = NULL;

    if (!mutex)
        return ;
    // Gets the client from its id
    QListIterator<Client *> it(this->clients);
    while (it.hasNext() && !client)
        if (it.next()->getId() == id)
            client = it.peekPrevious();
    // Gets the future that is waiting for the connection
    QSharedPointer<Future<QString> > future(this->connections.value(id).first);
    int wait = this->connections.value(id).second;
    this->connections.remove(id);
    if (!client)
        return Log::error("Client not found", Properties("id", id), "Clients", "_connect");
    if (client->getTransport() == LightBird::INetwork::TCP)
    {
        // The connection is done outside the mutex
        mutex.unlock();
        // Connects to the client
        client->getSocket().connectToHost(client->getPeerAddress(), client->getPeerPort());
        // Waits until the connection is established
        if (client->getSocket().waitForConnected(wait))
        {
            client->setPort(client->getSocket().localPort());
            future->setResult(id);
        }
        // An error occured
        else
            Log::error("Connection failed", Properties("address", client->getPeerAddress().toString())
                       .add("port", client->getPeerPort())
                       .add("transport", (client->getTransport() == LightBird::INetwork::TCP ? "TCP" : "UDP"))
                       .add("error", client->getSocket().errorString(), false), "Clients", "connect");
    }
}

void                Clients::_disconnected()
{
    SmartMutex      mutex(this->mutex);
    Client          *client = NULL;
    QAbstractSocket *socket;

    if (!mutex)
        return ;
    // If the sender of the signal is a QAbstractSocket
    if ((socket = qobject_cast<QAbstractSocket *>(this->sender())))
    {
        // Search the client associated with this socket
        QListIterator<Client *> it(this->clients);
        while (it.hasNext() && !client)
            // And disconnect it
            if (&(it.next()->getSocket()) == socket)
                it.peekPrevious()->disconnect();
    }
}

void            Clients::_finished()
{
    SmartMutex  mutex(this->mutex, "Clients", "_finished");
    Client      *client;

    if (!mutex)
        return ;
    // Delete the clients finished
    QMutableListIterator<Client *> it(this->clients);
    while (it.hasNext())
        if ((client = it.next())->isFinished())
        {
            it.remove();
            delete client;
        }
}
