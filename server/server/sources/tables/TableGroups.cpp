#include <QUuid>

#include "Defines.h"
#include "Log.h"
#include "Database.h"
#include "TableAccounts.h"
#include "TableGroups.h"

TableGroups::TableGroups(const QString &id)
{
    this->tableName = "groups";
    this->tableId = LightBird::ITable::Groups;
    if (!id.isEmpty())
        this->setId(id);
}

TableGroups::~TableGroups()
{
}

TableGroups::TableGroups(const TableGroups &t) : Table(), TableAccessors()
{
    *this = t;
}

TableGroups &TableGroups::operator=(const TableGroups &t)
{
    if (this != &t)
    {
        this->id = t.id;
        this->tableId = t.tableId;
        this->tableName = t.tableName;
    }
    return (*this);
}

bool            TableGroups::add(const QString &name, const QString &id_group)
{
    QSqlQuery   query;
    QString     id;

    id = QUuid::createUuid().toString().remove(0, 1).remove(36, 1);
    query.prepare(Database::instance()->getQuery("TableGroups", "add"));
    query.bindValue(":id", id);
    query.bindValue(":name", name);
    query.bindValue(":id_group", id_group);
    if (!Database::instance()->query(query) || query.numRowsAffected() == 0)
        return (false);
    this->id = id;
    return (true);
}

QString         TableGroups::getIdGroup()
{
    QSqlQuery                           query;
    QVector<QMap<QString, QVariant> >   result;

    query.prepare(Database::instance()->getQuery("TableGroups", "getIdGroup"));
    query.bindValue(":id", this->id);
    if (Database::instance()->query(query, result) && result.size() > 0)
        return (result[0]["id_group"].toString());
    return ("");
}

bool            TableGroups::setIdGroup(const QString &id_group)
{
    QSqlQuery   query;

    query.prepare(Database::instance()->getQuery("TableGroups", "setIdGroup"));
    query.bindValue(":id", this->id);
    query.bindValue(":id_group", id_group);
    return (Database::instance()->query(query));
}

bool            TableGroups::addAccount(const QString &id_account)
{
    QSqlQuery   query;
    QString     id;

    id = QUuid::createUuid().toString().remove(0, 1).remove(36, 1);
    query.prepare(Database::instance()->getQuery("TableGroups", "addAccount"));
    query.bindValue(":id", id);
    query.bindValue(":id_group", this->id);
    query.bindValue(":id_account", id_account);
    return (Database::instance()->query(query));
}

bool            TableGroups::removeAccount(const QString &id_account)
{
    QSqlQuery   query;

    query.prepare(Database::instance()->getQuery("TableGroups", "removeAccount"));
    query.bindValue(":id_group", this->id);
    query.bindValue(":id_account", id_account);
    return (Database::instance()->query(query) && query.numRowsAffected() > 0);
}

QStringList     TableGroups::getAccounts()
{
    QSqlQuery                           query;
    QVector<QMap<QString, QVariant> >   result;
    QStringList                         accounts;
    int                                 i;
    int                                 s;

    query.prepare(Database::instance()->getQuery("TableGroups", "getAccounts"));
    query.bindValue(":id_group", this->id);
    if (Database::instance()->query(query, result))
        for (i = 0, s = result.size(); i < s; ++i)
            accounts << result[i]["id_account"].toString();
    return (accounts);
}

bool                TableGroups::unitTests()
{
    TableGroups     b1;
    TableGroups     b2;
    QSqlQuery       query;
    QString         id1;
    QString         id2;

    Log::instance()->debug("Running unit tests...", "TableGroups", "unitTests");
    query.prepare("DELETE FROM groups WHERE name=\"b1\" OR name=\"b2\" OR name=\"b3\"");
    Database::instance()->query(query);
    query.prepare("DELETE FROM accounts WHERE name=\"a1\" OR name=\"a2\" OR name=\"a3\"");
    Database::instance()->query(query);
    try
    {
        ASSERT(b1.add("b1"));
        ASSERT(!b1.add("b1", ""));
        ASSERT(b1.getTableId() == LightBird::ITable::Groups);
        ASSERT(b1.getTableName() == "groups");
        ASSERT(b1.isTable(b1.getTableId()));
        ASSERT(b1.isTable(b1.getTableName()));
        ASSERT(!b1.getId().isEmpty());
        ASSERT(b1.setId(b1.getId()));
        b2 = b1;
        ASSERT(b1.getId() == b2.getId());
        b2.clear();
        ASSERT(b2.getId().isEmpty());
        ASSERT(b2.add("b2", ""));
        id2 = b2.getId();
        ASSERT(b2.add("a3", ""));
        id1 = b1.getId();
        b1 = b2;
        ASSERT(b2.remove());
        ASSERT(!b2.exists());
        ASSERT(b2.getId().isEmpty());
        ASSERT(!b1.exists());
        ASSERT(b1.setId(id1));
        ASSERT(b2.remove(id2));
        ASSERT(!b2.remove(id2));
        ASSERT(!b2.remove());
        ASSERT(b1.getModified().isValid());
        ASSERT(b1.getCreated().isValid());
        ASSERT(b1.getName() == "b1");
        ASSERT(b1.setName("a3"));
        ASSERT(b1.getName() == "a3");
        ASSERT(b1.setName("a3"));
        ASSERT(b2.add("b2"));
        ASSERT(!b2.setName("a3"));
        ASSERT(!b2.setName(""));
        ASSERT(b1.remove());
        ASSERT(b2.remove());
        ASSERT(b1.add("b1", ""));
        ASSERT(!b2.add("b2", "toto"));
        ASSERT(b2.add("b2", b1.getId()));
        ASSERT(!b2.add("b2", b2.getIdGroup()));
        ASSERT(b2.add("b3", b2.getIdGroup()));
        ASSERT(!b2.setName("b2"));
        ASSERT(b2.setName("b1"));
        ASSERT(b1.getIdGroup().isEmpty());
        ASSERT(!b2.setIdGroup(b1.getIdGroup()));
        ASSERT(b2.getIdGroup() == b1.getId());
        TableAccounts a;
        ASSERT(a.add("a1"));
        ASSERT(b1.addAccount(a.getId()));
        ASSERT(a.add("a2"));
        ASSERT(b1.addAccount(a.getId()));
        ASSERT(b1.getAccounts().size() == 2);
        ASSERT(b1.getAccounts().contains(a.getId()));
        ASSERT(a.remove());
        ASSERT(b1.getAccounts().size() == 1);
        ASSERT(a.remove(b1.getAccounts().first()));
        ASSERT(b1.remove());
    }
    catch (Properties properties)
    {
        Log::instance()->error("Unit test failed", properties, "TableGroups", "unitTests");
        return (false);
    }
    Log::instance()->debug("Unit tests successful!", "TableGroups", "unitTests");
    return (true);
}
