#include "Defines.h"
#include "Library.h"
#include "LightBird.h"

using namespace LightBird;

TableEvents::TableEvents(const QString &id)
{
    this->tableName = "events";
    this->tableId = Table::Events;
    this->setId(id);
}

TableEvents::~TableEvents()
{
}

TableEvents::TableEvents(const TableEvents &table) : Table()
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

    query.prepare(Library::database().getQuery("TableEvents", "getEvents"));
    begin = b.toString(DATE_FORMAT);
    if (begin.isEmpty())
        begin = "1970-01-01 00:00:00";
    end = e.toString(DATE_FORMAT);
    query.bindValue(":name", name);
    query.bindValue(":begin", begin);
    query.bindValue(":end", end);
    if (!Library::database().query(query, response))
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

    id = createUuid();
    query.prepare(Library::database().getQuery("TableEvents", "add"));
    query.bindValue(":id", id);
    query.bindValue(":name", name);
    query.bindValue(":id_accessor", id_accessor);
    query.bindValue(":id_object", id_object);
    if (!Library::database().query(query) || query.numRowsAffected() == 0)
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

    query.prepare(Library::database().getQuery("TableEvents", "getName"));
    query.bindValue(":id", this->id);
    if (Library::database().query(query, result) && result.size() > 0)
        return (result[0]["name"].toString());
    return ("");
}

bool            TableEvents::setName(const QString &name)
{
    QSqlQuery   query;

    if (name.isEmpty())
        return (false);
    query.prepare(Library::database().getQuery("TableEvents", "setName"));
    query.bindValue(":id", this->id);
    query.bindValue(":name", name);
    return (Library::database().query(query));
}

QString     TableEvents::getIdAccessor() const
{
    QSqlQuery               query;
    QVector<QVariantMap>    result;

    query.prepare(Library::database().getQuery("TableEvents", "getIdAccessor"));
    query.bindValue(":id", this->id);
    if (Library::database().query(query, result) && result.size() > 0)
        return (result[0]["id_accessor"].toString());
    return ("");
}

bool            TableEvents::setIdAccessor(const QString &id_accessor)
{
    QSqlQuery   query;

    query.prepare(Library::database().getQuery("TableEvents", "setIdAccessor"));
    query.bindValue(":id", this->id);
    query.bindValue(":id_accessor", id_accessor);
    return (Library::database().query(query));
}

QString     TableEvents::getIdObject() const
{
    QSqlQuery               query;
    QVector<QVariantMap>    result;

    query.prepare(Library::database().getQuery("TableEvents", "getIdObject"));
    query.bindValue(":id", this->id);
    if (Library::database().query(query, result) && result.size() > 0)
        return (result[0]["id_object"].toString());
    return ("");
}

bool            TableEvents::setIdObject(const QString &id_object)
{
    QSqlQuery   query;

    query.prepare(Library::database().getQuery("TableEvents", "setIdObject"));
    query.bindValue(":id", this->id);
    query.bindValue(":id_object", id_object);
    return (Library::database().query(query));
}

QVariant    TableEvents::getInformation(const QString &name) const
{
    QSqlQuery               query;
    QVector<QVariantMap>    result;

    query.prepare(Library::database().getQuery("TableEvents", "getInformation"));
    query.bindValue(":id_event", this->id);
    query.bindValue(":name", name);
    if (Library::database().query(query, result) && result.size() > 0)
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

    query.prepare(Library::database().getQuery("TableEvents", "getInformations"));
    query.bindValue(":id_event", this->id);
    if (Library::database().query(query, result))
        for (i = 0, s = result.size(); i < s; ++i)
            informations.insert(result[i]["name"].toString(), result[i]["value"]);
    return (informations);
}

bool        TableEvents::setInformation(const QString &name, const QVariant &value)
{
    QSqlQuery               query;
    QVector<QVariantMap>    result;

    query.prepare(Library::database().getQuery("TableEvents", "setInformation_select"));
    query.bindValue(":id_event", this->id);
    query.bindValue(":name", name);
    Library::database().query(query, result);
    if (result.size() > 0)
    {
        query.prepare(Library::database().getQuery("TableEvents", "setInformation_update"));
        query.bindValue(":value", value);
        query.bindValue(":id_event", this->id);
        query.bindValue(":name", name);
    }
    else
    {
        query.prepare(Library::database().getQuery("TableEvents", "setInformation_insert"));
        query.bindValue(":id", createUuid());
        query.bindValue(":id_event", this->id);
        query.bindValue(":name", name);
        query.bindValue(":value", value);
    }
    return (Library::database().query(query));
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

    query.prepare(Library::database().getQuery("TableEvents", "removeInformation"));
    query.bindValue(":id_event", this->id);
    query.bindValue(":name", name);
    return (Library::database().query(query) && query.numRowsAffected() > 0);
}

bool            TableEvents::removeInformations(const QStringList &informations)
{
    QSqlQuery   query;
    bool        result = true;

    if (informations.isEmpty())
    {
        query.prepare(Library::database().getQuery("TableEvents", "removeInformations"));
        query.bindValue(":id_event", this->id);
        result = Library::database().query(query);
    }
    else
    {
        QStringListIterator it(informations);
        while (it.hasNext())
            result = this->removeInformation(it.next());
    }
    return (result);
}
