#include <QCoreApplication>

#include "Clients.h"
#include "Log.h"
#include "Mutex.h"
#include "SocketTcp.h"
#include "SocketUdp.h"
#include "Threads.h"
#include "Server.h"

Clients::Clients()
    : _network(NULL)
{
    moveToThread(this);
    // Starts the thread
    Threads::instance()->newThread(this, false);
    // Waits that the thread is started
    isInitialized(Future<bool>(_threadStarted).getResult());
}

Clients::~Clients()
{
    Mutex mutex(_mutex, "Clients", "~Clients");

    if (!mutex)
        return ;
    if (_network)
        _network->close();
    mutex.unlock();
    // Quit the thread if it is still running
    quit();
    wait();
    delete _network;
    LOG_TRACE("Clients destroyed!", "Clients", "~Clients");
}

void    Clients::run()
{
    _network = ClientsNetwork::create();
    _threadStarted.setResult(_network != NULL);
    // This method only returns when the clients is shutdown
    if (_network)
        _network->execute();
    Mutex mutex(_mutex, "Clients", "run");
    // If some clients are still running, we wait for them
    if (_clients.size() && Server::isRunning())
        _threadFinished.wait(&_mutex);
    // Remove the remaining clients
    _clients.clear();
    _writeBuffers.clear();
    // Releases the remaining connections requests
    QMapIterator<QString, Future<QString> *> it(_connections);
    while (it.hasNext())
        delete it.next().value();
    _connections.clear();
    moveToThread(QCoreApplication::instance()->thread());
}

Future<QString> Clients::connect(const QHostAddress &address
    , quint16 port
    , const QStringList &protocols
    , LightBird::INetwork::Transport transport
    , const QVariantMap &informations
    , const QStringList &contexts
    , int wait)
{
    Mutex           mutex(_mutex, "Clients", "connect");
    Future<QString> *future;

    if (!mutex)
        return (Future<QString>());
    if (transport == LightBird::INetwork::TCP)
    {
        // Creates the socket
        QSharedPointer<Socket> socket(SocketTcp::create(address, port));
        if (!socket->isConnected() && !(static_cast<SocketTcp *>(socket.data()))->isConnecting())
            return (Future<QString>());
        // Creates the client
        Client *client = new Client(socket, protocols, transport, LightBird::IClient::CLIENT, this, contexts);
        client->getInformations() = informations;
        _clients.push_back(QSharedPointer<Client>(client));
        // When the socket is connected, _connected is called
        QObject::connect(socket.data(), SIGNAL(connected(Socket*,bool)), this, SLOT(_connected(Socket*,bool)), Qt::DirectConnection);
        // When new data are received on this socket, Client::readyRead is called
        QObject::connect(socket.data(), SIGNAL(readyRead()), client, SLOT(readyRead()), Qt::DirectConnection);
        // When the socket is disconnected, _disconnected is called
        QObject::connect(socket.data(), SIGNAL(disconnected(Socket*)), this, SLOT(_disconnected(Socket*)), Qt::DirectConnection);
        // When the client is finished, _finished is called
        QObject::connect(client, SIGNAL(finished(Client*)), this, SLOT(_finished(Client*)), Qt::DirectConnection);
        // Keeps the future in order to set its result in IReadWrite::connect
        future = new Future<QString>();
        _connections.insert(client->getId(), future);
        _network->addSocket(socket, wait);
        return (Future<QString>(*future));
    }
    else
    {
        // Ensures that the client is not already connected
        for (QListIterator<QSharedPointer<Client> > it(_clients); it.hasNext(); it.next())
            if (it.peekNext()->getPeerAddress() == address && it.peekNext()->getPeerPort() == port)
                return (Future<QString>(it.peekNext()->getId()));
        // Creates the socket
        QSharedPointer<Socket> socket(SocketUdp::create(address, port));
        if (!socket->isConnected())
            return (Future<QString>());
        // Creates the client
        Client *client = new Client(socket, protocols, transport, LightBird::IClient::CLIENT, this, contexts);
        client->getInformations() = informations;
        _clients.push_back(QSharedPointer<Client>(client));
        // When new data are received on this socket, Client::readyRead is called
        QObject::connect(socket.data(), SIGNAL(readyRead()), client, SLOT(readyRead()), Qt::DirectConnection);
        // When the socket is disconnected, _disconnected is called
        QObject::connect(socket.data(), SIGNAL(disconnected(Socket*)), this, SLOT(_disconnected(Socket*)), Qt::DirectConnection);
        // When the client is finished, _finished is called
        QObject::connect(client, SIGNAL(finished(Client*)), this, SLOT(_finished(Client*)), Qt::DirectConnection);
        // The client is now connected
        _network->addSocket(socket);
        client->connected(true);
        return (Future<QString>(client->getId()));
    }
    return (Future<QString>());
}

bool    Clients::send(const QString &id, const QString &idPlugin, const QString &p, const QVariantMap &informations)
{
    Mutex   mutex(_mutex, Mutex::READ, "Clients", "send");
    Client  *client = NULL;
    QString protocol;

    if (!mutex)
        return (false);
    // Searches the client
    for (QListIterator<QSharedPointer<Client> > it(_clients); it.hasNext(); it.next())
        if (it.peekNext()->getId() == id)
        {
            client = it.peekNext().data();
            break;
        }
    // The client does not exist
    if (!client)
        return (false);
    // Checks the protocol
    if ((protocol = client->getProtocol(p)).isEmpty())
    {
        LOG_WARNING("Invalid protocol", Properties("idClient", id).add("idPlugin", idPlugin).add("protocol", p, false), "Clients", "send");
        return (false);
    }
    client->send(protocol, informations, idPlugin);
    return (true);
}

bool    Clients::receive(const QString &id, const QString &p, const QVariantMap &informations)
{
    Mutex   mutex(_mutex, Mutex::READ, "Clients", "send");
    Client  *client = NULL;
    QString protocol;

    if (!mutex)
        return (false);
    // Searches the client
    for (QListIterator<QSharedPointer<Client> > it(_clients); it.hasNext(); it.next())
        if (it.peekNext()->getId() == id)
        {
            client = it.peekNext().data();
            break;
        }
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

bool    Clients::pause(const QString &id, int msec)
{
    Mutex   mutex(_mutex, Mutex::READ, "Clients", "pause");

    if (!mutex)
        return (false);
    for (QListIterator<QSharedPointer<Client> > it(_clients); it.hasNext(); it.next())
        if (it.peekNext()->getId() == id)
            return (it.peekNext()->pause(msec));
    return (false);
}

bool    Clients::resume(const QString &id)
{
    Mutex   mutex(_mutex, Mutex::READ, "Clients", "resume");

    if (!mutex)
        return (false);
    for (QListIterator<QSharedPointer<Client> > it(_clients); it.hasNext(); it.next())
        if (it.peekNext()->getId() == id)
            return (it.peekNext()->resume());
    return (false);
}

Future<bool>    Clients::getClient(const QString &id, LightBird::INetwork::Client &client, bool &found) const
{
    Mutex   mutex(_mutex, Mutex::READ, "Clients", "getClient");

    found = false;
    if (!mutex)
        return (Future<bool>(false));
    for (QListIterator<QSharedPointer<Client> > it(_clients); it.hasNext(); it.next())
        if (it.peekNext()->getId() == id)
        {
            Future<bool> *future = new Future<bool>(false);
            Future<bool> result(*future);
            it.peekNext()->getInformations(client, future);
            found = true;
            return (result);
        }
    return (Future<bool>(false));
}

QStringList Clients::getClients() const
{
    Mutex       mutex(_mutex, Mutex::READ, "Clients", "getClients");
    QStringList result;

    if (!mutex)
        return (QStringList());
    // Stores the id of the clients in a string list
    for (QListIterator<QSharedPointer<Client> > it(_clients); it.hasNext(); it.next())
        result << it.peekNext()->getId();
    return (result);
}

bool    Clients::disconnect(const QString &id, bool fatal)
{
    Mutex   mutex(_mutex, Mutex::READ, "Clients", "disconnect");

    if (!mutex)
        return (false);
    for (QListIterator<QSharedPointer<Client> > it(_clients); it.hasNext(); it.next())
        if (it.peekNext()->getId() == id)
        {
            it.peekNext()->disconnect(fatal);
            return (true);
        }
    return (false);
}

bool    Clients::setDisconnectIdle(const QString &id, qint64 msec, bool fatal)
{
    Mutex   mutex(_mutex, Mutex::READ, "Clients", "setDisconnectIdle");

    if (!mutex)
        return (false);
    for (QListIterator<QSharedPointer<Client> > it(_clients); it.hasNext(); it.next())
        if (it.peekNext()->getId() == id)
        {
            it.peekNext()->setDisconnectIdle(msec, fatal);
            return (true);
        }
    return (false);
}

bool    Clients::getDisconnectIdle(const QString &id, bool *fatal, qint64 &result)
{
    Mutex   mutex(_mutex, Mutex::READ, "Clients", "getDisconnectIdle");

    if (!mutex)
        return (false);
    for (QListIterator<QSharedPointer<Client> > it(_clients); it.hasNext(); it.next())
        if (it.peekNext()->getId() == id)
        {
            result = it.peekNext()->getDisconnectIdle(fatal);
            return (true);
        }
    return (false);
}

bool    Clients::setDisconnectTime(const QString &id, const QDateTime &time, bool fatal)
{
    Mutex   mutex(_mutex, Mutex::READ, "Clients", "setDisconnectTime");

    if (!mutex)
        return (false);
    for (QListIterator<QSharedPointer<Client> > it(_clients); it.hasNext(); it.next())
        if (it.peekNext()->getId() == id)
        {
            it.peekNext()->setDisconnectTime(time, fatal);
            return (true);
        }
    return (false);
}

bool    Clients::getDisconnectTime(const QString &id, bool *fatal, QDateTime &result)
{
    Mutex   mutex(_mutex, Mutex::READ, "Clients", "getDisconnectTime");

    if (!mutex)
        return (false);
    for (QListIterator<QSharedPointer<Client> > it(_clients); it.hasNext(); it.next())
        if (it.peekNext()->getId() == id)
        {
            result = it.peekNext()->getDisconnectTime(fatal);
            return (true);
        }
    return (false);
}

void    Clients::close()
{
    Mutex mutex(_mutex, "Clients", "close");

    if (!mutex)
        return ;
    // Stop listening on the network
    if (_network)
        _network->close();
    // Disconnects all the clients in the map
    if (_clients.size() > 0)
        for (QListIterator<QSharedPointer<Client> > it(_clients); it.hasNext(); it.next())
            it.peekNext()->disconnect();
    // Or quit the thread if there is already no remaining clients
    else
        _threadFinished.wakeAll();
}

void    Clients::_connected(Socket *socket, bool success)
{
    Mutex   mutex(_mutex, "Clients", "_connected");

    if (!mutex)
        return ;
    // Searches the client associated with this socket
    for (QListIterator<QSharedPointer<Client> > it(_clients); it.hasNext(); it.next())
        if (&it.peekNext()->getSocket() == socket)
        {
            Client *client = it.peekNext().data();
            client->connected(success);
            // Unlocks the future that is waiting for the connection
            if (!_connections.contains(client->getId()))
                return ;
            QSharedPointer<Future<QString> > future(_connections.value(client->getId()));
            _connections.remove(client->getId());
            if (success)
                future->setResult(client->getId());
            else
                LOG_ERROR("Connection failed", Properties("address", client->getPeerAddress().toString()).add("port", client->getPeerPort()).add("socket", client->getSocket().descriptor()).add("transport", "TCP"), "Clients", "_connected");
            return ;
        }
}

void    Clients::read(Client *client)
{
    QByteArray  &data = client->getData();
    qint64 read;

    data.clear();
    // Calls the IDoRead interface of the plugins
    if (!client->doRead(data))
    {
        // If no plugin implements it, the server read the data itself
        data.resize(client->getSocket().size());
        if ((read = client->getSocket().read(data.data(), data.size())) != data.size())
            data.resize(qMax(read, qint64(0)));
    }
    client->bytesRead();
}

void Clients::write(Client *client, const QByteArray &data)
{
    qint64 result;
    qint64 written = 0;
    bool doWrite = true;

    if (client->getSocket().isConnected() && data.size())
    {
        while (written < data.size())
        {
            // Tries to call the IDoWrite interface of the plugins
            if (!doWrite || !(doWrite = client->doWrite(data.data() + written, data.size() - written, result)))
                // If no plugins implements IDoWrite, we write the data ourselves
                result = client->getSocket().write(data.data() + written, data.size() - written);
            if (result > 0)
                written += result;
            // The socket is not ready to write more data, so we wait until readyWrite is emited
            else if (result == 0)
            {
                Mutex mutex(_mutex, "Clients", "write");
                if (!mutex)
                    return ;
                QSharedPointer<WriteBuffer> writeBuffer(new WriteBuffer(client, data, written));
                _writeBuffers.insert(_getClient(client), writeBuffer);
                QObject::connect(&client->getSocket(), SIGNAL(readyWrite()), this, SLOT(_write()), Qt::DirectConnection);
                mutex.unlock();
                _write();
                return ;
            }
            else if (result < 0)
            {
                LOG_WARNING("An error occurred while writing the data", Properties("id", client->getId()).add("error", result).add("written", written).add("size", data.size() - written), "Clients", "write");
                break;
            }
        }
    }
    client->bytesWritten();
    return ;
}

void Clients::_write()
{
    Client *client;
    qint64 result;
    QHash<QSharedPointer<Client>, QSharedPointer<WriteBuffer> > writeBuffer;
    Mutex mutex(_mutex, "Clients", "_write");

    if (!mutex || _writeBuffers.isEmpty())
        return ;
    // We make a copy of the writeBuffer in order to write the data outside of the mutex.
    // This is important because the API (IDoWrite) must never be called in a mutex.
    writeBuffer = _writeBuffers;
    _writeBuffers.clear();
    mutex.unlock();

    QMutableHashIterator<QSharedPointer<Client>, QSharedPointer<WriteBuffer> > it(writeBuffer);
    while (it.hasNext())
    {
        WriteBuffer &w = *it.next().value();
        client = it.key().data();
        result = -1;
        if (client->getSocket().isConnected())
        {
            while (!w.isWritten())
            {
                // Tries to call the IDoWrite interface of the plugins
                if (!client->doWrite(w.getData(), w.getSize(), result))
                    // If no plugins implements IDoWrite, we write the data ourselves
                    result = client->getSocket().write(w.getData(), w.getSize());
                if (result > 0)
                    w.bytesWritten(result);
                else
                    break;
            }
            if (w.isWritten() || result < 0)
            {
                if (result < 0)
                    LOG_WARNING("An error occurred while writing the data", Properties("return", result).add("size", w.getTotalSize()).add("written", w.bytesWritten()).add("id", client->getId()), "Clients", "_write");
                QObject::disconnect(&client->getSocket(), SIGNAL(readyWrite()));
                it.remove();
                client->bytesWritten();
            }
        }
        else
            it.remove();
    }

    mutex.lock();
    // Finally we put the data that have not been sent in the writeBuffer,
    // which may have been filled by other threads in the meantime.
    _writeBuffers.unite(writeBuffer);
}

void    Clients::_disconnected(Socket *socket)
{
    Mutex   mutex(_mutex, "Clients", "_disconnected");

    if (!mutex)
        return ;
    // Searches the client associated with this socket
    for (QListIterator<QSharedPointer<Client> > it(_clients); it.hasNext(); it.next())
        if (&it.peekNext()->getSocket() == socket)
        {
            // Removes the client of the disconnected socket
            it.peekNext()->disconnect();
            // Removes the client from the write buffer since we won't be able to write to him anymore
            if (_writeBuffers.remove(it.peekNext()))
                it.peekNext()->bytesWritten();
            break;
        }
}

void        Clients::_finished(Client *client)
{
    Mutex mutex(_mutex, "Clients", "_finished");

    if (!mutex)
        return ;
    // Searches the clients that have been finished
    QMutableListIterator<QSharedPointer<Client> > it(_clients);
    while (it.hasNext())
        if (it.next().data() == client)
        {
            it.remove();
            if (_clients.size() == 0 && !_network->isListening())
                _threadFinished.wakeAll();
            break;
        }
}

QSharedPointer<Client> Clients::_getClient(Client *client)
{
    for (QListIterator<QSharedPointer<Client> > it(_clients); it.hasNext(); it.next())
        if (it.peekNext() == client)
            return it.peekNext();
    return QSharedPointer<Client>(NULL);
}
