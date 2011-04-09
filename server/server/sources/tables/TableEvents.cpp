#include <QCryptographicHash>
#include <QUuid>

#include "Defines.h"
#include "Log.h"
#include "Database.h"
#include "TableEvents.h"

TableEvents::TableEvents(const QString &id)
{
    this->tableName = "events";
    this->tableId = LightBird::ITable::Events;
    if (!id.isEmpty())
        Table::setId(id);
}

TableEvents::~TableEvents()
{
}

TableEvents::TableEvents(const TableEvents &t) : Table()
{
    *this = t;
}

TableEvents &TableEvents::operator=(const TableEvents &t)
{
    if (this != &t)
    {
        this->id = t.id;
        this->tableId = t.tableId;
        this->tableName = t.tableName;
    }
    return (*this);
}

QStringList TableEvents::getEvents(const QString &name, const QDateTime &b, const QDateTime &e)
{
    QSqlQuery                           query;
    QVector<QMap<QString, QVariant> >   response;
    int                                 i;
    int                                 s;
    QStringList                         result;
    QString                             begin;
    QString                             end;

    query.prepare(Database::instance()->getQuery("TableEvents", "getEvents"));
    begin = b.toString(DATE_FORMAT);
    if (begin.isEmpty())
        begin = "1970-01-01 00:00:00";
    end = e.toString(DATE_FORMAT);
    query.bindValue(":name", name);
    query.bindValue(":begin", begin);
    query.bindValue(":end", end);
    if (!Database::instance()->query(query, response))
        return (result);
    for (i = 0, s = response.size(); i < s; ++i)
        result << response[i]["id"].toString();
    return (result);
}

bool            TableEvents::add(const QString &name, const QMap<QString, QVariant> &informations,
                                 const QString &id_accessor, const QString &id_object)
{
    QSqlQuery   query;
    QString     id;

    id = QUuid::createUuid().toString().remove(0, 1).remove(36, 1);
    query.prepare(Database::instance()->getQuery("TableEvents", "add"));
    query.bindValue(":id", id);
    query.bindValue(":name", name);
    query.bindValue(":id_accessor", id_accessor);
    query.bindValue(":id_object", id_object);
    if (!Database::instance()->query(query) || query.numRowsAffected() == 0)
        return (false);
    this->id = id;
    if (!informations.isEmpty())
        this->setInformations(informations);
    return (true);
}

bool        TableEvents::add(const QString &name, const QString &id_accessor, const QString &id_object)
{
    return (this->add(name, QMap<QString, QVariant>(), id_accessor, id_object));
}

QString     TableEvents::getName()
{
    QSqlQuery                           query;
    QVector<QMap<QString, QVariant> >   result;

    query.prepare(Database::instance()->getQuery("TableEvents", "getName"));
    query.bindValue(":id", this->id);
    if (Database::instance()->query(query, result) && result.size() > 0)
        return (result[0]["name"].toString());
    return ("");
}

bool            TableEvents::setName(const QString &name)
{
    QSqlQuery   query;

    if (name.isEmpty())
        return (false);
    query.prepare(Database::instance()->getQuery("TableEvents", "setName"));
    query.bindValue(":id", this->id);
    query.bindValue(":name", name);
    return (Database::instance()->query(query));
}

QString     TableEvents::getIdAccessor()
{
    QSqlQuery                           query;
    QVector<QMap<QString, QVariant> >   result;

    query.prepare(Database::instance()->getQuery("TableEvents", "getIdAccessor"));
    query.bindValue(":id", this->id);
    if (Database::instance()->query(query, result) && result.size() > 0)
        return (result[0]["id_accessor"].toString());
    return ("");
}

bool        TableEvents::setIdAccessor(const QString &idAccessor)
{
    QSqlQuery   query;

    query.prepare(Database::instance()->getQuery("TableEvents", "setIdAccessor"));
    query.bindValue(":id", this->id);
    query.bindValue(":id_accessor", idAccessor);
    return (Database::instance()->query(query));
}

QString     TableEvents::getIdObject()
{
    QSqlQuery                           query;
    QVector<QMap<QString, QVariant> >   result;

    query.prepare(Database::instance()->getQuery("TableEvents", "getIdObject"));
    query.bindValue(":id", this->id);
    if (Database::instance()->query(query, result) && result.size() > 0)
        return (result[0]["id_object"].toString());
    return ("");
}

bool        TableEvents::setIdObject(const QString &idObject)
{
    QSqlQuery   query;

    query.prepare(Database::instance()->getQuery("TableEvents", "setIdObject"));
    query.bindValue(":id", this->id);
    query.bindValue(":id_object", idObject);
    return (Database::instance()->query(query));
}

QVariant    TableEvents::getInformation(const QString &name)
{
    QVector<QMap<QString, QVariant> >   result;
    QSqlQuery                           query;

    query.prepare(Database::instance()->getQuery("TableEvents", "getInformation"));
    query.bindValue(":id_event", this->id);
    query.bindValue(":name", name);
    if (Database::instance()->query(query, result) && result.size() > 0)
        return (result.value(0).value("value").toString());
    return ("");
}

bool        TableEvents::setInformation(const QString &name, const QVariant &value)
{
    QVector<QMap<QString, QVariant> >   result;
    QSqlQuery                           query;

    query.prepare(Database::instance()->getQuery("TableEvents", "setInformation_select"));
    query.bindValue(":id_event", this->id);
    query.bindValue(":name", name);
    Database::instance()->query(query, result);
    if (result.size() > 0)
    {
        query.prepare(Database::instance()->getQuery("TableEvents", "setInformation_update"));
        query.bindValue(":value", value);
        query.bindValue(":id_event", this->id);
        query.bindValue(":name", name);
    }
    else
    {
        query.prepare(Database::instance()->getQuery("TableEvents", "setInformation_insert"));
        query.bindValue(":id", QUuid::createUuid().toString().remove(0, 1).remove(36, 1));
        query.bindValue(":id_event", this->id);
        query.bindValue(":name", name);
        query.bindValue(":value", value);
    }
    return (Database::instance()->query(query));
}

QMap<QString, QVariant> TableEvents::getInformations()
{
    QVector<QMap<QString, QVariant> >   result;
    QSqlQuery                           query;
    QMap<QString, QVariant>             informations;
    int                                 i;
    int                                 s;

    query.prepare(Database::instance()->getQuery("TableEvents", "getInformations"));
    query.bindValue(":id_event", this->id);
    if (Database::instance()->query(query, result))
        for (i = 0, s = result.size(); i < s; ++i)
            informations.insert(result[i]["name"].toString(), result[i]["value"]);
    return (informations);
}

bool        TableEvents::setInformations(const QMap<QString, QVariant> &informations)
{
    QMapIterator<QString, QVariant> it(informations);
    bool                            result = true;

    while (it.hasNext() && result)
    {
        it.next();
        if (!this->setInformation(it.key(), it.value()))
            result = false;
    }
    return (result);
}

bool        TableEvents::removeInformation(const QString &name)
{
    QSqlQuery   query;

    query.prepare(Database::instance()->getQuery("TableEvents", "removeInformation"));
    query.bindValue(":id_event", this->id);
    query.bindValue(":name", name);
    return (Database::instance()->query(query) && query.numRowsAffected() > 0);
}

bool        TableEvents::removeInformations(const QStringList &informations)
{
    QStringListIterator it(informations);
    bool                result = true;

    while (it.hasNext())
    {
        it.next();
        if (!this->removeInformation(it.peekPrevious()))
            result = false;
    }
    return (result);
}

bool    TableEvents::unitTests()
{
    QMap<QString, QVariant> informations;
    QStringList             events;

    Log::instance()->debug("Running unit tests...", "TableEvents", "unitTests");
    try
    {
        TableEvents e1;
        ASSERT(e1.getId().isEmpty());
        informations["name1"] = "value1";
        informations["name2"] = "value2";
        informations["name3"] = "value3";
        ASSERT(e1.add("e1", informations, "", ""));
        ASSERT(!e1.getId().isEmpty());
        ASSERT(e1.getName() == "e1");
        ASSERT(e1.getCreated().isValid());
        ASSERT(e1.getIdAccessor().isEmpty());
        ASSERT(e1.getIdObject().isEmpty());
        ASSERT(e1.getInformation("name1") == "value1");
        ASSERT(e1.getInformation("name42").toString().isEmpty());
        ASSERT(e1.getInformations() == informations);
        TableEvents e2;
        ASSERT(e2.setId(e1.getId()));
        ASSERT(e2.setName("e2"));
        ASSERT(e2.getName() == "e2");
        ASSERT(e2.setIdAccessor(""));
        ASSERT(e2.getIdAccessor().isEmpty());
        ASSERT(e2.setIdObject(""));
        ASSERT(e2.getIdObject().isEmpty());
        ASSERT(e2.setInformation("name2", "value2new"));
        ASSERT(e2.getInformation("name2") == "value2new");
        ASSERT(e2.setInformation("name4", "value4"));
        ASSERT(e2.getInformation("name4") == "value4");
        informations.clear();
        informations["name1"] = "value1new";
        informations["name3"] = "value3new";
        informations["name5"] = "value5";
        ASSERT(e2.setInformations(informations));
        informations["name2"] = "value2new";
        informations["name4"] = "value4";
        ASSERT(e2.getInformations() == informations);
        ASSERT(e2.removeInformation("name5"));
        informations.remove("name5");
        ASSERT(e2.getInformations() == informations);
        ASSERT((events = e1.getEvents("e2", QDateTime::currentDateTime().addSecs(-40))).size());
        ASSERT(e1.setId(events.front()));
        ASSERT(e1.getName() == "e2");
        ASSERT(e2.remove());
        ASSERT(!e1.remove());
    }
    catch (Properties properties)
    {
        Log::instance()->error("Unit test failed", properties, "TableEvents", "unitTests");
        return (false);
    }
    Log::instance()->debug("Unit tests successful!", "TableEvents", "unitTests");
    return (true);
}
