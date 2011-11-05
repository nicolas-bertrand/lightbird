#include "ApiSessions.h"
#include "Database.h"
#include "Defines.h"
#include "Session.h"
#include "SmartMutex.h"
#include "Tools.h"

Session::Session(const QString &id) : destroyed(false)
{
    QVector<QMap<QString, QVariant> >   result;
    QSqlQuery                           query;
    int                                 i;
    int                                 s;

    this->tableName = "sessions";
    this->tableId = LightBird::ITable::Unknow;
    if (!this->setId(id))
        return ;
    // Get the details of the session
    query.prepare(Database::instance()->getQuery("Sessions", "getSession"));
    query.bindValue(":id", id);
    if (!Database::instance()->query(query, result) || query.numRowsAffected() == 0)
        return ;
    this->expiration = result[0]["expiration"].toDateTime();
    this->id_account = result[0]["id_account"].toString();
    this->creation = result[0]["creation"].toDateTime();
    // Get the informations
    query.prepare(Database::instance()->getQuery("Sessions", "getInformations"));
    query.bindValue(":id_session", id);
    if (Database::instance()->query(query, result))
        for (i = 0, s = result.size(); i < s; ++i)
            this->informations[result[i]["name"].toString()] = result[i]["value"];
}

Session::Session(const QDateTime &e, const QString &a, const QStringList &c, const QVariantMap &i) :
                 expiration(e), id_account(a), creation(QDateTime::currentDateTime()), clients(c), informations(i), destroyed(false)
{
    QSqlQuery   query;
    QString     id;

    id = Tools::createUuid();
    query.prepare(Database::instance()->getQuery("Sessions", "add"));
    query.bindValue(":id", id);
    query.bindValue(":expiration", expiration.toString(DATE_FORMAT));
    query.bindValue(":id_account", id_account);
    if (!Database::instance()->query(query) || query.numRowsAffected() == 0)
        return ;
    this->id = id;
    if (!this->informations.isEmpty())
        this->setInformations(this->informations);
}

Session::~Session()
{
}

Session::Session(const Session &session)
{
    *this = session;
}

Session &Session::operator=(const Session &session)
{
    if (this != &session)
    {
        Table::operator=(session);
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

QString         Session::getAccount() const
{
    SmartMutex  mutex(this->mutex, SmartMutex::READ, "Session", "getAccount");

    if (!mutex)
        return (QString());
    return (this->id_account);
}

bool            Session::setAccount(const QString &id_account)
{
    QSqlQuery   query;
    SmartMutex  mutex(this->mutex, SmartMutex::WRITE, "Session", "setAccount");

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

bool            Session::isExpired() const
{
    SmartMutex  mutex(this->mutex, SmartMutex::READ, "Session", "isExpired");

    if (!mutex || destroyed)
        return (true);
    return (this->expiration < QDateTime::currentDateTime());

}

QDateTime       Session::getExpiration() const
{
    SmartMutex  mutex(this->mutex, SmartMutex::READ, "Session", "getExpiration");

    if (!mutex)
        return (QDateTime());
    return (this->expiration);
}

bool            Session::setExpiration(const QDateTime &expiration)
{
    QSqlQuery   query;
    SmartMutex  mutex(this->mutex, SmartMutex::WRITE, "Session", "setExpiration");

    if (!mutex)
        return (false);
    query.prepare(Database::instance()->getQuery("Sessions", "setExpiration"));
    query.bindValue(":id", this->id);
    query.bindValue(":expiration", expiration.toString(DATE_FORMAT));
    if (!Database::instance()->query(query) || query.numRowsAffected() <= 0)
        return (false);
    this->expiration = expiration;
    ApiSessions::instance()->expiration();
    return (true);
}

QDateTime       Session::getCreation() const
{
    SmartMutex  mutex(this->mutex, SmartMutex::READ, "Session", "getCreation");

    if (!mutex)
        return (QDateTime());
    return (this->creation);
}

QStringList     Session::getClients() const
{
    SmartMutex  mutex(this->mutex, SmartMutex::READ, "Session", "getClients");

    if (!mutex)
        return (QStringList());
    return (this->clients);
}

bool            Session::setClient(const QString &client)
{
    SmartMutex  mutex(this->mutex, SmartMutex::WRITE, "Session", "setClient");

    if (!mutex)
        return (false);
    if (!this->clients.contains(client))
        this->clients << client;
    return (true);
}

bool            Session::setClients(const QStringList &clients)
{
    SmartMutex  mutex(this->mutex, SmartMutex::WRITE, "Session", "setClients");

    if (!mutex)
        return (false);
    this->clients << clients;
    this->clients.removeDuplicates();
    return (true);
}

bool    Session::removeClient(const QString &client)
{
    SmartMutex  mutex(this->mutex, SmartMutex::WRITE, "Session", "removeClient");

    if (!mutex)
        return (false);
    this->clients.removeAll(client);
    return (true);
}

bool    Session::removeClients(const QStringList &clients)
{
    SmartMutex  mutex(this->mutex, SmartMutex::WRITE, "Session", "removeClients");

    if (!mutex)
        return (false);
    QStringListIterator it(clients);
    while (it.hasNext())
        this->clients.removeAll(it.next());
    return (true);
}

QVariant        Session::getInformation(const QString &name) const
{
    SmartMutex  mutex(this->mutex, SmartMutex::READ, "Session", "getInformation");

    if (!mutex)
        return (QVariant());
    return (this->informations.value(name));
}

QMap<QString, QVariant> Session::getInformations() const
{
    SmartMutex  mutex(this->mutex, SmartMutex::READ, "Session", "getInformations");

    if (!mutex)
        return (QMap<QString, QVariant>());
    return (this->informations);
}

bool            Session::setInformation(const QString &name, const QVariant &value)
{
    QSqlQuery   query;
    SmartMutex  mutex(this->mutex, SmartMutex::WRITE, "Session", "setInformation");

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
        query.bindValue(":id", Tools::createUuid());
        query.bindValue(":id_session", this->id);
        query.bindValue(":name", name);
        query.bindValue(":value", value);
    }
    if (!Database::instance()->query(query) || query.numRowsAffected() <= 0)
        return (false);
    this->informations[name] = value;
    return (true);
}

bool            Session::setInformations(const QMap<QString, QVariant> &informations)
{
    bool        result = true;
    SmartMutex  mutex(this->mutex, SmartMutex::WRITE, "Session", "setInformations");

    if (!mutex)
        return (false);
    QMapIterator<QString, QVariant> it(informations);
    while (it.hasNext() && result)
    {
        it.next();
        result = this->setInformation(it.key(), it.value());
    }
    return (result);
}

bool            Session::removeInformation(const QString &name)
{
    QSqlQuery   query;
    SmartMutex  mutex(this->mutex, SmartMutex::WRITE, "Session", "removeInformation");

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

bool            Session::removeInformations(const QStringList &informations)
{
    bool        result = true;
    SmartMutex  mutex(this->mutex, SmartMutex::WRITE, "Session", "removeInformations");

    if (!mutex)
        return (false);
    QStringListIterator it(informations);
    while (it.hasNext() && result)
        result = this->removeInformation(it.next());
    return (result);
}

bool            Session::destroy()
{
    QSqlQuery   query;
    SmartMutex  mutex(this->mutex, SmartMutex::WRITE, "Session", "destroy");

    if (!mutex)
        return (false);
    query.prepare(Database::instance()->getQuery("Sessions", "delete"));
    query.bindValue(":id", this->id);
    if (Database::instance()->query(query) && query.numRowsAffected() == 1)
        this->destroyed = true;
    return (this->destroyed);
}
