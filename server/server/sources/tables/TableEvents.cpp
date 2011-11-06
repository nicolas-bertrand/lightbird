#include "Database.h"
#include "Defines.h"
#include "TableEvents.h"
#include "Tools.h"

TableEvents::TableEvents(const QString &id)
{
    this->tableName = "events";
    this->tableId = LightBird::ITable::Events;
    this->setId(id);
}

TableEvents::~TableEvents()
{
}

TableEvents::TableEvents(const TableEvents &table)
{
    *this = table;
}

TableEvents &TableEvents::operator=(const TableEvents &table)
{
    Table::operator=(table);
    return (*this);
}

QStringList TableEvents::getEvents(const QString &name, const QDateTime &b, const QDateTime &e) const
{
    QSqlQuery               query;
    QVector<QVariantMap>    response;
    int                     i;
    int                     s;
    QStringList             result;
    QString                 begin;
    QString                 end;

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

bool            TableEvents::add(const QString &name, const QVariantMap &informations,
                                 const QString &id_accessor, const QString &id_object)
{
    QSqlQuery   query;
    QString     id;

    id = Tools::createUuid();
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
    return (this->add(name, QVariantMap(), id_accessor, id_object));
}

QString     TableEvents::getName() const
{
    QSqlQuery               query;
    QVector<QVariantMap>    result;

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

QString     TableEvents::getIdAccessor() const
{
    QSqlQuery               query;
    QVector<QVariantMap>    result;

    query.prepare(Database::instance()->getQuery("TableEvents", "getIdAccessor"));
    query.bindValue(":id", this->id);
    if (Database::instance()->query(query, result) && result.size() > 0)
        return (result[0]["id_accessor"].toString());
    return ("");
}

bool            TableEvents::setIdAccessor(const QString &id_accessor)
{
    QSqlQuery   query;

    query.prepare(Database::instance()->getQuery("TableEvents", "setIdAccessor"));
    query.bindValue(":id", this->id);
    query.bindValue(":id_accessor", id_accessor);
    return (Database::instance()->query(query));
}

QString     TableEvents::getIdObject() const
{
    QSqlQuery               query;
    QVector<QVariantMap>    result;

    query.prepare(Database::instance()->getQuery("TableEvents", "getIdObject"));
    query.bindValue(":id", this->id);
    if (Database::instance()->query(query, result) && result.size() > 0)
        return (result[0]["id_object"].toString());
    return ("");
}

bool            TableEvents::setIdObject(const QString &id_object)
{
    QSqlQuery   query;

    query.prepare(Database::instance()->getQuery("TableEvents", "setIdObject"));
    query.bindValue(":id", this->id);
    query.bindValue(":id_object", id_object);
    return (Database::instance()->query(query));
}

QVariant    TableEvents::getInformation(const QString &name) const
{
    QSqlQuery               query;
    QVector<QVariantMap>    result;

    query.prepare(Database::instance()->getQuery("TableEvents", "getInformation"));
    query.bindValue(":id_event", this->id);
    query.bindValue(":name", name);
    if (Database::instance()->query(query, result) && result.size() > 0)
        return (result.value(0).value("value").toString());
    return ("");
}

QMap<QString, QVariant> TableEvents::getInformations() const
{
    QSqlQuery               query;
    QVector<QVariantMap>    result;
    QVariantMap             informations;
    int                     i;
    int                     s;

    query.prepare(Database::instance()->getQuery("TableEvents", "getInformations"));
    query.bindValue(":id_event", this->id);
    if (Database::instance()->query(query, result))
        for (i = 0, s = result.size(); i < s; ++i)
            informations.insert(result[i]["name"].toString(), result[i]["value"]);
    return (informations);
}

bool        TableEvents::setInformation(const QString &name, const QVariant &value)
{
    QSqlQuery               query;
    QVector<QVariantMap>    result;

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
        query.bindValue(":id", Tools::createUuid());
        query.bindValue(":id_event", this->id);
        query.bindValue(":name", name);
        query.bindValue(":value", value);
    }
    return (Database::instance()->query(query));
}

bool        TableEvents::setInformations(const QVariantMap &informations)
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

bool            TableEvents::removeInformations(const QStringList &informations)
{
    QSqlQuery   query;
    bool        result = true;

    if (informations.isEmpty())
    {
        query.prepare(Database::instance()->getQuery("TableEvents", "removeInformations"));
        query.bindValue(":id_event", this->id);
        result = Database::instance()->query(query);
    }
    else
    {
        QStringListIterator it(informations);
        while (it.hasNext())
            result = this->removeInformation(it.next());
    }
    return (result);
}
