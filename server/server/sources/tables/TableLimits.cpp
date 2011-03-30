#include <QUuid>

#include "Defines.h"
#include "Log.h"
#include "Database.h"
#include "TableGroups.h"
#include "TableLimits.h"

TableLimits::TableLimits(const QString &id)
{
    this->tableName = "limits";
    this->tableId = Streamit::ITable::Limits;
    if (!id.isEmpty())
        this->setId(id);
}

TableLimits::~TableLimits()
{
}

TableLimits::TableLimits(const TableLimits &t) : Table()
{
    *this = t;
}

TableLimits &TableLimits::operator=(const TableLimits &t)
{
    if (this != &t)
    {
        this->id = t.id;
        this->tableId = t.tableId;
        this->tableName = t.tableName;
    }
    return (*this);
}

bool    TableLimits::add(const QString &name, const QString &value, const QString &id_accessor)
{
    QSqlQuery   query;
    QString     id;

    id = QUuid::createUuid().toString().remove(0, 1).remove(36, 1);
    query.prepare(Database::instance()->getQuery("TableLimits", "add"));
    query.bindValue(":id", id);
    query.bindValue(":name", name);
    query.bindValue(":value", value);
    query.bindValue(":id_accessor", id_accessor);
    if (!Database::instance()->query(query) || query.numRowsAffected() == 0)
        return (false);
    this->id = id;
    return (true);
}

QString TableLimits::getName()
{
    QSqlQuery                           query;
    QVector<QMap<QString, QVariant> >   result;

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

QString TableLimits::getValue()
{
    QSqlQuery                           query;
    QVector<QMap<QString, QVariant> >   result;

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

QString TableLimits::getIdAccessor()
{
    QSqlQuery                           query;
    QVector<QMap<QString, QVariant> >   result;

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

bool            TableLimits::unitTests()
{
    TableLimits l;
    TableGroups g1;
    TableGroups g2;
    QSqlQuery   query;

    Log::instance()->debug("Running unit tests...", "TableLimits", "unitTests");
    query.prepare("DELETE FROM limits WHERE name IN('l1', 'l2')");
    Database::instance()->query(query);
    query.prepare("DELETE FROM groups WHERE name IN('g1', 'g2')");
    Database::instance()->query(query);
    try
    {
        ASSERT(g1.add("g1"));
        ASSERT(g2.add("g2"));
        ASSERT(l.add("l1", "v1", g2.getId()));
        ASSERT(g2.getLimits().size() == 1);
        ASSERT(g2.getLimits().contains(l.getId()));
        ASSERT(l.getName() == "l1");
        ASSERT(l.setName("l2"));
        ASSERT(l.getName() == "l2");
        ASSERT(l.setName("l3"));
        ASSERT(l.getName() == "l3");
        ASSERT(l.getValue() == "v1");
        ASSERT(l.setValue("v2"));
        ASSERT(l.getValue() == "v2");
        ASSERT(l.setValue("l4"));
        ASSERT(l.getValue() == "l4");
        ASSERT(l.getIdAccessor() == g2.getId());
        ASSERT(l.setIdAccessor(""));
        ASSERT(l.getIdAccessor().isEmpty());
        ASSERT(l.setIdAccessor(g1.getId()));
        ASSERT(l.getIdAccessor() == g1.getId());
        ASSERT(g2.remove());
        ASSERT(l.exists());
        ASSERT(g1.remove());
        ASSERT(!l.exists());
    }
    catch (Properties properties)
    {
        Log::instance()->error("Unit test failed", properties, "TableLimits", "unitTests");
        return (false);
    }
    Log::instance()->debug("Unit tests successful!", "TableLimits", "unitTests");
    return (true);
}
