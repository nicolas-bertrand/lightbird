#include "Library.h"
#include "LightBird.h"

using namespace LightBird;

TableAccessors::TableAccessors()
{
}

TableAccessors::~TableAccessors()
{
}

TableAccessors::TableAccessors(const TableAccessors &table) : Table()
{
    *this = table;
}

TableAccessors &TableAccessors::operator=(const TableAccessors &table)
{
    Table::operator=(table);
    return (*this);
}

QString         TableAccessors::getName() const
{
    QSqlQuery               query;
    QVector<QVariantMap>    result;

    query.prepare(Library::database().getQuery("TableAccessors", "getName").replace(":table", this->tableName));
    query.bindValue(":id", this->id);
    if (Library::database().query(query, result) && result.size() > 0)
        return (result[0]["name"].toString());
    return ("");
}

bool            TableAccessors::setName(const QString &name)
{
    QSqlQuery   query;

    if (name.isEmpty())
        return (false);
    query.prepare(Library::database().getQuery("TableAccessors", "setName").replace(":table", this->tableName));
    query.bindValue(":id", this->id);
    query.bindValue(":name", name);
    return (Library::database().query(query));
}

bool            TableAccessors::isAllowed(const QString &id_object, const QString &right) const
{
    return (TablePermissions().isAllowed(this->id, id_object, right));
}

bool            TableAccessors::getRights(const QString &id_object, QStringList &allowed, QStringList &denied) const
{
    return (TablePermissions().getRights(this->id, id_object, allowed, denied));
}

QStringList     TableAccessors::getLimits() const
{
    QSqlQuery               query;
    QVector<QVariantMap>    result;
    QStringList             tags;
    int                     i;
    int                     s;

    query.prepare(Library::database().getQuery("TableAccessors", "getLimits"));
    query.bindValue(":id_accessor", this->id);
    Library::database().query(query, result);
    for (i = 0, s = result.size(); i < s; ++i)
        tags << result[i]["id"].toString();
    return (tags);
}
