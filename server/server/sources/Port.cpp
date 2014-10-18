#include "Configurations.h"
#include "Log.h"
#include "Port.h"
#include "Mutex.h"
#include "Threads.h"

Port::Port(unsigned short port, LightBird::INetwork::Transport transport, const QStringList &protocols, unsigned int maxClients)
{
    this->_port = port;
    this->_transport = transport;
    this->_protocols = protocols;
    if ((this->_maxClients = maxClients) == 0)
        this->_maxClients = ~0;
}

Port::~Port()
{
}

Future<bool>    Port::getClient(const QString &id, LightBird::INetwork::Client &client, bool &found) const
{
    Mutex   mutex(this->mutex, Mutex::READ, "Port", "getClient");

    found = false;
    if (!mutex)
        return (false);
    for (QListIterator<QSharedPointer<Client> > it(this->clients); it.hasNext(); it.next())
        if (it.peekNext()->getId() == id)
        {
            Future<bool> *future = new Future<bool>(false);
            Future<bool> result(*future);
            it.peekNext()->getInformations(client, future);
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
    // Stores the id of the clients in the string list
    for (QListIterator<QSharedPointer<Client> > it(this->clients); it.hasNext(); it.next())
        result << it.peekNext()->getId();
    return (result);
}

bool    Port::disconnect(const QString &id, bool fatal)
{
    Mutex   mutex(this->mutex, Mutex::READ, "Port", "disconnect");

    if (!mutex)
        return (false);
    // Searches the client that has the given id and disconnects it
    for (QListIterator<QSharedPointer<Client> > it(this->clients); it.hasNext(); it.next())
        if (it.peekNext()->getId() == id)
        {
            it.peekNext()->disconnect(fatal);
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
    // Searches the client
    for (QListIterator<QSharedPointer<Client> > it(this->clients); it.hasNext(); it.next())
        if (it.peekNext()->getId() == id)
        {
            client = it.peekNext().data();
            break;
        }
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
    for (QListIterator<QSharedPointer<Client> > it(this->clients); it.hasNext(); it.next())
        if (it.peekNext()->getId() == idClient)
            return (it.peekNext()->pause(msec));
    return (false);
}

bool    Port::resume(const QString &idClient)
{
    Mutex   mutex(this->mutex, Mutex::READ, "Port", "resume");

    if (!mutex)
        return (false);
    for (QListIterator<QSharedPointer<Client> > it(this->clients); it.hasNext(); it.next())
        if (it.peekNext()->getId() == idClient)
            return (it.peekNext()->resume());
    return (false);
}

QSharedPointer<Client> Port::_getClient(Client *client)
{
    for (QListIterator<QSharedPointer<Client> > it(this->clients); it.hasNext(); it.next())
        if (it.peekNext() == client)
            return it.peekNext();
    return QSharedPointer<Client>(NULL);
}
