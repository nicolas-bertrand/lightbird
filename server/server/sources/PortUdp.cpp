#include <QTimer>

#include "Log.h"
#include "PortUdp.h"

PortUdp::PortUdp(unsigned short port, const QStringList &protocols, unsigned int maxClients, QObject *parent) :
                 Port(port, LightBird::INetwork::UDP, protocols, maxClients, parent)
{
    // When a client connected to the server, the slot _read is called
    QObject::connect(&this->socket, SIGNAL(readyRead()), this, SLOT(_readPendingDatagrams()), Qt::QueuedConnection);
    // Listen on the given port
    if (!this->socket.bind(port))
    {
        Log::error("Failed to bind", Properties("port", port).add("protocols", protocols.join(" ")).add("transport", "UDP")
                   .add("maxClients", this->getMaxClients()), "PortUdp", "PortUdp");
        return ;
    }
    Log::info("Listening...", Properties("port", port).add("protocols", protocols.join(" ")).add("transport", "UDP")
              .add("maxClients", this->getMaxClients()), "PortUdp", "PortUdp");
    this->_isListening(true);
}

PortUdp::~PortUdp()
{
    Log::trace("PortUdp destroyed!", Properties("Port", this->getPort()), "PortUdp", "~PortUdp");
}

bool    PortUdp::isListening()
{
    if (!Port::isListening() || this->socket.state() != QAbstractSocket::BoundState)
        return (false);
    return (true);
}

void    PortUdp::stopListening()
{
    Log::error("Port closed", Properties("port", this->getPort()).add("protocols", this->getProtocols().join(" ")).add("transport", "UDP"), "PortUdp", "stopListening");
    // Close the udp socket and disconect its signals
    this->socket.close();
    this->socket.disconnect();
    // Removes all the remaining clients
    Port::stopListening();
}

bool    PortUdp::read(QByteArray &data, QObject *)
{
    data.clear();
    return (false);
}

bool    PortUdp::write(QByteArray &data, QObject *caller)
{
    Client  *client;
    int     wrote;

    // If the caller is a client, write the data on the socket
    if ((client = qobject_cast<Client *>(caller)))
        if ((wrote = this->socket.writeDatagram(data, client->getPeerAddress(), client->getPeerPort())) != data.size())
        {
            Log::debug("All data has not been written", Properties("wrote", wrote).add("toWrite", data.size()).add("id", client->getId()), "PortUdp", "write");
            return (false);
        }
    return (true);
}

void    PortUdp::_readPendingDatagrams()
{
    Client          *client;
    QHostAddress    peerAddress;
    quint16         peerPort;

    // While there are pending datagrams, we read them
    while (this->socket.hasPendingDatagrams())
    {
        QByteArray *data = new QByteArray();
        data->resize(this->socket.pendingDatagramSize());
        this->socket.readDatagram(data->data(), data->size(), &peerAddress, &peerPort);
        client = NULL;
        QListIterator<Client *> it(this->clients);
        while (it.hasNext() == true && client == NULL)
        {
            if (it.peekNext()->getPeerAddress() == peerAddress && it.peekNext()->getPeerPort() == peerPort)
                client = it.peekNext();
            it.next();
        }
        if (client != NULL || (unsigned int)this->clients.size() < this->getMaxClients())
        {
            if (client == NULL)
                client = this->_addClient(&this->socket, peerAddress, peerPort);
            client->read(data);
        }
        else
            delete data;
    }
}
