#include "Database.h"
#include "TableTags.h"
#include "Tools.h"

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

    id = Tools::createUuid();
    query.prepare(Database::instance()->getQuery("TableTags", "add"));
    query.bindValue(":id", id);
    query.bindValue(":id_object", id_object);
    query.bindValue(":name", name);
    if (!Database::instance()->query(query) || query.numRowsAffected() == 0)
        return (false);
    this->id = id;
    return (true);
}

QString TableTags::getIdObject() const
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

QString TableTags::getName() const
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
