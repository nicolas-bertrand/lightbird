#include <QTcpSocket>

#include "Log.h"
#include "PortTcp.h"

PortTcp::PortTcp(unsigned short port, const QStringList &protocols, unsigned int maxClients, QObject *parent) :
                 Port(port, LightBird::INetwork::TCP, protocols, maxClients, parent)
{
    // When a client connects to the server, the slot _newConnection is called
    QObject::connect(&this->tcpServer, SIGNAL(newConnection()), this, SLOT(_newConnection()), Qt::QueuedConnection);
    // Listen on the given port
    if (!this->tcpServer.listen(QHostAddress::Any, port))
    {
        Log::error("Failed to listen", Properties("port", port).add("protocols", protocols.join(" ")).add("transport", "TCP")
                   .add("maxClients", this->getMaxClients()), "PortTcp", "PortTcp");
        return ;
    }
    Log::info("Listening...", Properties("port", port).add("protocols", protocols.join(" ")).add("transport", "TCP")
              .add("maxClients", this->getMaxClients()), "PortTcp", "PortTcp");
    this->_isListening(true);
}

PortTcp::~PortTcp()
{
    Log::trace("PortTcp destroyed!", Properties("Port", this->getPort()), "PortTcp", "~PortTcp");
}

bool    PortTcp::isListening()
{
    if (!Port::isListening())
        return (false);
    return (this->tcpServer.isListening());
}

void    PortTcp::stopListening()
{
    Log::error("Port closed", Properties("port", this->getPort()).add("protocols", this->getProtocols().join(" ")).add("transport", "TCP"), "PortTcp", "stopListening");
    // Stop listening on the network
    this->tcpServer.close();
    // Removes all the remaining clients
    Port::stopListening();
}

bool    PortTcp::read(QByteArray &data, Client *client)
{
    data.clear();
    // Calls the IDoRead interface of the plugins
    if (!client->doRead(data))
        //If no plugins implements it, the server read the data itself
        data = client->getSocket().readAll();
    if (data.size() > 0)
        return (true);
    return (false);
}

bool    PortTcp::write(QByteArray &data, Client *client)
{
    int wrote;

    // Calls the IDoWrite interface of the plugins
    if (!client->doWrite(data))
    {
        //If no plugins implements it, the server write the data itself
        if ((wrote = client->getSocket().write(data)) != data.size())
        {
            Log::warning("All data has not been written", Properties("wrote", wrote).add("toWrite", data.size()).add("id", client->getId()), "PortTcp", "write");
            return (false);
        }
        client->getSocket().waitForBytesWritten();
    }
    return (true);
}

void    PortTcp::_newConnection()
{
    QTcpSocket  *socket;
    Client      *client;

    // Iterates other all the pending connections
    while (this->tcpServer.hasPendingConnections() && (unsigned int)this->clients.size() < this->getMaxClients())
    {
        // Creates the socket of the client if the socket is in connected state
        if ((socket = this->tcpServer.nextPendingConnection())->state() == QAbstractSocket::ConnectedState)
        {
            socket->setParent(NULL);
            // Creates the client
            client = this->_addClient(socket, socket->peerAddress(), socket->peerPort());
            // Join the client and its socket
            this->sockets[socket] = client;
            // When new data are received on this socket, _read is called
            QObject::connect(socket, SIGNAL(readyRead()), client, SLOT(read()), Qt::QueuedConnection);
            // When the client is disconnected, _disconnected() is called
            QObject::connect(socket, SIGNAL(disconnected()), this, SLOT(_disconnected()), Qt::QueuedConnection);
        }
        else
        {
            Log::debug("Invalid socket", Properties("port", this->getPort()).add("state", socket->state()), "PortTcp", "_newConnection");
            delete socket;
        }
    }
}

void    PortTcp::_disconnected()
{
    QAbstractSocket *socket;

    // If the sender of the signal is a QAbstractSocket
    if ((socket = qobject_cast<QAbstractSocket *>(this->sender())))
    {
        // Search the client associated with this socket
        if (this->sockets.contains(socket))
        {
            // Removes the client of the disconnected socket
            this->_removeClient(this->sockets.value(socket));
        }
    }
}

Client      *PortTcp::_finished()
{
    Client  *client;

    // Removes the client from the clients list, and delete its instance
    if ((client = Port::_finished()))
    {
        // Removes the socket from the sockets map
        this->sockets.remove(this->sockets.key(client));
        // If the server is still listening and there are pending connections, a new client can replace
        // the destroyed client (if maxNumberOfclient has been reached)
        if (this->isListening() && this->tcpServer.hasPendingConnections())
            this->_newConnection();
        // This method is called while there is a finished client to ensure that they have all been deleted
        this->_finished();
    }
    return (NULL);
}
