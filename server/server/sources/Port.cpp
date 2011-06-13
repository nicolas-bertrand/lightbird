#include "Configurations.h"
#include "Log.h"
#include "Port.h"
#include "Threads.h"

Port::Port(unsigned short port, LightBird::INetwork::Transport transport, const QStringList &protocols,
           unsigned int maxClients, QObject *object) : QObject(object)
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
        it.peekNext()->quit();
        it.peekNext()->wait();
        delete it.peekNext();
        it.next();
    }
}

unsigned short  Port::getPort()
{
    return (this->port);
}

LightBird::INetwork::Transport Port::getTransport()
{
    return (this->transport);
}

const QStringList   &Port::getProtocols()
{
    return (this->protocols);
}

unsigned int    Port::getMaxClients()
{
    return (this->maxClients);
}

bool            Port::isListening()
{
    return (this->listening);
}

void            Port::stopListening()
{
    QListIterator<Client *> it(this->clients);

    // Removes all the clients in the map
    if (this->clients.size() > 0)
        while (it.hasNext())
            this->_removeClient(it.next());
    // Or emit a signal if there is already no remaining clients
    else
        emit this->allClientsRemoved(this->port);
}

bool            Port::getClient(const QString &id, LightBird::INetwork::Client *client, void *thread, Future<bool> *future)
{
    QListIterator<Client *> it(this->clients);

    // Search the client
    while (it.hasNext())
        if (it.next()->getId() == id)
        {
            it.peekPrevious()->getInformations(client, thread, future);
            return (true);
        }
    return (false);
}

QStringList     Port::getClients()
{
    QStringList             result;
    QListIterator<Client *> it(this->clients);

    // Stores the id of the clients in a string list
    while (it.hasNext())
        result << it.next()->getId();
    return (result);
}

bool            Port::disconnect(const QString &id)
{
    QListIterator<Client *> it(this->clients);

    // Search the client that has the given id and disconnect it
    while (it.hasNext())
        if (it.next()->getId() == id)
        {
            it.peekPrevious()->quit();
            return (true);
        }
    return (false);
}

void            Port::_isListening(bool listening)
{
    this->listening = listening;
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
        // Quit the client's thread. Its instance will also be deleted after the thread finished.
        Threads::instance()->deleteThread(client);
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
    // If there are no more connected client and the server is no longer listening, the signal allClientsRemoved is emited
    if (this->clients.size() == 0 && !this->isListening())
        emit this->allClientsRemoved(this->port);
    return (client);
}
