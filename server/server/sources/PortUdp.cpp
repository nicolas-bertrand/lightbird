#include <QCoreApplication>

#include "Log.h"
#include "PortUdp.h"
#include "Mutex.h"
#include "Threads.h"
#include "Server.h"

PortUdp::PortUdp(unsigned short port, const QStringList &protocols, const QStringList &contexts, unsigned int maxClients)
    : Port(port, LightBird::INetwork::UDP, protocols, contexts, maxClients)
    , _serverUdp(NULL)
    , _maxReadBufferSize(1000)
{
    moveToThread(this);
    // Starts the thread
    Threads::instance()->newThread(this, false);
    // Waits that the thread is started
    Future<bool>(_threadStarted).getResult();
}

PortUdp::~PortUdp()
{
    Mutex mutex(_mutex, "PortUdp", "~PortUdp");

    if (!mutex)
        return ;
    if (_serverUdp)
        _serverUdp->close();
    mutex.unlock();
    // Quit the thread if it is still running
    quit();
    wait();
    delete _serverUdp;
    LOG_TRACE("Port UDP destroyed!", Properties("port", _port), "PortUdp", "~PortUdp");
}

void    PortUdp::run()
{
    // Creates the UDP server
    _serverUdp = ServerUdp::create(_port);
    // Listen on the given port
    if (!_serverUdp->isListening())
    {
        LOG_ERROR("Failed to listen on the port", Properties("port", _port).add("protocols", _protocols.join(" ")).add("transport", "UDP").add("maxClients", _maxClients), "PortUdp", "run");
        _threadStarted.setResult(false);
        moveToThread(QCoreApplication::instance()->thread());
        return ;
    }
    // When a client connects to the server, the slot _newConnection is called
    QObject::connect(_serverUdp, SIGNAL(readyRead()), this, SLOT(_readPendingDatagrams()), Qt::DirectConnection);
    LOG_INFO("Listening...", Properties("port", _port).add("protocols", _protocols.join(" ")).add("transport", "UDP").add("maxClients", _maxClients), "PortUdp", "run");
    _threadStarted.setResult(true);
    // This method only returns when the port is closed
    _serverUdp->execute();
    Mutex mutex(_mutex, "PortUdp", "run");
    // If some clients are still running, we wait for them
    if (_clients.size() && Server::isRunning())
        _threadFinished.wait(&_mutex);
    // Remove the remaining clients
    _clients.clear();
    _writeBuffers.clear();
    // Removes the unread datagrams
    QHashIterator<Client *, QByteArray *> readBuffer(_readBuffer);
    while (readBuffer.hasNext())
        delete readBuffer.next().value();
    _readBuffer.clear();
    moveToThread(QCoreApplication::instance()->thread());
    LOG_INFO("Port closed", Properties("port", _port), "PortUdp", "run");
}

void    PortUdp::close()
{
    Mutex mutex(_mutex, "PortUdp", "close");

    if (!mutex)
        return ;
    // Stop listening on the network
    if (_serverUdp)
        _serverUdp->close();
    // Disconnects all the clients in the map
    if (_clients.size() > 0)
        for (QListIterator<QSharedPointer<Client> > it(_clients); it.hasNext(); it.next())
            it.peekNext()->disconnect();
    // Or quit the thread if there is already no remaining clients
    else
        _threadFinished.wakeAll();
}

void    PortUdp::_readPendingDatagrams()
{
    Mutex mutex(_mutex, "PortUdp", "_readPendingDatagrams");
    Client *client;
    QHostAddress peerAddress;
    quint16 peerPort;
    qint64 size;

    if (!mutex)
        return ;
    // While there are pending datagrams, we read them
    while ((size = _serverUdp->hasPendingDatagrams()))
    {
        // Reads the next datagram
        QByteArray *data = new QByteArray();
        data->resize(size);
        _serverUdp->readDatagram(data->data(), data->size(), peerAddress, peerPort);
        client = NULL;
        // Checks if the client is already connected
        for (QListIterator<QSharedPointer<Client> > it(_clients); it.hasNext(); it.next())
            if (it.peekNext()->getPeerAddress() == peerAddress && it.peekNext()->getPeerPort() == peerPort)
            {
                client = it.peekNext().data();
                break;
            }
        if ((client || (uint)_clients.size() < _maxClients) && (uint)_readBuffer.size() < _maxReadBufferSize)
        {
            // If the client does not exist yet it is created
            if (client == NULL)
            {
                // Creates the client
                client = new Client(_serverUdp->getListenSocket(), _protocols, _transport, _contexts, LightBird::IClient::SERVER, this);
                // Adds the client
                _clients.push_back(QSharedPointer<Client>(client));
                // When the client is finished, _finished is called
                QObject::connect(client, SIGNAL(finished(Client*)), this, SLOT(_finished(Client*)), Qt::DirectConnection);
                client->connected(true);
            }
            // Notifies the Client that data are ready to be read
            _readBuffer.insert(client, data);
            client->readyRead();
        }
        // There is already too many clients connected
        else
            delete data;
    }
}

void    PortUdp::read(Client *client)
{
    Mutex       mutex(_mutex, "PortUdp", "read");
    QByteArray  &data = client->getData();
    quint64     size = 0;

    if (!mutex)
        return ;
    data.clear();
    // Gets the total size of the data buffers of this client
    QListIterator<QByteArray *> it(_readBuffer.values(client));
    while (it.hasNext())
        size += it.next()->size();
    data.resize(size);
    // Transfers the data to the Client buffer
    it.toFront();
    while (it.hasNext())
    {
        data.append(*it.peekNext());
        delete it.next();
    }
    _readBuffer.remove(client);
    client->bytesRead();
}

void PortUdp::write(Client *client, const QByteArray &data)
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
                Mutex mutex(_mutex, "PortTcp", "write");
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
                LOG_WARNING("An error occurred while writing the data", Properties("id", client->getId()).add("error", result).add("written", written).add("size", data.size() - written), "PortUdp", "write");
                break;
            }
        }
    }
    client->bytesWritten();
    return ;
}

void PortUdp::_write()
{
    Client *client;
    qint64 result;
    QHash<QSharedPointer<Client>, QSharedPointer<WriteBuffer> > writeBuffer;
    Mutex mutex(_mutex, "PortUdp", "_write");

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
                    LOG_WARNING("An error occurred while writing the data", Properties("return", result).add("size", w.getTotalSize()).add("written", w.bytesWritten()).add("id", client->getId()), "PortUdp", "_write");
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

void PortUdp::_finished(Client *client)
{
    Mutex mutex(_mutex, "PortUdp", "_finished");

    if (!mutex)
        return ;
    QMutableListIterator<QSharedPointer<Client> > it(_clients);
    while (it.hasNext())
        if (it.next().data() == client)
        {
            // Removes the data that have not been read by the Client
            QListIterator<QByteArray *> it2(_readBuffer.values(client));
            while (it2.hasNext())
                delete it2.next();
            _readBuffer.remove(client);
            it.remove();
            if (_clients.size() == 0 && !_serverUdp->isListening())
                _threadFinished.wakeAll();
            break;
        }
}
