#include "Library.h"
#include "LightBird.h"

using namespace LightBird;

TableLimits::TableLimits(const QString &id)
{
    this->tableName = "limits";
    this->tableId = Table::Limits;
    this->setId(id);
}

TableLimits::~TableLimits()
{
}

TableLimits::TableLimits(const TableLimits &table)
    : Table()
{
    *this = table;
}

TableLimits &TableLimits::operator=(const TableLimits &table)
{
    Table::operator=(table);
    return (*this);
}

bool    TableLimits::add(const QString &name, const QString &value, const QString &id_accessor, const QString &id_object)
{
    QSqlQuery   query;
    QString     id;

    id = createUuid();
    query.prepare(Library::database().getQuery("TableLimits", "add"));
    query.bindValue(":id", id);
    query.bindValue(":name", name);
    query.bindValue(":value", value);
    query.bindValue(":id_accessor", id_accessor);
    query.bindValue(":id_object", id_object);
    if (!Library::database().query(query) || query.numRowsAffected() == 0)
        return (false);
    this->id = id;
    return (true);
}

QString TableLimits::getIdAccessor() const
{
    QSqlQuery               query;
    QVector<QVariantMap>    result;

    query.prepare(Library::database().getQuery("TableLimits", "getIdAccessor"));
    query.bindValue(":id", this->id);
    if (Library::database().query(query, result) && result.size() > 0)
        return (result[0]["id_accessor"].toString());
    return ("");
}

bool    TableLimits::setIdAccessor(const QString &id_accessor)
{
    QSqlQuery   query;

    query.prepare(Library::database().getQuery("TableLimits", "setIdAccessor"));
    query.bindValue(":id", this->id);
    query.bindValue(":id_accessor", id_accessor);
    return (Library::database().query(query));
}

QString TableLimits::getIdObject() const
{
    QSqlQuery               query;
    QVector<QVariantMap>    result;

    query.prepare(Library::database().getQuery("TableLimits", "getIdObject"));
    query.bindValue(":id", this->id);
    if (Library::database().query(query, result) && result.size() > 0)
        return (result[0]["id_object"].toString());
    return ("");
}

bool    TableLimits::setIdObject(const QString &id_object)
{
    QSqlQuery   query;

    query.prepare(Library::database().getQuery("TableLimits", "setIdObject"));
    query.bindValue(":id", this->id);
    query.bindValue(":id_object", id_object);
    return (Library::database().query(query));
}

QString TableLimits::getName() const
{
    QSqlQuery               query;
    QVector<QVariantMap>    result;

    query.prepare(Library::database().getQuery("TableLimits", "getName"));
    query.bindValue(":id", this->id);
    if (Library::database().query(query, result) && result.size() > 0)
        return (result[0]["name"].toString());
    return ("");
}

bool    TableLimits::setName(const QString &name)
{
    QSqlQuery   query;

    query.prepare(Library::database().getQuery("TableLimits", "setName"));
    query.bindValue(":id", this->id);
    query.bindValue(":name", name);
    return (Library::database().query(query));
}

QString TableLimits::getValue() const
{
    QSqlQuery               query;
    QVector<QVariantMap>    result;

    query.prepare(Library::database().getQuery("TableLimits", "getValue"));
    query.bindValue(":id", this->id);
    if (Library::database().query(query, result) && result.size() > 0)
        return (result[0]["value"].toString());
    return ("");
}

bool    TableLimits::setValue(const QString &value)
{
    QSqlQuery   query;

    query.prepare(Library::database().getQuery("TableLimits", "setValue"));
    query.bindValue(":id", this->id);
    query.bindValue(":value", value);
    return (Library::database().query(query));
}
