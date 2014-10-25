#include "Configurations.h"
#include "Log.h"
#include "Port.h"
#include "Mutex.h"
#include "Threads.h"

Port::Port(unsigned short port, LightBird::INetwork::Transport transport, const QStringList &protocols, const QStringList &contexts, unsigned int maxClients)
{
    _port = port;
    _transport = transport;
    _protocols = protocols;
    _contexts = contexts;
    if ((_maxClients = maxClients) == 0)
        _maxClients = ~0;
}

Port::~Port()
{
}

Future<bool>    Port::getClient(const QString &id, LightBird::INetwork::Client &client, bool &found) const
{
    Mutex   mutex(_mutex, Mutex::READ, "Port", "getClient");

    found = false;
    if (!mutex)
        return (false);
    for (QListIterator<QSharedPointer<Client> > it(_clients); it.hasNext(); it.next())
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
    Mutex       mutex(_mutex, Mutex::READ, "Port", "getClients");
    QStringList result;

    if (!mutex)
        return (result);
    // Stores the id of the clients in the string list
    for (QListIterator<QSharedPointer<Client> > it(_clients); it.hasNext(); it.next())
        result << it.peekNext()->getId();
    return (result);
}

bool    Port::disconnect(const QString &id, bool fatal)
{
    Mutex   mutex(_mutex, Mutex::READ, "Port", "disconnect");

    if (!mutex)
        return (false);
    // Searches the client that has the given id and disconnects it
    for (QListIterator<QSharedPointer<Client> > it(_clients); it.hasNext(); it.next())
        if (it.peekNext()->getId() == id)
        {
            it.peekNext()->disconnect(fatal);
            return (true);
        }
    return (false);
}

bool    Port::setDisconnectIdle(const QString &id, qint64 msec, bool fatal)
{
    Mutex   mutex(_mutex, Mutex::READ, "Port", "setDisconnectIdle");

    if (!mutex)
        return (false);
    for (QListIterator<QSharedPointer<Client> > it(_clients); it.hasNext(); it.next())
        if (it.peekNext()->getId() == id)
        {
            it.peekNext()->setDisconnectIdle(msec, fatal);
            return (true);
        }
    return (false);
}

bool    Port::getDisconnectIdle(const QString &id, bool *fatal, qint64 &result)
{
    Mutex   mutex(_mutex, Mutex::READ, "Port", "getDisconnectIdle");

    if (!mutex)
        return (false);
    for (QListIterator<QSharedPointer<Client> > it(_clients); it.hasNext(); it.next())
        if (it.peekNext()->getId() == id)
        {
            result = it.peekNext()->getDisconnectIdle(fatal);
            return (true);
        }
    return (false);
}

bool    Port::setDisconnectTime(const QString &id, const QDateTime &time, bool fatal)
{
    Mutex   mutex(_mutex, Mutex::READ, "Port", "setDisconnectTime");

    if (!mutex)
        return (false);
    for (QListIterator<QSharedPointer<Client> > it(_clients); it.hasNext(); it.next())
        if (it.peekNext()->getId() == id)
        {
            it.peekNext()->setDisconnectTime(time, fatal);
            return (true);
        }
    return (false);
}

bool    Port::getDisconnectTime(const QString &id, bool *fatal, QDateTime &result)
{
    Mutex   mutex(_mutex, Mutex::READ, "Port", "getDisconnectTime");

    if (!mutex)
        return (false);
    for (QListIterator<QSharedPointer<Client> > it(_clients); it.hasNext(); it.next())
        if (it.peekNext()->getId() == id)
        {
            result = it.peekNext()->getDisconnectTime(fatal);
            return (true);
        }
    return (false);
}

bool    Port::send(const QString &id, const QString &p, const QVariantMap &informations)
{
    Mutex   mutex(_mutex, Mutex::READ, "Port", "send");
    Client  *client = NULL;
    QString protocol;

    if (!mutex)
        return (false);
    // Searches the client
    for (QListIterator<QSharedPointer<Client> > it(_clients); it.hasNext(); it.next())
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

bool    Port::pause(const QString &id, int msec)
{
    Mutex   mutex(_mutex, Mutex::READ, "Port", "pause");

    if (!mutex)
        return (false);
    for (QListIterator<QSharedPointer<Client> > it(_clients); it.hasNext(); it.next())
        if (it.peekNext()->getId() == id)
            return (it.peekNext()->pause(msec));
    return (false);
}

bool    Port::resume(const QString &id)
{
    Mutex   mutex(_mutex, Mutex::READ, "Port", "resume");

    if (!mutex)
        return (false);
    for (QListIterator<QSharedPointer<Client> > it(_clients); it.hasNext(); it.next())
        if (it.peekNext()->getId() == id)
            return (it.peekNext()->resume());
    return (false);
}

QSharedPointer<Client> Port::_getClient(Client *client)
{
    for (QListIterator<QSharedPointer<Client> > it(_clients); it.hasNext(); it.next())
        if (it.peekNext() == client)
            return it.peekNext();
    return QSharedPointer<Client>(NULL);
}
