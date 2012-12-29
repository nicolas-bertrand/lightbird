#include "Library.h"
#include "LightBird.h"

using namespace LightBird;

TableTags::TableTags(const QString &id)
{
    this->tableName = "tags";
    this->tableId = Table::Tags;
    this->setId(id);
}

TableTags::~TableTags()
{
}

TableTags::TableTags(const TableTags &table)
    : Table()
{
    *this = table;
}

TableTags &TableTags::operator=(const TableTags &table)
{
    Table::operator=(table);
    return (*this);
}

bool    TableTags::add(const QString &id_object, const QString &name)
{
    QSqlQuery   query(Library::database().getDatabase());
    QString     id;

    id = createUuid();
    query.prepare(Library::database().getQuery("TableTags", "add"));
    query.bindValue(":id", id);
    query.bindValue(":id_object", id_object);
    query.bindValue(":name", name);
    if (!Library::database().query(query) || query.numRowsAffected() == 0)
        return (false);
    this->id = id;
    return (true);
}

QString TableTags::getIdObject() const
{
    QSqlQuery            query(Library::database().getDatabase());
    QVector<QVariantMap> result;

    query.prepare(Library::database().getQuery("TableTags", "getIdObject"));
    query.bindValue(":id", this->id);
    if (Library::database().query(query, result) && result.size() > 0)
        return (result[0]["id_object"].toString());
    return ("");
}

bool    TableTags::setIdObject(const QString &id_object)
{
    QSqlQuery   query(Library::database().getDatabase());

    query.prepare(Library::database().getQuery("TableTags", "setIdObject"));
    query.bindValue(":id", this->id);
    query.bindValue(":id_object", id_object);
    return (Library::database().query(query));
}

QString TableTags::getName() const
{
    QSqlQuery            query(Library::database().getDatabase());
    QVector<QVariantMap> result;

    query.prepare(Library::database().getQuery("TableTags", "getName"));
    query.bindValue(":id", this->id);
    if (Library::database().query(query, result) && result.size() > 0)
        return (result[0]["name"].toString());
    return ("");
}

bool    TableTags::setName(const QString &name)
{
    QSqlQuery   query(Library::database().getDatabase());

    query.prepare(Library::database().getQuery("TableTags", "setName"));
    query.bindValue(":id", this->id);
    query.bindValue(":name", name);
    return (Library::database().query(query));
}
