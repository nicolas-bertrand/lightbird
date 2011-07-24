#include <QCoreApplication>

#include "Log.h"
#include "PortUdp.h"
#include "SmartMutex.h"
#include "Threads.h"

PortUdp::PortUdp(unsigned short port, const QStringList &protocols, unsigned int maxClients) :
                 Port(port, LightBird::INetwork::UDP, protocols, maxClients)
{
    this->moveToThread(this);
    this->socket.moveToThread(this);
    // Start the thread
    Threads::instance()->newThread(this, false);
    // Waits that the thread is started
    Future<bool>(this->threadStarted).getResult();
}

PortUdp::~PortUdp()
{
    // Quit the thread if it is still running
    this->quit();
    this->wait();
    Log::trace("Port UDP destroyed!", Properties("port", this->getPort()), "PortUdp", "~PortUdp");
}

void    PortUdp::run()
{
    // When a client connected to the server, the slot _read is called
    QObject::connect(&this->socket, SIGNAL(readyRead()), this, SLOT(_readPendingDatagrams()), Qt::QueuedConnection);
    // Listen on the given port
    if (!this->socket.bind(this->getPort()))
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
    // Remove the remaining clients
    QListIterator<Client *> client(this->clients);
    while (client.hasNext())
        delete client.next();
    this->clients.clear();
    this->moveToThread(QCoreApplication::instance()->thread());
    Log::info("Port closed", Properties("port", this->getPort()), "PortUdp", "PortUdp");
}

bool    PortUdp::read(QByteArray &data, Client *)
{
    data.clear();
    return (false);
}

bool            PortUdp::write(QByteArray *data, Client *client)
{
    SmartMutex  mutex(this->mutex, "PortUdp", "write");
    int         wrote;

    if (!mutex)
        return (false);
    // Write the data on the socket
    if ((wrote = this->socket.writeDatagram(*data, client->getPeerAddress(), client->getPeerPort())) != data->size())
    {
        Log::debug("All data has not been written", Properties("wrote", wrote).add("toWrite", data->size()).add("id", client->getId()), "PortUdp", "write");
        return (false);
    }
    return (true);
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
            client->read(data);
        }
        // There is already too many clients connected
        else
            delete data;
    }
}

Client          *PortUdp::_finished()
{
    SmartMutex  mutex(this->mutex, "PortUdp", "_finished");

    // While there is a client to remove
    while (mutex && Port::_finished())
        ;
    return (NULL);
}

bool    PortUdp::_isListening() const
{
    return (Port::_isListening() && this->socket.state() == QAbstractSocket::BoundState);
}
