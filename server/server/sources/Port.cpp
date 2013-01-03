#include "Configurations.h"
#include "Log.h"
#include "Port.h"
#include "Mutex.h"
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
    // Deletes all the remaining clients
    QListIterator<Client *> it(this->clients);

    while (it.hasNext())
    {
        delete it.peekNext();
        it.next();
    }
}

Future<bool>    Port::getClient(const QString &id, LightBird::INetwork::Client &client, bool &found) const
{
    Mutex   mutex(this->mutex, Mutex::READ, "Port", "getClient");

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

QStringList Port::getClients() const
{
    Mutex       mutex(this->mutex, Mutex::READ, "Port", "getClients");
    QStringList result;

    if (!mutex)
        return (result);
    QListIterator<Client *> it(this->clients);
    // Stores the id of the clients in the string list
    while (it.hasNext())
        result << it.next()->getId();
    return (result);
}

bool    Port::disconnect(const QString &id)
{
    Mutex   mutex(this->mutex, Mutex::READ, "Port", "disconnect");

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

bool    Port::send(const QString &id, const QString &p, const QVariantMap &informations)
{
    Mutex   mutex(this->mutex, Mutex::READ, "Port", "send");
    Client  *client = NULL;
    QString protocol;

    if (!mutex)
        return (false);
    QListIterator<Client *> it(this->clients);
    // Searches the client
    while (it.hasNext() && !client)
        if (it.next()->getId() == id)
            client = it.peekPrevious();
    // The client does not exist
    if (!client)
        return (false);
    // Checks the protocol
    if ((protocol = client->getProtocol(p)).isEmpty())
    {
        LOG_WARNING("Invalid protocol", Properties("id", id).add("protocol", p, false), "Port", "send");
        return (false);
    }
    return (client->send(protocol, informations));
}

bool    Port::pause(const QString &idClient, int msec)
{
    Mutex   mutex(this->mutex, Mutex::READ, "Port", "pause");

    if (!mutex)
        return (false);
    QListIterator<Client *> it(this->clients);
    while (it.hasNext())
        if (it.next()->getId() == idClient)
            return (it.peekPrevious()->pause(msec));
    return (false);
}

bool    Port::resume(const QString &idClient)
{
    Mutex   mutex(this->mutex, Mutex::READ, "Port", "resume");

    if (!mutex)
        return (false);
    QListIterator<Client *> it(this->clients);
    while (it.hasNext())
        if (it.next()->getId() == idClient)
            return (it.peekPrevious()->resume());
    return (false);
}

void    Port::close()
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

Client  *Port::_addClient(QAbstractSocket *socket, const QHostAddress &peerAddress, unsigned short peerPort)
{
    Client  *client;

    // Creates the client
    client = new Client(socket, this->protocols, this->transport, this->port,
                        socket->socketDescriptor(), peerAddress, peerPort,
                        socket->peerName(), LightBird::IClient::SERVER, this);
    // Adds the client
    this->clients.push_back(client);
    // When the client is finished, _finished is called
    QObject::connect(client, SIGNAL(finished()), this, SLOT(_finished()), Qt::QueuedConnection);
    return (client);
}

void    Port::_removeClient(Client *client)
{
    // Checks if the client is in the clients list
    if (this->clients.contains(client))
        // Disconnects the client
        client->disconnect();
}

Client  *Port::_finished(Client *client)
{
    // Searches a finished client
    QListIterator<Client *> it(this->clients);
    while (it.hasNext() && !client)
        if (it.next()->isFinished())
            client = it.peekPrevious();
    // No client found
    if (!client)
        return (NULL);
    // Deletes the client
    this->clients.removeAll(client);
    delete client;
    // If there are no more connected client and the server is no longer listening, we quit the thread
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
    Mutex   mutex(this->mutex, Mutex::READ, "Port", "isListening");

    return (mutex && this->_isListening());
}

bool    Port::_isListening() const
{
    return (this->listening && this->isRunning());
}

void    Port::_isListening(bool listening)
{
    this->listening = listening;
}
