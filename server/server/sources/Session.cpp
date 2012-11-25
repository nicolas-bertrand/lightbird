#include "ApiSessions.h"
#include "Database.h"
#include "Defines.h"
#include "LightBird.h"
#include "Session.h"
#include "Mutex.h"

Session::Session(const QString &id) : destroyed(false)
{
    QSqlQuery               query;
    QVector<QVariantMap>    result;
    int                     i;
    int                     s;

    this->tableName = "sessions";
    this->tableId = LightBird::Table::Unknow;
    if (!this->setId(id))
        return ;
    // Get the details of the session
    query.prepare(Database::instance()->getQuery("Sessions", "getSession"));
    query.bindValue(":id", id);
    if (!Database::instance()->query(query, result) || result.size() <= 0)
        return ;
    this->expiration = result[0]["expiration"].toDateTime().toLocalTime();
    this->id_account = result[0]["id_account"].toString();
    this->creation = result[0]["creation"].toDateTime().toLocalTime();
    // Get the informations
    query.prepare(Database::instance()->getQuery("Sessions", "getInformations"));
    query.bindValue(":id_session", id);
    if (Database::instance()->query(query, result))
        for (i = 0, s = result.size(); i < s; ++i)
            this->informations[result[i]["name"].toString()] = result[i]["value"];
}

Session::Session(const QDateTime &e, const QString &a, const QStringList &c, const QVariantMap &informations) :
                 expiration(e), id_account(a), creation(QDateTime::currentDateTime()), clients(c), destroyed(false)
{
    QSqlQuery   query;
    QString     id;

    id = LightBird::createUuid();
    query.prepare(Database::instance()->getQuery("Sessions", "add"));
    query.bindValue(":id", id);
    query.bindValue(":expiration", expiration.toUTC().toString(DATE_FORMAT));
    query.bindValue(":id_account", id_account);
    if (!Database::instance()->query(query) || query.numRowsAffected() == 0)
        return ;
    this->id = id;
    if (!informations.isEmpty())
        this->setInformations(informations);
}

Session::~Session()
{
}

Session::Session(const Session &session) : LightBird::Table()
{
    *this = session;
}

Session &Session::operator=(const Session &session)
{
    if (this != &session)
    {
        LightBird::Table::operator=(session);
        this->expiration = session.expiration;
        this->id_account = session.id_account;
        this->creation = session.creation;
        this->clients = session.clients;
        this->informations = session.informations;
        this->destroyed = session.destroyed;
    }
    return (*this);
}

const QString   &Session::getId() const
{
    return (this->id);
}

QString Session::getAccount() const
{
    Mutex   mutex(this->mutex, Mutex::READ, "Session", "getAccount");

    if (!mutex)
        return (QString());
    return (this->id_account);
}

bool    Session::setAccount(const QString &id_account)
{
    QSqlQuery   query;
    Mutex       mutex(this->mutex, Mutex::WRITE, "Session", "setAccount");

    if (!mutex)
        return (false);
    query.prepare(Database::instance()->getQuery("Sessions", "setAccount"));
    query.bindValue(":id", this->id);
    query.bindValue(":id_account", id_account);
    if (!Database::instance()->query(query) || query.numRowsAffected() <= 0)
        return (false);
    this->id_account = id_account;
    return (true);
}

bool    Session::isExpired() const
{
    Mutex   mutex(this->mutex, Mutex::READ, "Session", "isExpired");

    if (!mutex || destroyed)
        return (true);
    return (this->expiration.isValid() && this->expiration < QDateTime::currentDateTime());

}

QDateTime   Session::getExpiration() const
{
    Mutex   mutex(this->mutex, Mutex::READ, "Session", "getExpiration");

    if (!mutex)
        return (QDateTime());
    return (this->expiration);
}

bool    Session::setExpiration(const QDateTime &expiration)
{
    QSqlQuery   query;
    Mutex       mutex(this->mutex, Mutex::WRITE, "Session", "setExpiration");

    if (!mutex)
        return (false);
    query.prepare(Database::instance()->getQuery("Sessions", "setExpiration"));
    query.bindValue(":id", this->id);
    query.bindValue(":expiration", expiration.toUTC().toString(DATE_FORMAT));
    if (!Database::instance()->query(query) || query.numRowsAffected() <= 0)
        return (false);
    this->expiration = expiration;
    ApiSessions::instance()->expiration();
    return (true);
}

QDateTime   Session::getCreation() const
{
    Mutex   mutex(this->mutex, Mutex::READ, "Session", "getCreation");

    if (!mutex)
        return (QDateTime());
    return (this->creation);
}

QStringList Session::getClients() const
{
    Mutex   mutex(this->mutex, Mutex::READ, "Session", "getClients");

    if (!mutex)
        return (QStringList());
    return (this->clients);
}

bool    Session::setClient(const QString &client)
{
    Mutex   mutex(this->mutex, Mutex::WRITE, "Session", "setClient");

    if (!mutex)
        return (false);
    if (!this->clients.contains(client))
        this->clients << client;
    return (true);
}

bool    Session::setClients(const QStringList &clients)
{
    Mutex   mutex(this->mutex, Mutex::WRITE, "Session", "setClients");

    if (!mutex)
        return (false);
    this->clients << clients;
    this->clients.removeDuplicates();
    return (true);
}

bool    Session::removeClient(const QString &client)
{
    Mutex   mutex(this->mutex, Mutex::WRITE, "Session", "removeClient");

    if (!mutex)
        return (false);
    this->clients.removeAll(client);
    return (true);
}

bool    Session::removeClients(const QStringList &clients)
{
    Mutex   mutex(this->mutex, Mutex::WRITE, "Session", "removeClients");

    if (!mutex)
        return (false);
    if (clients.isEmpty())
        this->clients.clear();
    else
    {
        QStringListIterator it(clients);
        while (it.hasNext())
            this->clients.removeAll(it.next());
    }
    return (true);
}

bool    Session::hasInformation(const QString &name) const
{
    Mutex   mutex(this->mutex, Mutex::READ, "Session", "hasInformation");

    if (!mutex)
        return (false);
    return (this->informations.contains(name));
}

QVariant    Session::getInformation(const QString &name) const
{
    Mutex   mutex(this->mutex, Mutex::READ, "Session", "getInformation");

    if (!mutex)
        return (QVariant());
    return (this->informations.value(name));
}

QVariantMap Session::getInformations() const
{
    Mutex   mutex(this->mutex, Mutex::READ, "Session", "getInformations");

    if (!mutex)
        return (QVariantMap());
    return (this->informations);
}

bool    Session::setInformation(const QString &name, const QVariant &value)
{
    QSqlQuery   query;
    Mutex       mutex(this->mutex, Mutex::WRITE, "Session", "setInformation");

    if (!mutex)
        return (false);
    if (this->informations.contains(name))
    {
        query.prepare(Database::instance()->getQuery("Sessions", "setInformation_update"));
        query.bindValue(":value", value);
        query.bindValue(":id_session", this->id);
        query.bindValue(":name", name);
    }
    else
    {
        query.prepare(Database::instance()->getQuery("Sessions", "setInformation_insert"));
        query.bindValue(":id", LightBird::createUuid());
        query.bindValue(":id_session", this->id);
        query.bindValue(":name", name);
        query.bindValue(":value", value);
    }
    if (!Database::instance()->query(query) || query.numRowsAffected() <= 0)
        return (false);
    this->informations[name] = value;
    return (true);
}

bool    Session::setInformations(const QVariantMap &informations)
{
    bool    result = true;

    QMapIterator<QString, QVariant> it(informations);
    while (it.hasNext() && result)
    {
        it.next();
        result = this->setInformation(it.key(), it.value());
    }
    return (result);
}

bool    Session::removeInformation(const QString &name)
{
    QSqlQuery   query;
    Mutex       mutex(this->mutex, Mutex::WRITE, "Session", "removeInformation");

    if (!mutex)
        return (false);
    query.prepare(Database::instance()->getQuery("Sessions", "removeInformation"));
    query.bindValue(":id_session", this->id);
    query.bindValue(":name", name);
    if (!Database::instance()->query(query) || query.numRowsAffected() <= 0)
        return (false);
    this->informations.remove(name);
    return (true);
}

bool    Session::removeInformations(const QStringList &informations)
{
    QSqlQuery   query;
    bool        result = true;

    if (informations.isEmpty())
    {
        Mutex mutex(this->mutex, Mutex::WRITE, "Session", "removeInformations");
        if (!mutex)
            return (false);
        query.prepare(Database::instance()->getQuery("Sessions", "removeInformations"));
        query.bindValue(":id_session", this->id);
        result = Database::instance()->query(query);
        this->informations.clear();
    }
    else
    {
        QStringListIterator it(informations);
        while (it.hasNext() && result)
            result = this->removeInformation(it.next());
    }
    return (result);
}

bool    Session::destroy(bool disconnect)
{
    return (ApiSessions::instance()->destroy(this->id, disconnect));
}

bool    Session::remove()
{
    QSqlQuery   query;
    Mutex       mutex(this->mutex, Mutex::WRITE, "Session", "destroy");

    if (!mutex)
        return (false);
    query.prepare(Database::instance()->getQuery("Sessions", "delete"));
    query.bindValue(":id", this->id);
    if (Database::instance()->query(query) && query.numRowsAffected() == 1)
        this->destroyed = true;
    return (this->destroyed);
}
