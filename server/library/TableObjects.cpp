#include "Library.h"
#include "LightBird.h"

using namespace LightBird;

TableObjects::TableObjects()
{
}

TableObjects::~TableObjects()
{
}

TableObjects::TableObjects(const TableObjects &table)
    : Table()
{
    *this = table;
}

TableObjects &TableObjects::operator=(const TableObjects &table)
{
    Table::operator=(table);
    return (*this);
}

QString TableObjects::getIdAccount() const
{
    QSqlQuery               query;
    QVector<QVariantMap>    result;

    query.prepare(Library::database().getQuery("TableObjects", "getIdAccount").replace(":table", this->tableName));
    query.bindValue(":id", this->id);
    if (Library::database().query(query, result) && result.size() > 0)
        return (result[0]["id_account"].toString());
    return ("");
}

bool    TableObjects::setIdAccount(const QString &id_account)
{
    QSqlQuery   query;

    query.prepare(Library::database().getQuery("TableObjects", "setIdAccount").replace(":table", this->tableName));
    query.bindValue(":id", this->id);
    query.bindValue(":id_account", id_account);
    return (Library::database().query(query));
}

QString TableObjects::getName() const
{
    QSqlQuery               query;
    QVector<QVariantMap>    result;

    query.prepare(Library::database().getQuery("TableObjects", "getName").replace(":table", this->tableName));
    query.bindValue(":id", this->id);
    if (Library::database().query(query, result) && result.size() > 0)
        return (result[0]["name"].toString());
    return ("");
}

bool    TableObjects::setName(const QString &name)
{
    QSqlQuery   query;

    if (!LightBird::isValidName(name))
        return (false);
    query.prepare(Library::database().getQuery("TableObjects", "setName").replace(":table", this->tableName));
    query.bindValue(":id", this->id);
    query.bindValue(":name", name);
    return (Library::database().query(query));
}

bool    TableObjects::isAllowed(const QString &id_accessor, const QString &right) const
{
    return (TablePermissions().isAllowed(id_accessor, this->id, right));
}

bool    TableObjects::getRights(const QString &id_accessor, QStringList &allowed, QStringList &denied) const
{
    return (TablePermissions().getRights(id_accessor, this->id, allowed, denied));
}

QStringList TableObjects::getTags() const
{
    QSqlQuery               query;
    QVector<QVariantMap>    result;
    QStringList             tags;
    int                     i;
    int                     s;

    query.prepare(Library::database().getQuery("TableObjects", "getTags"));
    query.bindValue(":id_object", this->id);
    Library::database().query(query, result);
    for (i = 0, s = result.size(); i < s; ++i)
        tags << result[i]["id"].toString();
    return (tags);
}

QStringList TableObjects::getLimits() const
{
    QSqlQuery               query;
    QVector<QVariantMap>    result;
    QStringList             limits;
    int                     i;
    int                     s;

    query.prepare(Library::database().getQuery("TableObjects", "getLimits"));
    query.bindValue(":id_object", this->id);
    Library::database().query(query, result);
    for (i = 0, s = result.size(); i < s; ++i)
        limits << result[i]["id"].toString();
    return (limits);
}
