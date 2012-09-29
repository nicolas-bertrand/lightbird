#include <QCoreApplication>
#include <QTcpSocket>

#include "Log.h"
#include "PortTcp.h"
#include "SmartMutex.h"
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
    SmartMutex  mutex(this->mutex, "PortTcp", "~PortTcp");

    if (!mutex)
        return ;
    // Quit the thread if it is still running
    this->quit();
    this->wait();
    // Clean the write buffer
    while (!this->writeBuffer.isEmpty())
        delete this->writeBuffer.dequeue().second;
    Log::trace("Port TCP destroyed!", Properties("port", this->getPort()), "PortTcp", "~PortTcp");
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
        Log::error("Failed to listen on the port", Properties("port", this->getPort()).add("protocols", this->getProtocols().join(" "))
                   .add("transport", "TCP").add("maxClients", this->getMaxClients()), "PortTcp", "PortTcp");
        this->threadStarted.setResult(false);
        this->moveToThread(QCoreApplication::instance()->thread());
        return ;
    }
    Log::info("Listening...", Properties("port", this->getPort()).add("protocols", this->getProtocols().join(" "))
              .add("transport", "TCP").add("maxClients", this->getMaxClients()), "PortTcp", "PortTcp");
    Port::_isListening(true);
    this->threadStarted.setResult(true);
    this->exec();
    SmartMutex mutex(this->mutex, "PortTcp", "run");
    this->tcpServer.close();
    // Remove the remaining clients
    QListIterator<Client *> client(this->clients);
    while (client.hasNext())
        delete client.next();
    this->clients.clear();
    this->moveToThread(QCoreApplication::instance()->thread());
    Log::info("Port closed", Properties("port", this->getPort()), "PortTcp", "PortTcp");
}

void    PortTcp::read(Client *client)
{
    emit this->readSignal(client);
}

bool            PortTcp::write(QByteArray *data, Client *client)
{
    SmartMutex  mutex(this->mutex, "PortTcp", "write");

    if (!mutex)
        return (false);
    // If a signal has not already be send
    if (this->writeBuffer.isEmpty())
        emit writeSignal();
    this->writeBuffer.enqueue(QPair<Client *, QByteArray *>(client, data));
    this->writeBufferClients.push_back(client);
    return (true);
}

void            PortTcp::close()
{
    SmartMutex  mutex(this->mutex, "PortTcp", "close");

    if (!mutex)
        return ;
    // Stop listening on the network
    this->tcpServer.close();
    this->tcpServer.disconnect();
    // Removes all the remaining clients
    Port::close();
}

void            PortTcp::_newConnection()
{
    SmartMutex  mutex(this->mutex, "PortTcp", "_newConnection");
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
            // When the data have been written on this socket, Client::written is called
            QObject::connect(socket, SIGNAL(bytesWritten(qint64)), client, SLOT(bytesWritten()), Qt::QueuedConnection);
            // When the client is disconnected, _disconnected() is called
            QObject::connect(socket, SIGNAL(disconnected()), this, SLOT(_disconnected()), Qt::QueuedConnection);
            // Read the data received between the creation of the client and the connection of the read signal
            client->readyRead();
        }
        else
        {
            Log::debug("Invalid socket", Properties("port", this->getPort()).add("state", socket->state()), "PortTcp", "_newConnection");
            delete socket;
        }
    }
}

void            PortTcp::_read(Client *client)
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
            Log::warning("An error occured while reading the data", Properties("id", client->getId())
                         .add("error", read).add("size", data.size()), "PortTcp", "_read");
            data.resize(read);
        }
    }
    client->bytesRead();
}

void            PortTcp::_write()
{
    SmartMutex  mutex(this->mutex, "PortTcp", "_write");
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
        // The API should never be called in a mutex, and clients is only modified in the current thread
        mutex.unlock();
        // Calls the IDoWrite interface of the plugins, if the client still exists and is connected
        if (this->clients.contains(client) &&
            client->getSocket().state() == QAbstractSocket::ConnectedState &&
            !client->doWrite(*data))
        {
            // If no plugins implements IDoWrite, the server write the data itself
            if ((wrote = client->getSocket().write(*data)) != data->size())
                Log::warning("All data has not been written", Properties("wrote", wrote)
                             .add("size", data->size()).add("id", client->getId()), "PortTcp", "write");
        }
        mutex.lock();
        delete data;
        this->writeBuffer.dequeue();
        // Notifies the Client that the data are being written
        client->bytesWriting();
    }
    this->writeBufferClients.clear();
}

void                PortTcp::_disconnected()
{
    SmartMutex      mutex(this->mutex, SmartMutex::READ, "PortTcp", "_disconnected");
    QAbstractSocket *socket;

    if (!mutex)
        return ;
    // If the sender of the signal is a QAbstractSocket
    if ((socket = qobject_cast<QAbstractSocket *>(this->sender())))
        // Searches the client associated with this socket
        if (this->sockets.contains(socket))
            // Removes the client of the disconnected socket
            this->_removeClient(this->sockets.value(socket));
}

Client          *PortTcp::_finished(Client *client)
{
    SmartMutex  mutex(this->mutex, "PortTcp", "_finished");

    if (!mutex)
        return (NULL);
    // Searches the clients that have been finished
    client = NULL;
    QListIterator<Client *> it(this->clients);
    while (it.hasNext())
        // The client is deleted only if there is no remaining data in the writeBuffer
        if ((client = it.next())->isFinished() && !this->writeBufferClients.contains(client))
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
