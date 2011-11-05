#include "Database.h"
#include "Table.h"
#include "TableAccounts.h"
#include "TableCollections.h"
#include "TableDirectories.h"
#include "TableEvents.h"
#include "TableFiles.h"
#include "TableGroups.h"
#include "TableLimits.h"
#include "TablePermissions.h"
#include "TableTags.h"

Table::Table()
{
}

Table::~Table()
{
}

Table::Table(const Table &table)
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
    QSqlQuery   query;

    if (!this->exists(id))
        return (false);
    this->id = id;
    return (true);
}

bool            Table::exists(const QString &id)
{
    QSqlQuery                           query;
    QVector<QMap<QString, QVariant> >   result;

    query.prepare(Database::instance()->getQuery("Table", "exists").replace(":table", this->tableName));
    if (id.isEmpty())
        query.bindValue(":id", this->id);
    else
        query.bindValue(":id", id);
    if (Database::instance()->query(query, result) && result.size() > 0)
        return (true);
    if (id.isEmpty())
        this->id.clear();
    return (false);
}

void            Table::clear()
{
    this->id.clear();
}

bool            Table::remove(const QString &id)
{
    QSqlQuery   query;

    query.prepare(Database::instance()->getQuery("Table", "remove").replace(":table", this->tableName));
    if (id.isEmpty())
        query.bindValue(":id", this->id);
    else
        query.bindValue(":id", id);
    return (Database::instance()->query(query) && query.numRowsAffected() > 0);
}

QDateTime       Table::getModified() const
{
    QSqlQuery                           query;
    QVector<QMap<QString, QVariant> >   result;

    query.prepare(Database::instance()->getQuery("Table", "getModified").replace(":table", this->tableName));
    query.bindValue(":id", this->id);
    if (Database::instance()->query(query, result) && result.size() > 0)
        return (result[0]["modified"].toDateTime());
    return (QDateTime());
}

QDateTime       Table::getCreated() const
{
    QSqlQuery                           query;
    QVector<QMap<QString, QVariant> >   result;

    query.prepare(Database::instance()->getQuery("Table", "getCreated").replace(":table", this->tableName));
    query.bindValue(":id", this->id);
    if (Database::instance()->query(query, result) && result.size() > 0)
        return (result[0]["created"].toDateTime());
    return (QDateTime());
}

const QString   &Table::getTableName() const
{
    return (this->tableName);
}

LightBird::ITable::Table Table::getTableId() const
{
    return (this->tableId);
}

bool            Table::isTable(const QString &tableName) const
{
    if (this->tableName != tableName)
        return (false);
    return (true);
}

bool            Table::isTable(LightBird::ITable::Table tableId) const
{
    if (this->tableId != tableId)
        return (false);
    return (true);
}

LightBird::ITableAccessors   *Table::toAccessors()
{
    return (dynamic_cast<LightBird::ITableAccessors *>(this));
}

LightBird::ITableAccounts    *Table::toAccounts()
{
    return (dynamic_cast<LightBird::ITableAccounts *>(this));
}

LightBird::ITableCollections *Table::toCollections()
{
    return (dynamic_cast<LightBird::ITableCollections *>(this));
}

LightBird::ITableDirectories *Table::toDirectories()
{
    return (dynamic_cast<LightBird::ITableDirectories *>(this));
}

LightBird::ITableEvents      *Table::toEvents()
{
    return (dynamic_cast<LightBird::ITableEvents *>(this));
}

LightBird::ITableFiles       *Table::toFiles()
{
    return (dynamic_cast<LightBird::ITableFiles *>(this));
}

LightBird::ITableGroups      *Table::toGroups()
{
    return (dynamic_cast<LightBird::ITableGroups *>(this));
}

LightBird::ITableLimits      *Table::toLimits()
{
    return (dynamic_cast<LightBird::ITableLimits *>(this));
}

LightBird::ITableObjects     *Table::toObjects()
{
    return (dynamic_cast<LightBird::ITableObjects *>(this));
}

LightBird::ITablePermissions *Table::toPermissions()
{
    return (dynamic_cast<LightBird::ITablePermissions *>(this));
}

LightBird::ITableTags        *Table::toTags()
{
    return (dynamic_cast<LightBird::ITableTags *>(this));
}
