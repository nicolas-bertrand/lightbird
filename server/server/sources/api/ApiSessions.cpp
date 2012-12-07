#include "ApiSessions.h"
#include "Database.h"
#include "Defines.h"
#include "Events.h"
#include "LightBird.h"
#include "Log.h"
#include "Network.h"
#include "Server.h"
#include "Session.h"
#include "Mutex.h"

ApiSessions::ApiSessions(QObject *parent)
    : QObject(parent)
{
    QSqlQuery   query;

    QObject::connect(&this->timer, SIGNAL(timeout()), this, SLOT(expiration()), Qt::QueuedConnection);
    QObject::connect(this, SIGNAL(expirationSignal()), this, SLOT(expiration()), Qt::QueuedConnection);
    this->timer.setSingleShot(true);
    // Delete the expired sessions
    query.prepare(Database::instance()->getQuery("Sessions", "deleteExpiredSessions"));
    query.bindValue(":expiration", QDateTime::currentDateTimeUtc().toString(DATE_FORMAT));
    Database::instance()->query(query);
    this->expiration();
    LOG_TRACE("ApiSessions created", "ApiSessions", "ApiSessions");
}

ApiSessions::~ApiSessions()
{
    LOG_TRACE("ApiSessions destroyed!", "ApiSessions", "~ApiSessions");
}

LightBird::Session  ApiSessions::create(const QDateTime &expiration, const QString &id_account, const QStringList &clients, const QVariantMap &informations)
{
    Mutex   mutex(this->mutex, "ApiSessions", "create");
    Session *session;

    if (!mutex)
        return (QSharedPointer<LightBird::ISession>());
    // Create the session
    session = new Session(expiration, id_account, clients, informations);
    // Add it in the cache
    this->sessions[session->getId()] = QSharedPointer<LightBird::ISession>(session);
    this->expiration();
    return (this->sessions.value(session->getId()));
}

bool    ApiSessions::destroy(const QString &id, bool disconnect)
{
    QSharedPointer<LightBird::ISession> session(this->getSession(id));
    Mutex   mutex(this->mutex, "ApiSessions", "destroy");

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
    dynamic_cast<Session *>(session.data())->remove();
    // Removes the session from the cache
    this->sessions.remove(id);
    Events::instance()->send("session_destroyed", id);
    return (session->isExpired());
}

QStringList ApiSessions::getSessions(const QString &id_account, const QString &client) const
{
    QSqlQuery            query;
    QVector<QVariantMap> result;
    int                  i;
    int                  s;
    QStringList          sessions;
    Mutex   mutex(this->mutex, "ApiSessions", "getSessions");

    if (!mutex)
        return (QStringList());
    // If a specific client is wanted, the session is in the cache
    if (!client.isEmpty())
    {
        QMapIterator<QString, LightBird::Session> it(this->sessions);
        while (it.hasNext())
        {
            it.next();
            if (it.value()->getClients().contains(client) &&
                (id_account.isEmpty() || id_account == it.value()->getAccount()))
                sessions << it.key();
        }
    }
    // Otherwise it should be in the database
    else
    {
        // Get the sessions that match the account
        query.prepare(Database::instance()->getQuery("Sessions", "getSessions"));
        query.bindValue(":id_account", id_account);
        query.bindValue(":ignore_account", id_account.isEmpty());
        if (Database::instance()->query(query, result))
            for (i = 0, s = result.size(); i < s; ++i)
                sessions << result[i]["id"].toString();
    }
    return (sessions);
}

LightBird::Session  ApiSessions::getSession(const QString &id)
{
    Mutex   mutex(this->mutex, "ApiSessions", "destroy");
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

bool    ApiSessions::exists(const QString &id)
{
    return (!this->getSession(id).isNull() && !this->getSession(id)->isExpired());
}

void    ApiSessions::expiration()
{
    QSqlQuery            query;
    QVector<QVariantMap> result;
    int                  i;
    qint64               s;

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
    query.bindValue(":expiration", QDateTime::currentDateTimeUtc().addSecs(2).toString(DATE_FORMAT));
    if (Database::instance()->query(query, result))
        for (i = 0, s = result.size(); i < s; ++i)
            this->destroy(result[i]["id"].toString());
    // Search the next session that is going to expire
    query.prepare(Database::instance()->getQuery("Sessions", "getNextExpiration"));
    Mutex mutex(this->mutex, "ApiSessions", "expiration");
    // The timer will call expiration the next time a session expire
    if (mutex && Database::instance()->query(query, result) && result.size() > 0)
    {
        s = result[0]["expiration"].toDateTime().toMSecsSinceEpoch() - LightBird::currentMSecsSinceEpochUtc();
        // Avoid a possible int overflow
        s = (s < 0) ? 0 : (s > 2000000000) ? 2000000000 : s;
        this->timer.start(s);
    }
    else
        this->timer.stop();
}

ApiSessions *ApiSessions::instance()
{
    return (Server::instance().getApiSessions());
}
