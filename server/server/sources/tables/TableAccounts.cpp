#include <QCryptographicHash>
#include <QUuid>

#include "Defines.h"
#include "Log.h"
#include "Database.h"
#include "TableAccounts.h"
#include "TableGroups.h"

TableAccounts::TableAccounts(const QString &id)
{
    this->tableName = "accounts";
    this->tableId = Streamit::ITable::Accounts;
    if (!id.isEmpty())
        this->setId(id);
}

TableAccounts::~TableAccounts()
{
}

TableAccounts::TableAccounts(const TableAccounts &t) : Table(), TableAccessors()
{
    *this = t;
}

TableAccounts &TableAccounts::operator=(const TableAccounts &t)
{
    if (this != &t)
    {
        this->id = t.id;
        this->tableId = t.tableId;
        this->tableName = t.tableName;
        this->connectionDate = t.connectionDate;
    }
    return (*this);
}

bool        TableAccounts::setId(const QString &id)
{
    if (!Table::setId(id))
        return (false);
    this->connectionDate = QDateTime::currentDateTime();
    return (true);
}

bool        TableAccounts::setIdFromName(const QString &name)
{
    QVector<QMap<QString, QVariant> >   result;
    QSqlQuery                           query;

    query.prepare(Database::instance()->getQuery("TableAccounts", "setIdFromName"));
    query.bindValue(":name", name);
    if (Database::instance()->query(query, result) && result.size() != 1)
        return (false);
    this->id = result[0]["id"].toString();
    this->connectionDate = QDateTime::currentDateTime();
    return (true);
}

QString     TableAccounts::getIdFromNameAndPassword(const QString &name, const QString &password)
{
    QVector<QMap<QString, QVariant> >   result;
    QSqlQuery                           query;
    QString                             hash = "";

    // Convert the password into a sha1 hash if it not empty
    if (!password.isEmpty())
        hash = QCryptographicHash::hash(password.toAscii(), QCryptographicHash::Sha1).toHex();
    query.prepare(Database::instance()->getQuery("TableAccounts", "getIdFromNameAndPassword"));
    query.bindValue(":name", name);
    query.bindValue(":password", hash);
    // If there are only one result, the name and the password match
    if (Database::instance()->query(query, result) && result.size() == 1)
    {
        Log::debug("Account connected", Properties("id", result[0]["id"]).add("name", name), "TableAccounts", "connect");
        return (result[0]["id"].toString());
    }
    else
        Log::debug("Failed to connect an account", Properties("name", name).add("passwordHash", hash), "TableAccounts", "connect");
    return ("");
}

bool        TableAccounts::setIdFromNameAndPassword(const QString &name, const QString &password)
{
    QString id;

    if ((id = this->getIdFromNameAndPassword(name, password)).isEmpty())
        return (false);
    this->id = id;
    this->connectionDate = QDateTime::currentDateTime();
    return (true);
}

QString     TableAccounts::getIdFromIdentifiantAndSalt(const QString &identifiant, const QString &salt)
{
    QVector<QMap<QString, QVariant> >   result;
    QSqlQuery                           query;
    int                                 i;
    int                                 s;

    query.prepare(Database::instance()->getQuery("TableAccounts", "getIdFromIdentifiantAndSalt"));
    if (Database::instance()->query(query, result))
        // Check the identifiant of all the account
        for (i = 0, s = result.size(); i < s; ++i)
        {
            // If the identifiant of the account match, it is connected
            if (identifiant == QCryptographicHash::hash(result[i]["name"].toByteArray() + result[i]["password"].toByteArray() + salt.toAscii(), QCryptographicHash::Sha1).toHex())
            {
                Log::debug("Account connected", Properties("id", result[i]["id"]).add("identifiant", identifiant).add("salt", salt), "TableAccounts", "connect");
                return (result[i]["id"].toString());
            }
        }
    Log::debug("Failed to connect an account", Properties("identifiant", identifiant).add("salt", salt), "TableAccounts", "connect");
    return ("");
}

bool        TableAccounts::setIdFromIdentifiantAndSalt(const QString &identifiant, const QString &salt)
{
    QString id;

    if ((id = this->getIdFromIdentifiantAndSalt(identifiant, salt)).isEmpty())
        return (false);
    this->id = id;
    this->connectionDate = QDateTime::currentDateTime();
    return (true);
}

bool            TableAccounts::add(const QString &name, const QMap<QString, QVariant> &informations,
                                   const QString &password, bool administrator, bool active)
{
    QSqlQuery   query;
    QString     id;
    QString     hash = "";

    id = QUuid::createUuid().toString().remove(0, 1).remove(36, 1);
    if (!password.isEmpty())
        hash = QCryptographicHash::hash(password.toAscii(), QCryptographicHash::Sha1).toHex();
    query.prepare(Database::instance()->getQuery("TableAccounts", "add"));
    query.bindValue(":id", id);
    query.bindValue(":name", name);
    query.bindValue(":password", hash);
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
    return (this->add(name, QMap<QString, QVariant>(), password, administrator, active));
}

QString         TableAccounts::getPassword()
{
    QSqlQuery                           query;
    QVector<QMap<QString, QVariant> >   result;

    query.prepare(Database::instance()->getQuery("TableAccounts", "getPassword"));
    query.bindValue(":id", this->id);
    if (Database::instance()->query(query, result) && result.size() > 0)
        return (result[0]["password"].toString());
    return ("");
}

bool            TableAccounts::setPassword(const QString &password)
{
    QSqlQuery   query;
    QString     hash = "";

    if (!password.isEmpty())
        hash = QCryptographicHash::hash(password.toAscii(), QCryptographicHash::Sha1).toHex();
    query.prepare(Database::instance()->getQuery("TableAccounts", "setPassword"));
    query.bindValue(":password", hash);
    query.bindValue(":id", this->id);
    return (Database::instance()->query(query));
}

bool            TableAccounts::isAdministrator()
{
    QSqlQuery                           query;
    QVector<QMap<QString, QVariant> >   result;

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

bool            TableAccounts::isActive()
{
    QSqlQuery                           query;
    QVector<QMap<QString, QVariant> >   result;

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

QVariant        TableAccounts::getInformation(const QString &name)
{
    QVector<QMap<QString, QVariant> >   result;
    QSqlQuery                           query;

    query.prepare(Database::instance()->getQuery("TableAccounts", "getInformation"));
    query.bindValue(":id_account", this->id);
    query.bindValue(":name", name);
    if (Database::instance()->query(query, result) && result.size() > 0)
        return (result.value(0).value("value").toString());
    return ("");
}

bool            TableAccounts::setInformation(const QString &name, const QVariant &value)
{
    QVector<QMap<QString, QVariant> >   result;
    QSqlQuery                           query;

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
        query.bindValue(":id", QUuid::createUuid().toString().remove(0, 1).remove(36, 1));
        query.bindValue(":id_account", this->id);
        query.bindValue(":name", name);
        query.bindValue(":value", value);
    }
    return (Database::instance()->query(query));
}

QMap<QString, QVariant> TableAccounts::getInformations()
{
    QVector<QMap<QString, QVariant> >   result;
    QSqlQuery                           query;
    QMap<QString, QVariant>             informations;
    int                                 i;
    int                                 s;

    query.prepare(Database::instance()->getQuery("TableAccounts", "getInformations"));
    query.bindValue(":id_account", this->id);
    if (Database::instance()->query(query, result))
        for (i = 0, s = result.size(); i < s; ++i)
            informations.insert(result[i]["name"].toString(), result[i]["value"]);
    return (informations);
}

bool            TableAccounts::setInformations(const QMap<QString, QVariant> &informations)
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

bool                    TableAccounts::removeInformations(const QStringList &informations)
{
    QStringListIterator it(informations);
    bool                result = true;

    while (it.hasNext())
    {
        it.next();
        if (!this->removeInformation(it.peekPrevious()))
            result = false;
    }
    return (result);
}

const QDateTime &TableAccounts::getConnectionDate()
{
    return (this->connectionDate);
}

QStringList     TableAccounts::getGroups()
{
    QSqlQuery                           query;
    QVector<QMap<QString, QVariant> >   result;
    QStringList                         groups;
    int                                 i;
    int                                 s;

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

    id = QUuid::createUuid().toString().remove(0, 1).remove(36, 1);
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

bool                TableAccounts::unitTests()
{
    TableAccounts   a1;
    TableAccounts   a2;
    QSqlQuery       query;
    QString         id1;
    QString         id2;

    Log::instance()->debug("Running unit tests...", "TableAccounts", "unitTests");
    query.prepare("DELETE FROM accounts WHERE name=\"a1\" OR name=\"a2\" OR name=\"a3\"");
    Database::instance()->query(query);
    query.prepare("DELETE FROM groups WHERE name=\"g1\" OR name=\"g2\" OR name=\"g3\"");
    Database::instance()->query(query);
    try
    {
        ASSERT(a1.add("a1", "p1", true, true));
        ASSERT(!a1.add("a1", "p2"));
        ASSERT(a1.getTableId() == Streamit::ITable::Accounts);
        ASSERT(a1.getTableName() == "accounts");
        ASSERT(a1.isTable(a1.getTableId()));
        ASSERT(a1.isTable(a1.getTableName()));
        ASSERT(!a1.getId().isEmpty());
        ASSERT(a1.setId(a1.getId()));
        a2 = a1;
        ASSERT(a1.getId() == a2.getId());
        a2.clear();
        ASSERT(a2.getId().isEmpty());
        ASSERT(a2.add("a2", ""));
        id2 = a2.getId();
        ASSERT(a2.add("a3", ""));
        id1 = a1.getId();
        a1 = a2;
        ASSERT(a2.remove());
        ASSERT(!a2.exists());
        ASSERT(a2.getId().isEmpty());
        ASSERT(!a1.exists());
        ASSERT(a1.getId().isEmpty());
        ASSERT(a1.setId(id1));
        ASSERT(a2.remove(id2));
        ASSERT(!a2.remove(id2));
        ASSERT(!a2.remove());
        ASSERT(a1.getModified().isValid());
        ASSERT(a1.getCreated().isValid());
        ASSERT(a1.getName() == "a1");
        ASSERT(a1.setName("a3"));
        ASSERT(a1.getName() == "a3");
        ASSERT(a1.setName("a3"));
        ASSERT(TableAccounts(a1.getId()).getName() == "a3");
        ASSERT(a2.add("a2"));
        ASSERT(!a2.setName("a3"));
        ASSERT(!a2.setName(""));
        ASSERT(a1.getIdFromNameAndPassword("a3", "p1") == a1.getId());
        ASSERT(a2.setIdFromNameAndPassword("a3", "p1"));
        ASSERT(a2.getId() == a1.getId());
        ASSERT(a2.setIdFromNameAndPassword("a2", ""));
        ASSERT(a2.setIdFromIdentifiantAndSalt(QCryptographicHash::hash("a3" + QCryptographicHash::hash("p1", QCryptographicHash::Sha1).toHex() + "salt", QCryptographicHash::Sha1).toHex().data(), "salt"));
        ASSERT(a2.setIdFromIdentifiantAndSalt(QCryptographicHash::hash(QByteArray("a3") + a1.getPassword().toAscii() + "salt", QCryptographicHash::Sha1).toHex().data(), "salt"));
        ASSERT(a2.setIdFromIdentifiantAndSalt(QCryptographicHash::hash(QByteArray("a2") + "salt", QCryptographicHash::Sha1).toHex().data(), "salt"));
        ASSERT(a1.getPassword() == QCryptographicHash::hash("p1", QCryptographicHash::Sha1).toHex());
        ASSERT(a1.setPassword(""));
        ASSERT(a1.getPassword().isEmpty());
        ASSERT(a1.setPassword("p2"));
        ASSERT(a1.getPassword() == QCryptographicHash::hash("p2", QCryptographicHash::Sha1).toHex());
        ASSERT(a1.isActive());
        ASSERT(a1.isActive(false));
        ASSERT(!a1.isActive());
        ASSERT(a1.isActive(true));
        ASSERT(a1.isActive());
        ASSERT(a1.isAdministrator());
        ASSERT(a1.isAdministrator(false));
        ASSERT(!a1.isAdministrator());
        ASSERT(a1.isAdministrator(true));
        ASSERT(a1.isAdministrator());
        ASSERT(a1.setInformation("key1", "value1"));
        ASSERT(a1.getInformation("key1") == "value1");
        ASSERT(a1.setInformation("key1", "value2"));
        ASSERT(a1.getInformation("key1") == "value2");
        ASSERT(a1.setInformation("key2", "value3"));
        ASSERT(a1.getInformation("key2") == "value3");
        QMap<QString, QVariant> i;
        i.insert("key1", "value2");
        i.insert("key2", "value3");
        ASSERT(i == a1.getInformations());
        i.insert("key2", "value4");
        i.insert("key3", "value5");
        ASSERT(a1.setInformations(i));
        ASSERT(i == a1.getInformations());
        ASSERT(i.remove("key2"));
        ASSERT(a1.removeInformation("key3"));
        ASSERT(!a1.removeInformations(i.keys()));
        ASSERT(a1.getInformations().size() == 1);
        ASSERT(a1.getInformation("key2") == "value4");
        TableGroups g;
        ASSERT(g.add("g1"));
        ASSERT(a1.addGroup(g.getId()));
        ASSERT(g.add("g2"));
        ASSERT(a1.addGroup(g.getId()));
        ASSERT(a1.getGroups().size() == 2);
        ASSERT(a1.getGroups().contains(g.getId()));
        ASSERT(g.remove());
        ASSERT(a1.getGroups().size() == 1);
        ASSERT(g.remove(a1.getGroups().first()));
        ASSERT(a1.remove());
        ASSERT(a2.remove());
    }
    catch (Properties properties)
    {
        Log::instance()->error("Unit test failed", properties, "TableAccounts", "unitTests");
        return (false);
    }
    Log::instance()->debug("Unit tests successful!", "TableAccounts", "unitTests");
    return (true);
}
