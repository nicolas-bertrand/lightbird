#include "Database.h"
#include "TablePermissions.h"
#include "TableObjects.h"

TableObjects::TableObjects()
{
}

TableObjects::~TableObjects()
{
}

TableObjects::TableObjects(const TableObjects &t) : Table()
{
    *this = t;
}

TableObjects &TableObjects::operator=(const TableObjects &t)
{
    if (this != &t)
    {
        ;
    }
    return (*this);
}

QString TableObjects::getIdAccount()
{
    QSqlQuery                           query;
    QVector<QMap<QString, QVariant> >   result;

    query.prepare(Database::instance()->getQuery("TableObjects", "getIdAccount").replace(":table", this->tableName));
    query.bindValue(":id", this->id);
    if (Database::instance()->query(query, result) && result.size() > 0)
        return (result[0]["id_account"].toString());
    return ("");
}

bool    TableObjects::setIdAccount(const QString &id_account)
{
    QSqlQuery   query;

    query.prepare(Database::instance()->getQuery("TableObjects", "setIdAccount").replace(":table", this->tableName));
    query.bindValue(":id", this->id);
    query.bindValue(":id_account", id_account);
    return (Database::instance()->query(query));
}

QString TableObjects::getName()
{
    QSqlQuery                           query;
    QVector<QMap<QString, QVariant> >   result;

    query.prepare(Database::instance()->getQuery("TableObjects", "getName").replace(":table", this->tableName));
    query.bindValue(":id", this->id);
    if (Database::instance()->query(query, result) && result.size() > 0)
        return (result[0]["name"].toString());
    return ("");
}

bool    TableObjects::setName(const QString &name)
{
    QSqlQuery   query;

    if (name.isEmpty())
        return (false);
    query.prepare(Database::instance()->getQuery("TableObjects", "setName").replace(":table", this->tableName));
    query.bindValue(":id", this->id);
    query.bindValue(":name", name);
    return (Database::instance()->query(query));
}

bool    TableObjects::isAllowed(const QString &id_accessor, const QString &right)
{
    return (TablePermissions().isAllowed(id_accessor, this->id, right));
}

QStringList TableObjects::getRights(const QString &id_accessor)
{
    return (TablePermissions().getRights(id_accessor, this->id));
}

QStringList TableObjects::getTags()
{
    QSqlQuery                           query;
    QVector<QMap<QString, QVariant> >   result;
    QStringList                         tags;
    int                                 i;
    int                                 s;

    query.prepare(Database::instance()->getQuery("TableObjects", "getTags"));
    query.bindValue(":id_object", this->id);
    Database::instance()->query(query, result);
    for (i = 0, s = result.size(); i < s; ++i)
        tags << result[i]["id"].toString();
    return (tags);
}
