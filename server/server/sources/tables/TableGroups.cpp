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

QStringList     TableGroups::getIdFromName(const QString &name)
{
    QSqlQuery                           query;
    QVector<QMap<QString, QVariant> >   result;
    QStringList                         groups;
    int                                 i;
    int                                 s;

    query.prepare(Database::instance()->getQuery("TableGroups", "getId"));
    query.bindValue(":name", name);
    if (Database::instance()->query(query, result))
        for (i = 0, s = result.size(); i < s; ++i)
            groups << result[i]["id"].toString();
    return (groups);
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
