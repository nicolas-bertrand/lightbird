#include "Library.h"
#include "LightBird.h"
#include "Table.h"

using namespace LightBird;

Table::Table()
{
}

Table::~Table()
{
}

Table::Table(const Table &table)
    : QObject()
{
    *this = table;
}

Table &Table::operator=(const Table &table)
{
    if (this != &table)
    {
        this->id = table.id;
        this->tableName = table.tableName;
        this->tableId = table.tableId;
    }
    return (*this);
}

const QString   &Table::getId() const
{
    return (this->id);
}

bool            Table::setId(const QString &id)
{
    if (!this->exists(id))
        return (false);
    this->id = id;
    return (true);
}

bool            Table::exists(const QString &id)
{
    QSqlQuery               query;
    QVector<QVariantMap>    result;

    if (id.isEmpty() && this->id.isEmpty())
        return (false);
    query.prepare(Library::database().getQuery("Table", "exists").replace(":table", this->tableName));
    if (id.isEmpty())
        query.bindValue(":id", this->id);
    else
        query.bindValue(":id", id);
    if (Library::database().query(query, result) && result.size() > 0)
        return (true);
    if (id.isEmpty())
        this->id.clear();
    return (false);
}

Table::operator bool()
{
    return (this->exists());
}

void            Table::clear()
{
    this->id.clear();
}

bool            Table::remove(const QString &id)
{
    QSqlQuery   query;

    if (this->id.isEmpty() && id.isEmpty())
        return (false);
    query.prepare(Library::database().getQuery("Table", "remove").replace(":table", this->tableName));
    if (id.isEmpty())
    {
        query.bindValue(":id", this->id);
        this->clear();
    }
    else
        query.bindValue(":id", id);
    return (Library::database().query(query) && query.numRowsAffected() > 0);
}

QDateTime       Table::getModified() const
{
    QSqlQuery               query;
    QVector<QVariantMap>    result;

    query.prepare(Library::database().getQuery("Table", "getModified").replace(":table", this->tableName));
    query.bindValue(":id", this->id);
    if (Library::database().query(query, result) && result.size() > 0)
        return (result[0]["modified"].toDateTime());
    return (QDateTime());
}

QDateTime       Table::getCreated() const
{
    QSqlQuery               query;
    QVector<QVariantMap>    result;

    query.prepare(Library::database().getQuery("Table", "getCreated").replace(":table", this->tableName));
    query.bindValue(":id", this->id);
    if (Library::database().query(query, result) && result.size() > 0)
        return (result[0]["created"].toDateTime());
    return (QDateTime());
}

const QString   &Table::getTableName() const
{
    return (this->tableName);
}

Table::Id    Table::getTableId() const
{
    return (this->tableId);
}

bool            Table::isTable(const QString &tableName) const
{
    if (this->tableName != tableName)
        return (false);
    return (true);
}

bool            Table::isTable(Table::Id tableId) const
{
    if (this->tableId != tableId)
        return (false);
    return (true);
}

TableAccessors   *Table::toAccessors()
{
    return (dynamic_cast<TableAccessors *>(this));
}

TableAccounts    *Table::toAccounts()
{
    return (dynamic_cast<TableAccounts *>(this));
}

TableCollections *Table::toCollections()
{
    return (dynamic_cast<TableCollections *>(this));
}

TableDirectories *Table::toDirectories()
{
    return (dynamic_cast<TableDirectories *>(this));
}

TableEvents      *Table::toEvents()
{
    return (dynamic_cast<TableEvents *>(this));
}

TableFiles       *Table::toFiles()
{
    return (dynamic_cast<TableFiles *>(this));
}

TableGroups      *Table::toGroups()
{
    return (dynamic_cast<TableGroups *>(this));
}

TableLimits      *Table::toLimits()
{
    return (dynamic_cast<TableLimits *>(this));
}

TableObjects     *Table::toObjects()
{
    return (dynamic_cast<TableObjects *>(this));
}

TablePermissions *Table::toPermissions()
{
    return (dynamic_cast<TablePermissions *>(this));
}

TableTags        *Table::toTags()
{
    return (dynamic_cast<TableTags *>(this));
}
