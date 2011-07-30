#include "Database.h"
#include "TableGroups.h"
#include "Tools.h"

TableGroups::TableGroups(const QString &id)
{
    this->tableName = "groups";
    this->tableId = LightBird::ITable::Groups;
    this->setId(id);
}

TableGroups::~TableGroups()
{
}

TableGroups::TableGroups(const TableGroups &table)
{
    *this = table;
}

TableGroups &TableGroups::operator=(const TableGroups &table)
{
    TableAccessors::operator=(table);
    return (*this);
}

QStringList     TableGroups::getIdFromName(const QString &name) const
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

    id = Tools::createUuid();
    query.prepare(Database::instance()->getQuery("TableGroups", "add"));
    query.bindValue(":id", id);
    query.bindValue(":name", name);
    query.bindValue(":id_group", id_group);
    if (!Database::instance()->query(query) || query.numRowsAffected() == 0)
        return (false);
    this->id = id;
    return (true);
}

QString         TableGroups::getIdGroup() const
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

    id = Tools::createUuid();
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

QStringList     TableGroups::getAccounts() const
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
