#include "Library.h"
#include "LightBird.h"

using namespace LightBird;

TableAccounts::TableAccounts(const QString &id)
{
    this->tableName = "accounts";
    this->tableId = Table::Accounts;
    this->setId(id);
}

TableAccounts::~TableAccounts()
{
}

TableAccounts::TableAccounts(const TableAccounts &t)
    : TableAccessors()
{
    *this = t;
}

TableAccounts &TableAccounts::operator=(const TableAccounts &table)
{
    if (this != &table)
        TableAccessors::operator=(table);
    return (*this);
}

QString TableAccounts::getIdFromName(const QString &name) const
{
    QSqlQuery            query(Library::database().getDatabase());
    QVector<QVariantMap> result;

    query.prepare(Library::database().getQuery("TableAccounts", "getIdFromName"));
    query.bindValue(":name", name);
    if (!Library::database().query(query, result) || result.size() != 1)
        return ("");
    return (result[0]["id"].toString());
}

bool    TableAccounts::setIdFromName(const QString &name)
{
    QString id;

    if ((id = this->getIdFromName(name)).isEmpty())
        return (false);
    this->id = id;
    return (true);
}

QString TableAccounts::getIdFromNameAndPassword(const QString &name, const QString &password) const
{
    TableAccounts   account;

    if (!account.setIdFromName(name) || this->passwordHash(password, account.getId()) != account.getPassword())
        return ("");
    return (account.getId());
}

bool    TableAccounts::setIdFromNameAndPassword(const QString &name, const QString &password)
{
    QString id;

    if ((id = this->getIdFromNameAndPassword(name, password)).isEmpty())
        return (false);
    this->id = id;
    return (true);
}

QString TableAccounts::getIdFromIdentifiantAndSalt(const QString &identifiant, const QString &salt) const
{
    QSqlQuery            query(Library::database().getDatabase());
    QVector<QVariantMap> result;
    int                  i;
    int                  s;

    query.prepare(Library::database().getQuery("TableAccounts", "getIdFromIdentifiantAndSalt"));
    if (Library::database().query(query, result))
        // Check the identifiant of all the account
        for (i = 0, s = result.size(); i < s; ++i)
            // If the identifiant of the account match, it is connected
            if (identifiant == sha256(result[i]["name"].toByteArray() + result[i]["password"].toByteArray() + salt.toLatin1()))
                return (result[i]["id"].toString());
    return ("");
}

bool    TableAccounts::setIdFromIdentifiantAndSalt(const QString &identifiant, const QString &salt)
{
    QString id;

    if ((id = this->getIdFromIdentifiantAndSalt(identifiant, salt)).isEmpty())
        return (false);
    this->id = id;
    return (true);
}

bool    TableAccounts::add(const QString &name, const QVariantMap &informations,
                                              const QString &password, bool administrator, bool active)
{
    QSqlQuery   query(Library::database().getDatabase());
    QString     id;

    id = createUuid();
    query.prepare(Library::database().getQuery("TableAccounts", "add"));
    query.bindValue(":id", id);
    query.bindValue(":name", name);
    query.bindValue(":password", this->passwordHash(password, id));
    query.bindValue(":administrator", QString::number(administrator));
    query.bindValue(":active", QString::number(active));
    if (!Library::database().query(query) || query.numRowsAffected() == 0)
        return (false);
    this->id = id;
    if (!informations.isEmpty())
        this->setInformations(informations);
    return (true);
}

bool    TableAccounts::add(const QString &name, const QString &password, bool administrator, bool active)
{
    return (this->add(name, QVariantMap(), password, administrator, active));
}

QString TableAccounts::getPassword() const
{
    QSqlQuery            query(Library::database().getDatabase());
    QVector<QVariantMap> result;

    query.prepare(Library::database().getQuery("TableAccounts", "getPassword"));
    query.bindValue(":id", this->id);
    if (Library::database().query(query, result) && result.size() > 0)
        return (result[0]["password"].toString());
    return ("");
}

bool    TableAccounts::setPassword(const QString &password)
{
    QSqlQuery   query(Library::database().getDatabase());

    query.prepare(Library::database().getQuery("TableAccounts", "setPassword"));
    query.bindValue(":password", this->passwordHash(password, this->id));
    query.bindValue(":id", this->id);
    return (Library::database().query(query));
}

bool    TableAccounts::isAdministrator() const
{
    QSqlQuery            query(Library::database().getDatabase());
    QVector<QVariantMap> result;

    query.prepare(Library::database().getQuery("TableAccounts", "isAdministrator_select"));
    query.bindValue(":id", this->id);
    if (Library::database().query(query, result) && result.size() > 0 && result[0]["administrator"] == "1")
        return (true);
    return (false);
}

bool    TableAccounts::isAdministrator(bool administrator)
{
    QSqlQuery   query(Library::database().getDatabase());

    query.prepare(Library::database().getQuery("TableAccounts", "isAdministrator_update"));
    query.bindValue(":administrator", QString::number(administrator));
    query.bindValue(":id", this->id);
    return (Library::database().query(query));
}

bool    TableAccounts::isActive() const
{
    QSqlQuery            query(Library::database().getDatabase());
    QVector<QVariantMap> result;

    query.prepare(Library::database().getQuery("TableAccounts", "isActive_select"));
    query.bindValue(":id", this->id);
    if (Library::database().query(query, result) && result.size() > 0 && result[0]["active"] == "1")
        return (true);
    return (false);
}

bool    TableAccounts::isActive(bool active)
{
    QSqlQuery   query(Library::database().getDatabase());

    query.prepare(Library::database().getQuery("TableAccounts", "isActive_update"));
    query.bindValue(":active", QString::number(active));
    query.bindValue(":id", this->id);
    return (Library::database().query(query));
}

QVariant    TableAccounts::getInformation(const QString &name) const
{
    QSqlQuery            query(Library::database().getDatabase());
    QVector<QVariantMap> result;

    query.prepare(Library::database().getQuery("TableAccounts", "getInformation"));
    query.bindValue(":id_account", this->id);
    query.bindValue(":name", name);
    if (Library::database().query(query, result) && result.size() > 0)
        return (result.value(0).value("value").toString());
    return ("");
}

QVariantMap TableAccounts::getInformations() const
{
    QSqlQuery            query(Library::database().getDatabase());
    QVector<QVariantMap> result;
    QVariantMap          informations;
    int                  i;
    int                  s;

    query.prepare(Library::database().getQuery("TableAccounts", "getInformations"));
    query.bindValue(":id_account", this->id);
    if (Library::database().query(query, result))
        for (i = 0, s = result.size(); i < s; ++i)
            informations.insert(result[i]["name"].toString(), result[i]["value"]);
    return (informations);
}

bool    TableAccounts::setInformation(const QString &name, const QVariant &value)
{
    QSqlQuery            query(Library::database().getDatabase());
    QVector<QVariantMap> result;

    query.prepare(Library::database().getQuery("TableAccounts", "setInformation_select"));
    query.bindValue(":id_account", this->id);
    query.bindValue(":name", name);
    Library::database().query(query, result);
    if (result.size() > 0)
    {
        query.prepare(Library::database().getQuery("TableAccounts", "setInformation_update"));
        query.bindValue(":value", value);
        query.bindValue(":id_account", this->id);
        query.bindValue(":name", name);
    }
    else
    {
        query.prepare(Library::database().getQuery("TableAccounts", "setInformation_insert"));
        query.bindValue(":id", createUuid());
        query.bindValue(":id_account", this->id);
        query.bindValue(":name", name);
        query.bindValue(":value", value);
    }
    return (Library::database().query(query));
}

QStringList TableAccounts::setInformations(const QVariantMap &informations)
{
    QMapIterator<QString, QVariant> it(informations);
    QStringList                     result;

    while (it.hasNext())
    {
        it.next();
        if (!this->setInformation(it.key(), it.value()))
            result << it.key();
    }
    return (result);
}

bool    TableAccounts::removeInformation(const QString &name)
{
    QSqlQuery   query(Library::database().getDatabase());

    query.prepare(Library::database().getQuery("TableAccounts", "removeInformation"));
    query.bindValue(":id_account", this->id);
    query.bindValue(":name", name);
    return (Library::database().query(query) && query.numRowsAffected() > 0);
}

bool    TableAccounts::removeInformations(const QStringList &informations)
{
    QSqlQuery   query(Library::database().getDatabase());
    bool        result = true;

    if (informations.isEmpty())
    {
        query.prepare(Library::database().getQuery("TableAccounts", "removeInformations"));
        query.bindValue(":id_account", this->id);
        result = Library::database().query(query);
    }
    else
    {
        QStringListIterator it(informations);
        while (it.hasNext() && result)
            result = this->removeInformation(it.next());
    }
    return (result);
}

QStringList TableAccounts::getGroups() const
{
    QSqlQuery            query(Library::database().getDatabase());
    QVector<QVariantMap> result;
    QStringList          groups;
    int                  i;
    int                  s;

    query.prepare(Library::database().getQuery("TableAccounts", "getGroups"));
    query.bindValue(":id_account", this->id);
    if (Library::database().query(query, result))
        for (i = 0, s = result.size(); i < s; ++i)
            groups << result[i]["id_group"].toString();
    return (groups);
}

bool    TableAccounts::addGroup(const QString &id_group)
{
    QSqlQuery   query(Library::database().getDatabase());
    QString     id;

    id = createUuid();
    query.prepare(Library::database().getQuery("TableAccounts", "addGroup"));
    query.bindValue(":id", id);
    query.bindValue(":id_account", this->id);
    query.bindValue(":id_group", id_group);
    return (Library::database().query(query));
}

bool    TableAccounts::removeGroup(const QString &id_group)
{
    QSqlQuery   query(Library::database().getDatabase());

    query.prepare(Library::database().getQuery("TableAccounts", "removeGroup"));
    query.bindValue(":id_account", this->id);
    query.bindValue(":id_group", id_group);
    return (Library::database().query(query) && query.numRowsAffected() > 0);
}

QString TableAccounts::passwordHash(const QString &password, const QString &id) const
{
    if (password.isEmpty())
        return (QString());
    if (id.isEmpty())
        return (sha256(password.toLatin1()));
    return (sha256(password.toLatin1() + id.toLatin1()));
}
