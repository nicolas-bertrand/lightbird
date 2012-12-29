#include "Library.h"
#include "LightBird.h"

using namespace LightBird;

TableGroups::TableGroups(const QString &id)
{
    this->tableName = "groups";
    this->tableId = Table::Groups;
    this->setId(id);
}

TableGroups::~TableGroups()
{
}

TableGroups::TableGroups(const TableGroups &table)
    : TableAccessors()
{
    *this = table;
}

TableGroups &TableGroups::operator=(const TableGroups &table)
{
    TableAccessors::operator=(table);
    return (*this);
}

QStringList TableGroups::getIdFromName(const QString &name) const
{
    QSqlQuery            query(Library::database().getDatabase());
    QVector<QVariantMap> result;
    QStringList          groups;
    int                  i;
    int                  s;

    query.prepare(Library::database().getQuery("TableGroups", "getId"));
    query.bindValue(":name", name);
    if (Library::database().query(query, result))
        for (i = 0, s = result.size(); i < s; ++i)
            groups << result[i]["id"].toString();
    return (groups);
}

bool    TableGroups::add(const QString &name, const QString &id_group)
{
    QSqlQuery   query(Library::database().getDatabase());
    QString     id;

    id = createUuid();
    query.prepare(Library::database().getQuery("TableGroups", "add"));
    query.bindValue(":id", id);
    query.bindValue(":name", name);
    query.bindValue(":id_group", id_group);
    if (!Library::database().query(query) || query.numRowsAffected() == 0)
        return (false);
    this->id = id;
    return (true);
}

QString TableGroups::getIdGroup() const
{
    QSqlQuery            query(Library::database().getDatabase());
    QVector<QVariantMap> result;

    query.prepare(Library::database().getQuery("TableGroups", "getIdGroup"));
    query.bindValue(":id", this->id);
    if (Library::database().query(query, result) && result.size() > 0)
        return (result[0]["id_group"].toString());
    return ("");
}

bool    TableGroups::setIdGroup(const QString &id_group)
{
    QSqlQuery   query(Library::database().getDatabase());

    query.prepare(Library::database().getQuery("TableGroups", "setIdGroup"));
    query.bindValue(":id", this->id);
    query.bindValue(":id_group", id_group);
    return (Library::database().query(query));
}

bool    TableGroups::addAccount(const QString &id_account)
{
    QSqlQuery   query(Library::database().getDatabase());
    QString     id;

    id = createUuid();
    query.prepare(Library::database().getQuery("TableGroups", "addAccount"));
    query.bindValue(":id", id);
    query.bindValue(":id_group", this->id);
    query.bindValue(":id_account", id_account);
    return (Library::database().query(query));
}

bool    TableGroups::removeAccount(const QString &id_account)
{
    QSqlQuery   query(Library::database().getDatabase());

    query.prepare(Library::database().getQuery("TableGroups", "removeAccount"));
    query.bindValue(":id_group", this->id);
    query.bindValue(":id_account", id_account);
    return (Library::database().query(query) && query.numRowsAffected() > 0);
}

QStringList TableGroups::getAccounts() const
{
    QSqlQuery            query(Library::database().getDatabase());
    QVector<QVariantMap> result;
    QStringList          accounts;
    int                  i;
    int                  s;

    query.prepare(Library::database().getQuery("TableGroups", "getAccounts"));
    query.bindValue(":id_group", this->id);
    if (Library::database().query(query, result))
        for (i = 0, s = result.size(); i < s; ++i)
            accounts << result[i]["id_account"].toString();
    return (accounts);
}

QStringList TableGroups::getParents() const
{
    TableGroups group(this->id);
    QString     id;
    QStringList result;

    while (!(id = group.getIdGroup()).isEmpty())
    {
        result << id;
        group.setId(id);
    }
    return (result);
}
