#include <QCryptographicHash>

#include "Database.h"
#include "Sha256.h"
#include "TableAccounts.h"
#include "Tools.h"

TableAccounts::TableAccounts(const QString &id)
{
    this->tableName = "accounts";
    this->tableId = LightBird::ITable::Accounts;
    this->setId(id);
}

TableAccounts::~TableAccounts()
{
}

TableAccounts::TableAccounts(const TableAccounts &t) : Table(), TableAccessors()
{
    *this = t;
}

TableAccounts &TableAccounts::operator=(const TableAccounts &table)
{
    if (this != &table)
        TableAccessors::operator=(table);
    return (*this);
}

bool        TableAccounts::setId(const QString &id)
{
    if (!Table::setId(id))
        return (false);
    return (true);
}

QString     TableAccounts::getIdFromName(const QString &name) const
{
    QSqlQuery               query;
    QVector<QVariantMap>    result;

    query.prepare(Database::instance()->getQuery("TableAccounts", "getIdFromName"));
    query.bindValue(":name", name);
    if (!Database::instance()->query(query, result) || result.size() != 1)
        return ("");
    return (result[0]["id"].toString());
}

bool        TableAccounts::setIdFromName(const QString &name)
{
    QString id;

    if ((id = this->getIdFromName(name)).isEmpty())
        return (false);
    this->id = id;
    return (true);
}

QString     TableAccounts::getIdFromNameAndPassword(const QString &name, const QString &password) const
{
    TableAccounts   account;

    if (!account.setIdFromName(name) || this->passwordHash(password, account.getId()) != account.getPassword())
        return ("");
    return (account.getId());
}

bool        TableAccounts::setIdFromNameAndPassword(const QString &name, const QString &password)
{
    QString id;

    if ((id = this->getIdFromNameAndPassword(name, password)).isEmpty())
        return (false);
    this->id = id;
    return (true);
}

QString     TableAccounts::getIdFromIdentifiantAndSalt(const QString &identifiant, const QString &salt) const
{
    QSqlQuery               query;
    QVector<QVariantMap>    result;
    int                     i;
    int                     s;

    query.prepare(Database::instance()->getQuery("TableAccounts", "getIdFromIdentifiantAndSalt"));
    if (Database::instance()->query(query, result))
        // Check the identifiant of all the account
        for (i = 0, s = result.size(); i < s; ++i)
            // If the identifiant of the account match, it is connected
            if (identifiant == Sha256::hash(result[i]["name"].toByteArray() + result[i]["password"].toByteArray() + salt.toAscii()))
                return (result[i]["id"].toString());
    return ("");
}

bool        TableAccounts::setIdFromIdentifiantAndSalt(const QString &identifiant, const QString &salt)
{
    QString id;

    if ((id = this->getIdFromIdentifiantAndSalt(identifiant, salt)).isEmpty())
        return (false);
    this->id = id;
    return (true);
}

bool            TableAccounts::add(const QString &name, const QVariantMap &informations,
                                   const QString &password, bool administrator, bool active)
{
    QSqlQuery   query;
    QString     id;

    id = Tools::createUuid();
    query.prepare(Database::instance()->getQuery("TableAccounts", "add"));
    query.bindValue(":id", id);
    query.bindValue(":name", name);
    query.bindValue(":password", this->passwordHash(password, id));
    query.bindValue(":administrator", QString::number(administrator));
    query.bindValue(":active", QString::number(active));
    if (!Database::instance()->query(query) || query.numRowsAffected() == 0)
        return (false);
    this->id = id;
    if (!informations.isEmpty())
        this->setInformations(informations);
    return (true);
}

bool        TableAccounts::add(const QString &name, const QString &password, bool administrator, bool active)
{
    return (this->add(name, QVariantMap(), password, administrator, active));
}

QString         TableAccounts::getPassword() const
{
    QSqlQuery               query;
    QVector<QVariantMap>    result;

    query.prepare(Database::instance()->getQuery("TableAccounts", "getPassword"));
    query.bindValue(":id", this->id);
    if (Database::instance()->query(query, result) && result.size() > 0)
        return (result[0]["password"].toString());
    return ("");
}

bool            TableAccounts::setPassword(const QString &password)
{
    QSqlQuery   query;

    query.prepare(Database::instance()->getQuery("TableAccounts", "setPassword"));
    query.bindValue(":password", this->passwordHash(password, this->id));
    query.bindValue(":id", this->id);
    return (Database::instance()->query(query));
}

bool            TableAccounts::isAdministrator() const
{
    QSqlQuery               query;
    QVector<QVariantMap>    result;

    query.prepare(Database::instance()->getQuery("TableAccounts", "isAdministrator_select"));
    query.bindValue(":id", this->id);
    if (Database::instance()->query(query, result) && result.size() > 0 && result[0]["administrator"] == "1")
        return (true);
    return (false);
}

bool            TableAccounts::isAdministrator(bool administrator)
{
    QSqlQuery   query;

    query.prepare(Database::instance()->getQuery("TableAccounts", "isAdministrator_update"));
    query.bindValue(":administrator", QString::number(administrator));
    query.bindValue(":id", this->id);
    return (Database::instance()->query(query));
}

bool            TableAccounts::isActive() const
{
    QSqlQuery               query;
    QVector<QVariantMap>    result;

    query.prepare(Database::instance()->getQuery("TableAccounts", "isActive_select"));
    query.bindValue(":id", this->id);
    if (Database::instance()->query(query, result) && result.size() > 0 && result[0]["active"] == "1")
        return (true);
    return (false);
}

bool            TableAccounts::isActive(bool active)
{
    QSqlQuery   query;

    query.prepare(Database::instance()->getQuery("TableAccounts", "isActive_update"));
    query.bindValue(":active", QString::number(active));
    query.bindValue(":id", this->id);
    return (Database::instance()->query(query));
}

QVariant        TableAccounts::getInformation(const QString &name) const
{
    QSqlQuery               query;
    QVector<QVariantMap>    result;

    query.prepare(Database::instance()->getQuery("TableAccounts", "getInformation"));
    query.bindValue(":id_account", this->id);
    query.bindValue(":name", name);
    if (Database::instance()->query(query, result) && result.size() > 0)
        return (result.value(0).value("value").toString());
    return ("");
}

QVariantMap     TableAccounts::getInformations() const
{
    QSqlQuery               query;
    QVector<QVariantMap>    result;
    QVariantMap             informations;
    int                     i;
    int                     s;

    query.prepare(Database::instance()->getQuery("TableAccounts", "getInformations"));
    query.bindValue(":id_account", this->id);
    if (Database::instance()->query(query, result))
        for (i = 0, s = result.size(); i < s; ++i)
            informations.insert(result[i]["name"].toString(), result[i]["value"]);
    return (informations);
}

bool            TableAccounts::setInformation(const QString &name, const QVariant &value)
{
    QSqlQuery               query;
    QVector<QVariantMap>    result;

    query.prepare(Database::instance()->getQuery("TableAccounts", "setInformation_select"));
    query.bindValue(":id_account", this->id);
    query.bindValue(":name", name);
    Database::instance()->query(query, result);
    if (result.size() > 0)
    {
        query.prepare(Database::instance()->getQuery("TableAccounts", "setInformation_update"));
        query.bindValue(":value", value);
        query.bindValue(":id_account", this->id);
        query.bindValue(":name", name);
    }
    else
    {
        query.prepare(Database::instance()->getQuery("TableAccounts", "setInformation_insert"));
        query.bindValue(":id", Tools::createUuid());
        query.bindValue(":id_account", this->id);
        query.bindValue(":name", name);
        query.bindValue(":value", value);
    }
    return (Database::instance()->query(query));
}

bool            TableAccounts::setInformations(const QVariantMap &informations)
{
    QMapIterator<QString, QVariant> it(informations);
    bool                            result = true;

    while (it.hasNext() && result)
    {
        it.next();
        if (!this->setInformation(it.key(), it.value()))
            result = false;
    }
    return (result);
}

bool            TableAccounts::removeInformation(const QString &name)
{
    QSqlQuery   query;

    query.prepare(Database::instance()->getQuery("TableAccounts", "removeInformation"));
    query.bindValue(":id_account", this->id);
    query.bindValue(":name", name);
    return (Database::instance()->query(query) && query.numRowsAffected() > 0);
}

bool            TableAccounts::removeInformations(const QStringList &informations)
{
    QSqlQuery   query;
    bool        result = true;

    if (informations.isEmpty())
    {
        query.prepare(Database::instance()->getQuery("TableAccounts", "removeInformations"));
        query.bindValue(":id_account", this->id);
        result = Database::instance()->query(query);
    }
    else
    {
        QStringListIterator it(informations);
        while (it.hasNext() && result)
            result = this->removeInformation(it.next());
    }
    return (result);
}

QStringList     TableAccounts::getGroups() const
{
    QSqlQuery               query;
    QVector<QVariantMap>    result;
    QStringList             groups;
    int                     i;
    int                     s;

    query.prepare(Database::instance()->getQuery("TableAccounts", "getGroups"));
    query.bindValue(":id_account", this->id);
    if (Database::instance()->query(query, result))
        for (i = 0, s = result.size(); i < s; ++i)
            groups << result[i]["id_group"].toString();
    return (groups);
}

bool            TableAccounts::addGroup(const QString &id_group)
{
    QSqlQuery   query;
    QString     id;

    id = Tools::createUuid();
    query.prepare(Database::instance()->getQuery("TableAccounts", "addGroup"));
    query.bindValue(":id", id);
    query.bindValue(":id_account", this->id);
    query.bindValue(":id_group", id_group);
    return (Database::instance()->query(query));
}

bool            TableAccounts::removeGroup(const QString &id_group)
{
    QSqlQuery   query;

    query.prepare(Database::instance()->getQuery("TableAccounts", "removeGroup"));
    query.bindValue(":id_account", this->id);
    query.bindValue(":id_group", id_group);
    return (Database::instance()->query(query) && query.numRowsAffected() > 0);
}

QString         TableAccounts::passwordHash(const QString &password, const QString &id) const
{
    if (password.isEmpty())
        return (QString());
    if (id.isEmpty())
        return (Sha256::hash(password.toAscii()));
    return (Sha256::hash(password.toAscii() + id.toAscii()));
}
