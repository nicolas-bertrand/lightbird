#include <QUuid>

#include "Defines.h"
#include "Log.h"
#include "Database.h"
#include "TableDirectories.h"
#include "TableTags.h"

TableTags::TableTags(const QString &id)
{
    this->tableName = "tags";
    this->tableId = LightBird::ITable::Tags;
    if (!id.isEmpty())
        this->setId(id);
}

TableTags::~TableTags()
{
}

TableTags::TableTags(const TableTags &t) : Table()
{
    *this = t;
}

TableTags &TableTags::operator=(const TableTags &t)
{
    if (this != &t)
    {
        this->id = t.id;
        this->tableId = t.tableId;
        this->tableName = t.tableName;
    }
    return (*this);
}

bool    TableTags::add(const QString &id_object, const QString &name)
{
    QSqlQuery   query;
    QString     id;

    id = QUuid::createUuid().toString().remove(0, 1).remove(36, 1);
    query.prepare(Database::instance()->getQuery("TableTags", "add"));
    query.bindValue(":id", id);
    query.bindValue(":id_object", id_object);
    query.bindValue(":name", name);
    if (!Database::instance()->query(query) || query.numRowsAffected() == 0)
        return (false);
    this->id = id;
    return (true);
}

QString TableTags::getIdObject()
{
    QSqlQuery                           query;
    QVector<QMap<QString, QVariant> >   result;

    query.prepare(Database::instance()->getQuery("TableTags", "getIdObject"));
    query.bindValue(":id", this->id);
    if (Database::instance()->query(query, result) && result.size() > 0)
        return (result[0]["id_object"].toString());
    return ("");
}

bool    TableTags::setIdObject(const QString &id_object)
{
    QSqlQuery   query;

    query.prepare(Database::instance()->getQuery("TableTags", "setIdObject"));
    query.bindValue(":id", this->id);
    query.bindValue(":id_object", id_object);
    return (Database::instance()->query(query));
}

QString TableTags::getName()
{
    QSqlQuery                           query;
    QVector<QMap<QString, QVariant> >   result;

    query.prepare(Database::instance()->getQuery("TableTags", "getName"));
    query.bindValue(":id", this->id);
    if (Database::instance()->query(query, result) && result.size() > 0)
        return (result[0]["name"].toString());
    return ("");
}

bool    TableTags::setName(const QString &name)
{
    QSqlQuery   query;

    query.prepare(Database::instance()->getQuery("TableTags", "setName"));
    query.bindValue(":id", this->id);
    query.bindValue(":name", name);
    return (Database::instance()->query(query));
}

bool            TableTags::unitTests()
{
    TableTags           t;
    TableDirectories    d1;
    TableDirectories    d2;
    QSqlQuery           query;

    Log::instance()->debug("Running unit tests...", "TableTags", "unitTests");
    query.prepare("DELETE FROM tags WHERE name IN('t1', 't2')");
    Database::instance()->query(query);
    query.prepare("DELETE FROM directories WHERE name IN('d1', 'd2')");
    Database::instance()->query(query);
    try
    {
        ASSERT(d1.add("d1"));
        ASSERT(d2.add("d2"));
        ASSERT(t.add(d2.getId(), "t1"));
        ASSERT(d2.getTags().size() == 1);
        ASSERT(d2.getTags().contains(t.getId()));
        ASSERT(t.getIdObject() == d2.getId());
        ASSERT(t.setIdObject(d1.getId()));
        ASSERT(t.getIdObject() == d1.getId());
        ASSERT(t.getName() == "t1");
        ASSERT(t.setName("t2"));
        ASSERT(t.getName() == "t2");
        ASSERT(d2.remove());
        ASSERT(t.exists());
        ASSERT(d1.remove());
        ASSERT(!t.exists());
    }
    catch (Properties properties)
    {
        Log::instance()->error("Unit test failed", properties, "TableTags", "unitTests");
        return (false);
    }
    Log::instance()->debug("Unit tests successful!", "TableTags", "unitTests");
    return (true);
}
