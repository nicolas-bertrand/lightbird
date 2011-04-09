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

Table::Table(const Table &t) : QObject()
{
    *this = t;
}

Table &Table::operator=(const Table &t)
{
    if (this != &t)
    {
        ;
    }
    return (*this);
}

const QString   &Table::getId()
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

QDateTime       Table::getModified()
{
    QSqlQuery                           query;
    QVector<QMap<QString, QVariant> >   result;

    query.prepare(Database::instance()->getQuery("Table", "getModified").replace(":table", this->tableName));
    query.bindValue(":id", this->id);
    if (Database::instance()->query(query, result) && result.size() > 0)
        return (result[0]["modified"].toDateTime());
    return (QDateTime());
}

QDateTime       Table::getCreated()
{
    QSqlQuery                           query;
    QVector<QMap<QString, QVariant> >   result;

    query.prepare(Database::instance()->getQuery("Table", "getCreated").replace(":table", this->tableName));
    query.bindValue(":id", this->id);
    if (Database::instance()->query(query, result) && result.size() > 0)
        return (result[0]["created"].toDateTime());
    return (QDateTime());
}

const QString   &Table::getTableName()
{
    return (this->tableName);
}

LightBird::ITable::Tables Table::getTableId()
{
    return (this->tableId);
}

bool            Table::isTable(const QString &tableName)
{
    if (this->tableName != tableName)
        return (false);
    return (true);
}

bool            Table::isTable(LightBird::ITable::Tables tableId)
{
    if (this->tableId != tableId)
        return (false);
    return (true);
}

LightBird::ITableAccessors   *Table::toTableAccessors()
{
    return (dynamic_cast<LightBird::ITableAccessors *>(this));
}

LightBird::ITableAccounts    *Table::toTableAccounts()
{
    return (dynamic_cast<LightBird::ITableAccounts *>(this));
}

LightBird::ITableCollections *Table::toTableCollections()
{
    return (dynamic_cast<LightBird::ITableCollections *>(this));
}

LightBird::ITableDirectories *Table::toTableDirectories()
{
    return (dynamic_cast<LightBird::ITableDirectories *>(this));
}

LightBird::ITableEvents      *Table::toTableEvents()
{
    return (dynamic_cast<LightBird::ITableEvents *>(this));
}

LightBird::ITableFiles       *Table::toTableFiles()
{
    return (dynamic_cast<LightBird::ITableFiles *>(this));
}

LightBird::ITableGroups      *Table::toTableGroups()
{
    return (dynamic_cast<LightBird::ITableGroups *>(this));
}

LightBird::ITableLimits      *Table::toTableLimits()
{
    return (dynamic_cast<LightBird::ITableLimits *>(this));
}

LightBird::ITableObjects     *Table::toTableObjects()
{
    return (dynamic_cast<LightBird::ITableObjects *>(this));
}

LightBird::ITablePermissions *Table::toTablePermissions()
{
    return (dynamic_cast<LightBird::ITablePermissions *>(this));
}

LightBird::ITableTags        *Table::toTableTags()
{
    return (dynamic_cast<LightBird::ITableTags *>(this));
}
