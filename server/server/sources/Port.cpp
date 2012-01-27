#include "Configurations.h"
#include "Log.h"
#include "Port.h"
#include "SmartMutex.h"
#include "Threads.h"

Port::Port(unsigned short port, LightBird::INetwork::Transport transport, const QStringList &protocols, unsigned int maxClients)
{
    this->port = port;
    this->transport = transport;
    this->protocols = protocols;
    this->listening = false;
    if ((this->maxClients = maxClients) == 0)
        this->maxClients = ~0;
}

Port::~Port()
{
    // Delete all the remaining clients
    QListIterator<Client *> it(this->clients);

    while (it.hasNext())
    {
        delete it.peekNext();
        it.next();
    }
}

Future<bool>    Port::getClient(const QString &id, LightBird::INetwork::Client &client, bool &found) const
{
    SmartMutex  mutex(this->mutex, SmartMutex::READ, "Port", "getClient");

    found = false;
    if (!mutex)
        return (false);
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

QStringList     Port::getClients() const
{
    SmartMutex  mutex(this->mutex, SmartMutex::READ, "Port", "getClients");
    QStringList result;

    if (!mutex)
        return (result);
    QListIterator<Client *> it(this->clients);
    // Stores the id of the clients in the string list
    while (it.hasNext())
        result << it.next()->getId();
    return (result);
}

bool            Port::disconnect(const QString &id)
{
    SmartMutex  mutex(this->mutex, SmartMutex::READ, "Port", "disconnect");

    if (!mutex)
        return (false);
    QListIterator<Client *> it(this->clients);
    // Searches the client that has the given id and disconnects it
    while (it.hasNext())
        if (it.next()->getId() == id)
        {
            it.peekPrevious()->disconnect();
            return (true);
        }
    return (false);
}

bool            Port::send(const QString &id, const QString &p, const QVariantMap &informations)
{
    SmartMutex  mutex(this->mutex, SmartMutex::READ, "Port", "send");
    Client      *client = NULL;
    QString     protocol;

    if (!mutex)
        return (false);
    QListIterator<Client *> it(this->clients);
    // Searches the client
    while (it.hasNext() && !client)
        if (it.next()->getId() == id)
            client = it.peekPrevious();
    // The client doesn't exists
    if (!client)
        return (false);
    // Checks the protocol
    if ((protocol = client->getProtocol(p)).isEmpty())
    {
        Log::warning("Invalid protocol", Properties("id", id).add("protocol", p, false), "Port", "send");
        return (false);
    }
    return (client->send(protocol, informations));
}

void            Port::close()
{
    QListIterator<Client *> it(this->clients);

    this->listening = false;
    // Disconnects all the clients in the map
    if (this->clients.size() > 0)
        while (it.hasNext())
            this->_removeClient(it.next());
    // Or quit the thread if there is already no remaining clients
    else
        this->quit();
}

Client          *Port::_addClient(QAbstractSocket *socket, const QHostAddress &peerAddress, unsigned short peerPort)
{
    Client      *client;

    // Creates the client
    client = new Client(socket, this->transport, this->protocols, this->port,
                        socket->socketDescriptor(), peerAddress, peerPort,
                        socket->peerName(), LightBird::IClient::SERVER, this);
    // Add the client
    this->clients.push_back(client);
    // When the client thread is finished, _finished is called
    QObject::connect(client, SIGNAL(finished()), this, SLOT(_finished()), Qt::QueuedConnection);
    return (client);
}

void            Port::_removeClient(Client *client)
{
    // Check if the client is in the clients list
    if (this->clients.contains(client))
        // Disconnect the client
        client->disconnect();
}

Client          *Port::_finished()
{
    Client      *client = NULL;

    // Search the client that has been finished
    QListIterator<Client *> it(this->clients);
    while (it.hasNext() && !client)
        if (it.next()->isFinished())
            client = it.peekPrevious();
    // Delete the client
    this->clients.removeAll(client);
    delete client;
    // If there are no more connected client and the server is no longer listening, the thread is quit
    if (this->clients.size() == 0 && !this->_isListening())
        this->quit();
    return (client);
}

unsigned short  Port::getPort() const
{
    return (this->port);
}

LightBird::INetwork::Transport Port::getTransport() const
{
    return (this->transport);
}

const QStringList   &Port::getProtocols() const
{
    return (this->protocols);
}

unsigned int    Port::getMaxClients() const
{
    return (this->maxClients);
}

bool            Port::isListening() const
{
    SmartMutex  mutex(this->mutex, SmartMutex::READ, "Port", "isListening");

    return (mutex && this->_isListening());
}

bool            Port::_isListening() const
{
    return (this->listening && this->isRunning());
}

void            Port::_isListening(bool listening)
{
    this->listening = listening;
}
