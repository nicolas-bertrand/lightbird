#include <QCoreApplication>
#include <QTcpSocket>

#include "Log.h"
#include "PortTcp.h"
#include "Mutex.h"
#include "Threads.h"

PortTcp::PortTcp(unsigned short port, const QStringList &protocols, unsigned int maxClients) :
                 Port(port, LightBird::INetwork::TCP, protocols, maxClients)
{
    this->moveToThread(this);
    this->tcpServer.moveToThread(this);
    // Starts the thread
    Threads::instance()->newThread(this, false);
    // Waits that the thread is started
    Future<bool>(this->threadStarted).getResult();
}

PortTcp::~PortTcp()
{
    Mutex   mutex(this->mutex, "PortTcp", "~PortTcp");

    if (!mutex)
        return ;
    // Quit the thread if it is still running
    this->quit();
    this->wait();
    LOG_TRACE("Port TCP destroyed!", Properties("port", this->getPort()), "PortTcp", "~PortTcp");
}

void    PortTcp::run()
{
    // When a client connects to the server, the slot _newConnection is called
    QObject::connect(&this->tcpServer, SIGNAL(newConnection()), this, SLOT(_newConnection()), Qt::QueuedConnection);
    // Allows to read and write data from this thread
    QObject::connect(this, SIGNAL(readSignal(Client*)), this, SLOT(_read(Client*)), Qt::QueuedConnection);
    QObject::connect(this, SIGNAL(writeSignal()), this, SLOT(_write()), Qt::QueuedConnection);
    // Listen on the given port
    if (!this->tcpServer.listen(QHostAddress::Any, this->getPort()))
    {
        LOG_ERROR("Failed to listen on the port", Properties("port", this->getPort()).add("protocols", this->getProtocols().join(" "))
                   .add("transport", "TCP").add("maxClients", this->getMaxClients()), "PortTcp", "PortTcp");
        this->threadStarted.setResult(false);
        this->moveToThread(QCoreApplication::instance()->thread());
        return ;
    }
    LOG_INFO("Listening...", Properties("port", this->getPort()).add("protocols", this->getProtocols().join(" "))
              .add("transport", "TCP").add("maxClients", this->getMaxClients()), "PortTcp", "PortTcp");
    Port::_isListening(true);
    this->threadStarted.setResult(true);
    this->exec();
    Mutex mutex(this->mutex, "PortTcp", "run");
    this->tcpServer.close();
    // Remove the remaining clients
    QListIterator<Client *> client(this->clients);
    while (client.hasNext())
        delete client.next();
    this->clients.clear();
    this->moveToThread(QCoreApplication::instance()->thread());
    LOG_INFO("Port closed", Properties("port", this->getPort()), "PortTcp", "PortTcp");
}

void    PortTcp::close()
{
    Mutex   mutex(this->mutex, "PortTcp", "close");

    if (!mutex)
        return ;
    // Stop listening on the network
    this->tcpServer.close();
    // Removes all the remaining clients
    Port::close();
}

void    PortTcp::_newConnection()
{
    Mutex       mutex(this->mutex, "PortTcp", "_newConnection");
    QTcpSocket  *socket;
    Client      *client;

    if (!mutex)
        return ;
    // Iterates other all the pending connections
    while (this->tcpServer.hasPendingConnections() && (unsigned int)this->clients.size() < this->getMaxClients())
    {
        // Creates the socket of the client if the socket is in connected state
        if ((socket = this->tcpServer.nextPendingConnection())->state() == QAbstractSocket::ConnectedState)
        {
            // Creates the client
            client = this->_addClient(socket, socket->peerAddress(), socket->peerPort());
            socket->setParent(client);
            // Join the client and its socket
            this->sockets[socket] = client;
            // When new data are received on this socket, read() is called on the client
            QObject::connect(socket, SIGNAL(readyRead()), client, SLOT(readyRead()), Qt::QueuedConnection);
            // When the client is disconnected, _disconnected() is called
            QObject::connect(socket, SIGNAL(disconnected()), this, SLOT(_disconnected()), Qt::QueuedConnection);
            // Read the data received between the creation of the client and the connection of the read signal
            client->readyRead();
        }
        else
        {
            LOG_DEBUG("Invalid socket", Properties("port", this->getPort()).add("state", socket->state()), "PortTcp", "_newConnection");
            delete socket;
        }
    }
}

void    PortTcp::read(Client *client)
{
    emit this->readSignal(client);
}

void    PortTcp::_read(Client *client)
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
                         .add("error", read).add("size", data.size()), "PortTcp", "_read");
            data.resize(read);
        }
    }
    client->bytesRead();
}

bool    PortTcp::write(QByteArray *data, Client *client)
{
    Mutex   mutex(this->mutex, "PortTcp", "write");

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

void    PortTcp::_write()
{
    Mutex       mutex(this->mutex, "PortTcp", "_write");
    Client      *client;
    qint64      result;
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
                    LOG_WARNING("An error occured while writing the data", Properties("return", result).add("size", w.getTotalSize()).add("id", client->getId()), "PortTcp", "_write");
                if (result == 0)
                    LOG_ERROR("Write returned 0", Properties("size", w.getTotalSize()).add("id", client->getId()), "PortTcp", "_write");
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

void    PortTcp::_disconnected()
{
    Mutex   mutex(this->mutex, Mutex::READ, "PortTcp", "_disconnected");
    Client  *client;
    QAbstractSocket *socket;

    if (!mutex)
        return ;
    // If the sender of the signal is a QAbstractSocket
    if ((socket = qobject_cast<QAbstractSocket *>(this->sender())))
        // Searches the client associated with this socket
        if ((client = this->sockets.value(socket)))
        {
            // Removes the client of the disconnected socket
            this->_removeClient(client);
            // Removes the client from the write buffer since we won't be able to write to him anymore
            this->writeBuffer.remove(client);
        }
}

Client  *PortTcp::_finished(Client *client)
{
    Mutex   mutex(this->mutex, "PortTcp", "_finished");

    if (!mutex)
        return (NULL);
    // Searches the clients that have been finished
    client = NULL;
    QListIterator<Client *> it(this->clients);
    while (it.hasNext())
        // The client is deleted only if there is no remaining data in the writeBuffer
        if ((client = it.next())->isFinished() && !this->writeBuffer.contains(client))
        {
            // Removes the client
            Port::_finished(client);
            // Removes the socket from the sockets map
            this->sockets.remove(this->sockets.key(client));
        }
    // If the server is still listening and there are pending connections,
    // new clients can replace those that were destroyed.
    if (client && this->_isListening() && this->tcpServer.hasPendingConnections())
    {
        mutex.unlock();
        this->_newConnection();
    }
    return (NULL);
}

bool    PortTcp::_isListening() const
{
    return (Port::_isListening() && this->tcpServer.isListening());
}
