#include <QCoreApplication>

#include "Log.h"
#include "PortTcp.h"
#include "Mutex.h"
#include "Threads.h"

PortTcp::PortTcp(unsigned short port, const QStringList &protocols, unsigned int maxClients)
    : Port(port, LightBird::INetwork::TCP, protocols, maxClients)
    , _serverTcp(NULL)
{
    this->moveToThread(this);
    // Starts the thread
    Threads::instance()->newThread(this, false);
    // Waits that the thread is started
    Future<bool>(_threadStarted).getResult();
}

PortTcp::~PortTcp()
{
    Mutex mutex(this->mutex, "PortTcp", "~PortTcp");

    if (!mutex)
        return ;
    if (_serverTcp)
        _serverTcp->close();
    mutex.unlock();
    // Quit the thread if it is still running
    this->quit();
    this->wait();
    delete _serverTcp;
    LOG_TRACE("Port TCP destroyed!", Properties("port", this->getPort()), "PortTcp", "~PortTcp");
}

void PortTcp::run()
{
    // Creates the TCP server
    _serverTcp = ServerTcp::create(port);
    // Listen on the given port
    if (!_serverTcp->isListening())
    {
        LOG_ERROR("Failed to listen on the port", Properties("port", this->getPort()).add("protocols", this->getProtocols().join(" ")).add("transport", "TCP").add("maxClients", this->getMaxClients()), "PortTcp", "PortTcp");
        this->_threadStarted.setResult(false);
        this->moveToThread(QCoreApplication::instance()->thread());
        return ;
    }
    // When a client connects to the server, the slot _newConnection is called
    QObject::connect(_serverTcp, SIGNAL(newConnection()), this, SLOT(_newConnection()), Qt::DirectConnection);
    LOG_INFO("Listening...", Properties("port", this->getPort()).add("protocols", this->getProtocols().join(" ")).add("transport", "TCP").add("maxClients", this->getMaxClients()), "PortTcp", "PortTcp");
    _threadStarted.setResult(true);
    // This method only returns when the port is closed
    _serverTcp->execute();
    Mutex mutex(this->mutex, "PortTcp", "run");
    // If some clients are still running, we wait for them
    if (this->clients.size())
        _threadFinished.wait(&this->mutex);
    // Remove the remaining clients
    this->clients.clear();
    _writeBuffers.clear();
    this->moveToThread(QCoreApplication::instance()->thread());
    LOG_INFO("Port closed", Properties("port", this->getPort()), "PortTcp", "PortTcp");
}

void PortTcp::close()
{
    Mutex mutex(this->mutex, "PortTcp", "close");

    if (!mutex)
        return ;
    // Stop listening on the network
    if (_serverTcp)
        _serverTcp->close();
    // Disconnects all the clients in the map
    if (this->clients.size() > 0)
        for (QListIterator<QSharedPointer<Client> > it(this->clients); it.hasNext(); it.next())
            it.peekNext()->disconnect();
    // Or quit the thread if there is already no remaining clients
    else
        _threadFinished.wakeAll();
}

void PortTcp::_newConnection()
{
    Mutex mutex(this->mutex, "PortTcp", "_newConnection");
    QSharedPointer<Socket> socket;
    Client *client;

    if (!mutex)
        return ;
    // Iterates other all the pending connections
    while (_serverTcp->hasPendingConnections() && (unsigned int)this->clients.size() < this->getMaxClients())
    {
        // Creates the socket of the client if the socket is in connected state
        if ((socket = _serverTcp->nextPendingConnection()))
        {
            // Creates the client
            client = new Client(socket, this->protocols, this->transport, LightBird::IClient::SERVER, this);
            client->connected(true);
            // Adds the client
            this->clients.push_back(QSharedPointer<Client>(client));
            // When new data are received on this socket, Client::readyRead is called
            QObject::connect(socket.data(), SIGNAL(readyRead()), client, SLOT(readyRead()), Qt::DirectConnection);
            // When the socket is disconnected, _disconnected() is called
            QObject::connect(socket.data(), SIGNAL(disconnected(Socket*)), this, SLOT(_disconnected(Socket*)), Qt::DirectConnection);
            // When the client is finished, _finished is called
            QObject::connect(client, SIGNAL(finished()), this, SLOT(_finished()), Qt::DirectConnection);
        }
    }
}

void PortTcp::read(Client *client)
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

void PortTcp::write(Client *client, const QByteArray &data)
{
    qint64 result;
    qint64 written = 0;
    bool doWrite = true;

    if (client->getSocket().isConnected() && data.size())
    {
        while (written < data.size())
        {
            // Tries to call the IDoWrite interface of the plugins
            if (doWrite && !(doWrite = client->doWrite(data.data() + written, data.size() - written, result)))
                // If no plugins implements IDoWrite, we write the data ourselves
                result = client->getSocket().write(data.data() + written, data.size() - written);
            if (result > 0)
                written += result;
            // The socket is not ready to write more data, so we wait until readyWrite is emited
            else if (result == 0)
            {
                Mutex mutex(this->mutex, "PortTcp", "write");
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
                LOG_WARNING("An error occurred while writing the data", Properties("id", client->getId()).add("error", result).add("written", written).add("size", data.size() - written), "PortTcp", "write");
                break;
            }
        }
    }
    client->bytesWritten();
    return ;
}

void PortTcp::_write()
{
    Client *client;
    qint64 result;
    QHash<QSharedPointer<Client>, QSharedPointer<WriteBuffer> > writeBuffer;
    Mutex mutex(this->mutex, "PortTcp", "_write");

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
                    LOG_WARNING("An error occurred while writing the data", Properties("return", result).add("size", w.getTotalSize()).add("written", w.bytesWritten()).add("id", client->getId()), "PortTcp", "_write");
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

void PortTcp::_disconnected(Socket *socket)
{
    Mutex mutex(this->mutex, Mutex::READ, "PortTcp", "_disconnected");

    if (!mutex)
        return ;
    // Searches the client associated with this socket
    for (QListIterator<QSharedPointer<Client> > it(this->clients); it.hasNext(); it.next())
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

void PortTcp::_finished()
{
    Mutex mutex(this->mutex, "PortTcp", "_finished");

    if (!mutex)
        return ;
    // Searches the clients that have been finished
    QMutableListIterator<QSharedPointer<Client> > it(this->clients);
    while (it.hasNext())
        if (it.next()->isFinished())
        {
            it.remove();
            if (this->clients.size() == 0 && !_serverTcp->isListening())
                _threadFinished.wakeAll();
        }
}
