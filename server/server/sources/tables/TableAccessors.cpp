#include "Database.h"
#include "TableAccessors.h"

TableAccessors::TableAccessors()
{
}

TableAccessors::~TableAccessors()
{
}

TableAccessors::TableAccessors(const TableAccessors &t) : Table()
{
    *this = t;
}

TableAccessors &TableAccessors::operator=(const TableAccessors &t)
{
    if (this != &t)
    {
        ;
    }
    return (*this);
}

QString TableAccessors::getName()
{
    QSqlQuery                           query;
    QVector<QMap<QString, QVariant> >   result;

    query.prepare(Database::instance()->getQuery("TableAccessors", "getName").replace(":table", this->tableName));
    query.bindValue(":id", this->id);
    if (Database::instance()->query(query, result) && result.size() > 0)
        return (result[0]["name"].toString());
    return ("");
}

bool            TableAccessors::setName(const QString &name)
{
    QSqlQuery   query;

    if (name.isEmpty())
        return (false);
    query.prepare(Database::instance()->getQuery("TableAccessors", "setName").replace(":table", this->tableName));
    query.bindValue(":id", this->id);
    query.bindValue(":name", name);
    return (Database::instance()->query(query));
}

QStringList TableAccessors::getLimits()
{
    QSqlQuery                           query;
    QVector<QMap<QString, QVariant> >   result;
    QStringList                         tags;
    int                                 i;
    int                                 s;

    query.prepare(Database::instance()->getQuery("TableAccessors", "getLimits"));
    query.bindValue(":id_accessor", this->id);
    Database::instance()->query(query, result);
    for (i = 0, s = result.size(); i < s; ++i)
        tags << result[i]["id"].toString();
    return (tags);
}
