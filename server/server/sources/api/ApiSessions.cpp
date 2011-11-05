#include "ApiSessions.h"
#include "Database.h"
#include "Defines.h"
#include "Events.h"
#include "Log.h"
#include "Network.h"
#include "Server.h"
#include "Session.h"
#include "SmartMutex.h"

ApiSessions::ApiSessions(QObject *parent) : QObject(parent)
{
    QSqlQuery   query;

    QObject::connect(&this->timer, SIGNAL(timeout()), this, SLOT(expiration()), Qt::QueuedConnection);
    QObject::connect(this, SIGNAL(expirationSignal()), this, SLOT(expiration()), Qt::QueuedConnection);
    this->timer.setSingleShot(true);
    // Delete the expired sessions
    query.prepare(Database::instance()->getQuery("Sessions", "deleteExpiredSessions"));
    query.bindValue(":expiration", QDateTime::currentDateTime().toString(DATE_FORMAT));
    Database::instance()->query(query);
    this->expiration();
    Log::trace("ApiSessions created", "ApiSessions", "ApiSessions");
}

ApiSessions::~ApiSessions()
{
    Log::trace("ApiSessions destroyed!", "ApiSessions", "~ApiSessions");
}

QString         ApiSessions::create(const QDateTime &expiration, const QString &id_account, const QStringList &clients, const QVariantMap &informations)
{
    SmartMutex  mutex(this->mutex, "ApiSessions", "create");
    Session     *session;

    if (!mutex)
        return (QString());
    // Create the session
    session = new Session(expiration, id_account, clients, informations);
    // Add it in the cache
    this->sessions[session->getId()] = QSharedPointer<LightBird::ISession>(session);
    this->expiration();
    return (session->getId());
}

bool            ApiSessions::destroy(const QString &id, bool disconnect)
{
    QSharedPointer<LightBird::ISession> session(this->getSession(id));
    SmartMutex  mutex(this->mutex, "ApiSessions", "destroy");

    if (!mutex || session.isNull())
        return (false);
    // Disconnect the clients associated with the session
    if (disconnect)
    {
        QStringListIterator it(session->getClients());
        while (it.hasNext())
            Network::instance()->disconnect(it.next());
    }
    // Delete the session from the database
    dynamic_cast<Session *>(session.data())->destroy();
    // Removes the session from the cache
    this->sessions.remove(id);
    Events::instance()->send("session_destroyed", id);
    return (session->isExpired());
}

QStringList     ApiSessions::getSessions(const QString &id_account, const QString &client) const
{
    QSqlQuery               query;
    QVector<QVariantMap>    result;
    int                     i;
    int                     s;
    QStringList             sessions;
    SmartMutex  mutex(this->mutex, "ApiSessions", "getSessions");

    if (!mutex)
        return (QStringList());
    // Get the sessions that match the account
    query.prepare(Database::instance()->getQuery("Sessions", "getSessions"));
    query.bindValue(":id_account", id_account);
    if (Database::instance()->query(query, result))
        for (i = 0, s = result.size(); i < s; ++i)
            // Get the sessions that are associated with the client
            if (client.isEmpty() || (this->sessions.contains(result[i]["id"].toString()) &&
                                     this->sessions[result[i]["id"].toString()]->getClients().contains(client)))
                sessions << result[i]["id"].toString();
    return (sessions);
}

QSharedPointer<LightBird::ISession> ApiSessions::getSession(const QString &id)
{
    SmartMutex  mutex(this->mutex, "ApiSessions", "destroy");
    QSharedPointer<LightBird::ISession> result;

    if (!mutex)
        return (result);
    // Get the session from the cache
    if ((result = this->sessions.value(id)).isNull())
    {
        // The session is not in the cache, so we get it from the database
        QSharedPointer<LightBird::ISession> session(new Session(id));
        // Then we store it in the cache if it has been found
        if (dynamic_cast<Session *>(session.data())->exists())
            result = this->sessions[id] = session;
    }
    return (result);
}

void                        ApiSessions::expiration()
{
    QSqlQuery               query;
    QVector<QVariantMap>    result;
    int                     i;
    int                     s;

    // If the current thread is not the main thread (where the sessions live),
    // we emit a signal that will call expiration in the correct thread.
    // This is done because the timer must be used in the thread in which it lives.
    if (QThread::currentThread() != this->thread())
    {
        emit this->expirationSignal();
        return ;
    }
    // Get the list of the expired sessions in order to destroy them
    query.prepare(Database::instance()->getQuery("Sessions", "getExpiredSessions"));
    query.bindValue(":expiration", QDateTime::currentDateTime().addSecs(2).toString(DATE_FORMAT));
    if (Database::instance()->query(query, result))
        for (i = 0, s = result.size(); i < s; ++i)
            this->destroy(result[i]["id"].toString());
    // Search the next session that is going to expire
    query.prepare(Database::instance()->getQuery("Sessions", "getNextExpiration"));
    SmartMutex mutex(this->mutex, "ApiSessions", "expiration");
    // The timer will call expiration the next time a session expire
    if (mutex && Database::instance()->query(query, result) && result.size() > 0)
    {
        if ((s = result[0]["expiration"].toDateTime().toMSecsSinceEpoch() - QDateTime::currentDateTime().toMSecsSinceEpoch()) < 0)
            s = 0;
        this->timer.start(s);
    }
    else
        this->timer.stop();
}

ApiSessions *ApiSessions::instance()
{
    return (Server::instance().getApiSessions());
}
