#include <QCoreApplication>
#include <QTimer>

#include "Log.h"
#include "PortUdp.h"
#include "SmartMutex.h"
#include "Threads.h"

PortUdp::PortUdp(unsigned short port, const QStringList &protocols, unsigned int maxClients) :
                 Port(port, LightBird::INetwork::UDP, protocols, maxClients)
{
    this->moveToThread(this);
    this->socket.moveToThread(this);
    // Starts the thread
    Threads::instance()->newThread(this, false);
    // Waits that the thread is started
    Future<bool>(this->threadStarted).getResult();
}

PortUdp::~PortUdp()
{
    // Quits the thread if it is still running
    this->quit();
    this->wait();
    Log::trace("Port UDP destroyed!", Properties("port", this->getPort()), "PortUdp", "~PortUdp");
}

void    PortUdp::run()
{
    // When a client connected to the server, the slot _read is called
    QObject::connect(&this->socket, SIGNAL(readyRead()), this, SLOT(_readPendingDatagrams()), Qt::QueuedConnection);
    // Listens on the given port
    if (!this->socket.bind(QHostAddress::Any, this->getPort()))
    {
        Log::error("Failed to bind the port", Properties("port", this->getPort()).add("protocols", this->getProtocols().join(" "))
                   .add("transport", "UDP").add("maxClients", this->getMaxClients()), "PortUdp", "PortUdp");
        this->moveToThread(QCoreApplication::instance()->thread());
        return ;
    }
    Log::info("Listening...", Properties("port", this->getPort()).add("protocols", this->getProtocols().join(" "))
              .add("transport", "UDP").add("maxClients", this->getMaxClients()), "PortUdp", "PortUdp");
    Port::_isListening(true);
    this->threadStarted.setResult(true);
    this->exec();
    SmartMutex mutex(this->mutex, "PortUdp", "run");
    this->socket.close();
    // Removes the unread datagrams
    QHashIterator<Client *, QByteArray *> readBuffer(this->readBuffer);
    while (readBuffer.hasNext())
        delete readBuffer.next().value();
    this->readBuffer.clear();
    // Removes the remaining clients
    QListIterator<Client *> client(this->clients);
    while (client.hasNext())
        delete client.next();
    this->clients.clear();
    this->moveToThread(QCoreApplication::instance()->thread());
    Log::info("Port closed", Properties("port", this->getPort()), "PortUdp", "PortUdp");
}

void            PortUdp::read(Client *client)
{
    SmartMutex  mutex(this->mutex, "PortUdp", "read");
    QByteArray  &data = client->getData();
    quint64     size = 0;

    if (!mutex)
        return ;
    data.clear();
    // Gets the total size of the data buffers of this client
    QListIterator<QByteArray *> it(this->readBuffer.values(client));
    while (it.hasNext())
        size += it.next()->size();
    data.reserve(size);
    // Transfers the data to the Client buffer
    it.toFront();
    while (it.hasNext())
    {
        data.append(*it.peekNext());
        delete it.next();
    }
    this->readBuffer.remove(client);
    client->bytesRead();
}

bool            PortUdp::write(QByteArray *data, Client *client)
{
    SmartMutex  mutex(this->mutex, "PortUdp", "write");
    int         wrote;
    bool        result = true;

    if (!mutex)
        return (false);
    // Writes the data on the socket
    if ((wrote = this->socket.writeDatagram(*data, client->getPeerAddress(), client->getPeerPort())) != data->size())
    {
        Log::debug("All data has not been written", Properties("wrote", wrote).add("toWrite", data->size()).add("id", client->getId()), "PortUdp", "write");
        result = false;
    }
    // Notifies the Client that the data have been written
    QTimer::singleShot(0, client, SLOT(bytesWritten()));
    delete data;
    return (result);
}

void            PortUdp::close()
{
    SmartMutex  mutex(this->mutex, "PortUdp", "close");

    if (!mutex)
        return ;
    // Close the udp socket and disconect its signals
    this->socket.close();
    this->socket.disconnect();
    // Removes all the remaining clients
    Port::close();
}

void             PortUdp::_readPendingDatagrams()
{
    SmartMutex   mutex(this->mutex, "PortUdp", "_readPendingDatagrams");
    Client       *client;
    QHostAddress peerAddress;
    quint16      peerPort;

    if (!mutex)
        return ;
    // While there are pending datagrams, we read them
    while (this->socket.hasPendingDatagrams())
    {
        // Reads the next datagram
        QByteArray *data = new QByteArray();
        data->resize(this->socket.pendingDatagramSize());
        this->socket.readDatagram(data->data(), data->size(), &peerAddress, &peerPort);
        client = NULL;
        QListIterator<Client *> it(this->clients);
        while (it.hasNext() == true && client == NULL)
        {
            // The client is already connected
            if (it.peekNext()->getPeerAddress() == peerAddress && it.peekNext()->getPeerPort() == peerPort)
                client = it.peekNext();
            it.next();
        }
        if (client != NULL || (unsigned int)this->clients.size() < this->getMaxClients())
        {
            // If the client doesn't exists yet it is created
            if (client == NULL)
                client = this->_addClient(&this->socket, peerAddress, peerPort);
            // Notifies the Client that data are ready to be read
            this->readBuffer.insert(client, data);
            client->readyRead();
        }
        // There is already too many clients connected
        else
            delete data;
    }
}

Client  *PortUdp::_finished(Client *client)
{
    SmartMutex  mutex(this->mutex, "PortUdp", "_finished");

    if (mutex)
        return (NULL);
    // While there is a client to remove
    while ((client = Port::_finished()))
    {
        // Removes the data that have not been read by the Client
        QListIterator<QByteArray *> it(this->readBuffer.values(client));
        while (it.hasNext())
            delete it.next();
        this->readBuffer.remove(client);
    }
    return (NULL);
}

bool    PortUdp::_isListening() const
{
    return (Port::_isListening() && this->socket.state() == QAbstractSocket::BoundState);
}
