#include <QCoreApplication>
#include <QTcpSocket>
#include <QUdpSocket>

#include "Clients.h"
#include "Log.h"
#include "Mutex.h"
#include "Threads.h"

Clients::Clients()
{
    this->moveToThread(this);
    // Starts the thread
    Threads::instance()->newThread(this, false);
    // Waits that the thread is started
    Future<bool>(this->threadStarted).getResult();
}

Clients::~Clients()
{
}

void    Clients::run()
{
    // Connects the TCP clients
    QObject::connect(this, SIGNAL(connectSignal(QString)), this, SLOT(_connect(QString)), Qt::QueuedConnection);
    // Allows to read and write data from this thread
    QObject::connect(this, SIGNAL(readSignal(Client*)), this, SLOT(_read(Client*)), Qt::QueuedConnection);
    QObject::connect(this, SIGNAL(writeSignal()), this, SLOT(_write()), Qt::QueuedConnection);
    this->threadStarted.setResult(true);
    this->exec();
    this->shutdown();
    this->moveToThread(QCoreApplication::instance()->thread());
}

Future<QString> Clients::connect(const QHostAddress &address
    , quint16 port
    , const QStringList &protocols
    , LightBird::INetwork::Transport transport
    , const QVariantMap &informations
    , const QStringList &contexts
    , int wait)
{
    Mutex           mutex(this->mutex, "Clients", "connect");
    Future<QString> *future;

    if (!mutex)
        return (Future<QString>());
    if (transport == LightBird::INetwork::TCP)
    {
        // Creates the socket. The connection is made by the client in the ThreadPool, via IReadWrite
        QTcpSocket *socket = new QTcpSocket(NULL);
        // Creates the client
        Client *client = new Client(socket, protocols, transport, socket->localPort(), socket->socketDescriptor(),
                                    address, port, socket->peerName(), LightBird::IClient::CLIENT, this, contexts);
        client->getInformations() = informations;
        this->clients.push_back(client);
        socket->setParent(client);
        client->moveToThread(this);
        // When the client is finished, _finished is called
        QObject::connect(client, SIGNAL(finished()), this, SLOT(_finished()), Qt::QueuedConnection);
        // When new data are received on this socket, Client::read is called
        QObject::connect(socket, SIGNAL(readyRead()), client, SLOT(readyRead()), Qt::QueuedConnection);
        // When the data have been written on this socket, Client::written is called
        QObject::connect(socket, SIGNAL(bytesWritten(qint64)), client, SLOT(bytesWritten()), Qt::QueuedConnection);
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
        Client *client = new Client(socket, protocols, transport, socket->localPort(), socket->socketDescriptor(),
                                    address, port, socket->peerName(), LightBird::IClient::CLIENT, this, contexts);
        this->clients.push_back(client);
        socket->setParent(client);
        client->moveToThread(this);
        // When new data are received on this socket, Client::read is called
        QObject::connect(socket, SIGNAL(readyRead()), client, SLOT(readyRead()), Qt::QueuedConnection);
        // When the client is finished, _finished is called
        QObject::connect(client, SIGNAL(finished()), this, SLOT(_finished()), Qt::QueuedConnection);
        return (Future<QString>(client->getId()));
    }
    return (Future<QString>());
}

bool    Clients::send(const QString &idClient, const QString &idPlugin, const QString &p, const QVariantMap &informations)
{
    Mutex   mutex(this->mutex, "Clients", "send");
    Client  *client = NULL;
    QString protocol;

    if (!mutex)
        return (false);
    // Searches the client
    QListIterator<Client *> it(this->clients);
    while (it.hasNext() && !client)
        if (it.next()->getId() == idClient)
            client = it.peekPrevious();
    // The client does not exist
    if (!client)
        return (false);
    // Checks the protocol
    if ((protocol = client->getProtocol(p)).isEmpty())
    {
        LOG_WARNING("Invalid protocol", Properties("idClient", idClient).add("idPlugin", idPlugin).add("protocol", p, false), "Clients", "send");
        return (false);
    }
    client->send(protocol, informations, idPlugin);
    return (true);
}

bool    Clients::receive(const QString &id, const QString &p, const QVariantMap &informations)
{
    Mutex   mutex(this->mutex, "Clients", "send");
    Client  *client = NULL;
    QString protocol;

    if (!mutex)
        return (false);
    // Searches the client
    QListIterator<Client *> it(this->clients);
    while (it.hasNext() && !client)
        if (it.next()->getId() == id)
            client = it.peekPrevious();
    // The client does not exist
    if (!client)
        return (false);
    // Checks the protocol
    if ((protocol = client->getProtocol(p)).isEmpty())
    {
        LOG_WARNING("Invalid protocol", Properties("id", id).add("protocol", p, false), "Clients", "receive");
        return (false);
    }
    return (client->receive(protocol, informations));
}

bool    Clients::pause(const QString &idClient, int msec)
{
    Mutex   mutex(this->mutex, "Clients", "pause");

    if (!mutex)
        return (false);
    QListIterator<Client *> it(this->clients);
    while (it.hasNext())
        if (it.next()->getId() == idClient)
            return (it.peekPrevious()->pause(msec));
    return (false);
}

bool    Clients::resume(const QString &idClient)
{
    Mutex   mutex(this->mutex, "Clients", "resume");

    if (!mutex)
        return (false);
    QListIterator<Client *> it(this->clients);
    while (it.hasNext())
        if (it.next()->getId() == idClient)
            return (it.peekPrevious()->resume());
    return (false);
}

Future<bool>    Clients::getClient(const QString &id, LightBird::INetwork::Client &client, bool &found) const
{
    Mutex   mutex(this->mutex, "Clients", "getClient");

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

QStringList Clients::getClients() const
{
    Mutex       mutex(this->mutex, "Clients", "getClients");
    QStringList result;

    if (!mutex)
        return (QStringList());
    // Stores the id of the clients in a string list
    QListIterator<Client *> it(this->clients);
    while (it.hasNext())
        result << it.next()->getId();
    return (result);
}

bool    Clients::disconnect(const QString &id, bool fatal)
{
    Mutex   mutex(this->mutex, "Clients", "disconnect");

    if (!mutex)
        return (false);
    QListIterator<Client *> it(this->clients);
    while (it.hasNext())
        if (it.next()->getId() == id)
        {
            it.peekPrevious()->disconnect(fatal);
            return (true);
        }
    return (false);
}

void    Clients::shutdown()
{
    Mutex   mutex(this->mutex, "Clients", "shutdown");

    if (!mutex)
        return ;
    // Releases the remaining connections requests
    QMapIterator<QString, QPair<Future<QString> *, int> > it(this->connections);
    while (it.hasNext())
        delete it.next().value().first;
    this->connections.clear();
    // Removes the remaining clients
    QListIterator<Client *> client(this->clients);
    while (client.hasNext())
        delete client.next();
    this->clients.clear();
}

bool    Clients::connect(Client *client)
{
    Mutex   mutex(this->mutex, "Clients", "connect");
    QString id;

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
    // The connection to the client is made in the thread
    emit this->connectSignal(id);
    mutex.unlock();
    // Waits the result of the connection
    return (!future.getResult().isEmpty());
}

void    Clients::_connect(QString id)
{
    Mutex   mutex(this->mutex, "Clients", "_connect");
    Client  *client = NULL;

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
            LOG_ERROR("Connection failed", Properties("address", client->getPeerAddress().toString())
                      .add("port", client->getPeerPort())
                      .add("transport", (client->getTransport() == LightBird::INetwork::TCP ? "TCP" : "UDP"))
                      .add("error", client->getSocket().errorString(), false), "Clients", "connect");
    }
}

void    Clients::read(Client *client)
{
    emit this->readSignal(client);
}

void    Clients::_read(Client *client)
{
    QByteArray  &data = client->getData();
    int         read;

    data.clear();
    // Calls the IDoRead interface of the plugins
    if (!client->doRead(data))
    {
        // If no plugin implements it, the server read the data itself
        data.resize(client->getSocket().size());
        if ((read = client->getSocket().read(data.data(), data.size())) != data.size())
        {
            LOG_WARNING("An error occured while reading the data", Properties("id", client->getId())
                        .add("error", read).add("size", data.size()), "Clients", "_read");
            data.resize(read);
        }
    }
    client->bytesRead();
}

bool    Clients::write(QByteArray *data, Client *client)
{
    Mutex   mutex(this->mutex, "Clients", "write");

    if (!mutex)
        return (false);
    // Creates the write buffer
    QSharedPointer<WriteBuffer> writeBuffer(new WriteBuffer(client, data, this->thread()));
    QObject::connect(writeBuffer.data(), SIGNAL(bytesWritten()), this, SLOT(_write()), Qt::QueuedConnection);
    this->writeBuffer.insert(client, writeBuffer);
    // Writes the data via _write()
    emit writeSignal();
    return (true);
}

void    Clients::_write()
{
    Mutex   mutex(this->mutex, "Clients", "_write");
    Client  *client;
    qint64  result;
    QHash<Client *, QSharedPointer<WriteBuffer> > writeBuffer;

    if (!mutex || this->writeBuffer.isEmpty())
        return ;
    // We make a copy of the writeBuffer in order to write the data outside of the mutex.
    // This is important because the API (IDoWrite) must never be called in a mutex.
    // This is fine because Port::clients is only modified in this thread.
    writeBuffer = this->writeBuffer;
    this->writeBuffer.clear();
    mutex.unlock();

    QMutableHashIterator<Client *, QSharedPointer<WriteBuffer> > it(writeBuffer);
    while (it.hasNext())
    {
        WriteBuffer &w = *it.next().value();
        client = it.key();
        result = -1;
        // Ensures that we are still connected to the client
        if (!w.isWritten() && client->getSocket().state() == QAbstractSocket::ConnectedState)
        {
            // The WriteBuffer is ready to write more data
            if (!w.isWriting())
            {
                // Tries to call the IDoWrite interface of the plugins
                if (!client->doWrite(w.getData(), w.getSize(), result))
                    // If no plugins implements IDoWrite, we write the data ourselves
                    result = client->getSocket().write(w.getData(), w.getSize());
                w.bytesWriting(result);
                if (result < 0)
                    LOG_WARNING("An error occured while writing the data", Properties("return", result).add("size", w.getTotalSize()).add("id", client->getId()), "Clients", "_write");
                if (result == 0)
                    LOG_ERROR("Write returned 0", Properties("size", w.getTotalSize()).add("id", client->getId()), "Clients", "_write");
            }
            else
                result = 0;
        }
        if (result < 0)
            it.remove();
    }

    mutex.lock();
    // Finally we put the data that have not been sent in the writeBuffer,
    // which may have been filled by other threads in the meantime.
    if (!this->writeBuffer.isEmpty())
        emit writeSignal();
    this->writeBuffer.unite(writeBuffer);
}

void    Clients::_disconnected()
{
    Mutex           mutex(this->mutex, "Clients", "_disconnected");
    QAbstractSocket *socket;

    if (!mutex)
        return ;
    // If the sender of the signal is a QAbstractSocket
    if ((socket = qobject_cast<QAbstractSocket *>(this->sender())))
    {
        // Searches the client associated with this socket
        QListIterator<Client *> it(this->clients);
        while (it.hasNext())
            // And disconnect it
            if (&(it.next()->getSocket()) == socket)
            {
                it.peekPrevious()->disconnect();
                // Removes the client from the write buffer since we won't be able to write to him anymore
                this->writeBuffer.remove(it.peekPrevious());
                break;
            }
    }
}

void        Clients::_finished()
{
    Mutex   mutex(this->mutex, "Clients", "_finished");
    Client  *client;

    if (!mutex)
        return ;
    // Deletes the clients finished
    QMutableListIterator<Client *> it(this->clients);
    while (it.hasNext())
        // The client is deleted only if there is no remaining data in the writeBuffer
        if ((client = it.next())->isFinished() && !this->writeBuffer.contains(client))
        {
            it.remove();
            delete client;
        }
}
