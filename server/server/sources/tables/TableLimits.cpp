#include "Database.h"
#include "LightBird.h"
#include "TableLimits.h"

TableLimits::TableLimits(const QString &id)
{
    this->tableName = "limits";
    this->tableId = LightBird::ITable::Limits;
    this->setId(id);
}

TableLimits::~TableLimits()
{
}

TableLimits::TableLimits(const TableLimits &table) : Table()
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

    id = LightBird::createUuid();
    query.prepare(Database::instance()->getQuery("TableLimits", "add"));
    query.bindValue(":id", id);
    query.bindValue(":name", name);
    query.bindValue(":value", value);
    query.bindValue(":id_accessor", id_accessor);
    query.bindValue(":id_object", id_object);
    if (!Database::instance()->query(query) || query.numRowsAffected() == 0)
        return (false);
    this->id = id;
    return (true);
}

QString TableLimits::getIdAccessor() const
{
    QSqlQuery               query;
    QVector<QVariantMap>    result;

    query.prepare(Database::instance()->getQuery("TableLimits", "getIdAccessor"));
    query.bindValue(":id", this->id);
    if (Database::instance()->query(query, result) && result.size() > 0)
        return (result[0]["id_accessor"].toString());
    return ("");
}

bool    TableLimits::setIdAccessor(const QString &id_accessor)
{
    QSqlQuery   query;

    query.prepare(Database::instance()->getQuery("TableLimits", "setIdAccessor"));
    query.bindValue(":id", this->id);
    query.bindValue(":id_accessor", id_accessor);
    return (Database::instance()->query(query));
}

QString TableLimits::getIdObject() const
{
    QSqlQuery               query;
    QVector<QVariantMap>    result;

    query.prepare(Database::instance()->getQuery("TableLimits", "getIdObject"));
    query.bindValue(":id", this->id);
    if (Database::instance()->query(query, result) && result.size() > 0)
        return (result[0]["id_object"].toString());
    return ("");
}

bool    TableLimits::setIdObject(const QString &id_object)
{
    QSqlQuery   query;

    query.prepare(Database::instance()->getQuery("TableLimits", "setIdObject"));
    query.bindValue(":id", this->id);
    query.bindValue(":id_object", id_object);
    return (Database::instance()->query(query));
}

QString TableLimits::getName() const
{
    QSqlQuery               query;
    QVector<QVariantMap>    result;

    query.prepare(Database::instance()->getQuery("TableLimits", "getName"));
    query.bindValue(":id", this->id);
    if (Database::instance()->query(query, result) && result.size() > 0)
        return (result[0]["name"].toString());
    return ("");
}

bool    TableLimits::setName(const QString &name)
{
    QSqlQuery   query;

    query.prepare(Database::instance()->getQuery("TableLimits", "setName"));
    query.bindValue(":id", this->id);
    query.bindValue(":name", name);
    return (Database::instance()->query(query));
}

QString TableLimits::getValue() const
{
    QSqlQuery               query;
    QVector<QVariantMap>    result;

    query.prepare(Database::instance()->getQuery("TableLimits", "getValue"));
    query.bindValue(":id", this->id);
    if (Database::instance()->query(query, result) && result.size() > 0)
        return (result[0]["value"].toString());
    return ("");
}

bool    TableLimits::setValue(const QString &value)
{
    QSqlQuery   query;

    query.prepare(Database::instance()->getQuery("TableLimits", "setValue"));
    query.bindValue(":id", this->id);
    query.bindValue(":value", value);
    return (Database::instance()->query(query));
}
