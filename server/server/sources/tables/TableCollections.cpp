#include "Configurations.h"
#include "Database.h"
#include "TableCollections.h"
#include "TablePermissions.h"
#include "Tools.h"

TableCollections::TableCollections(const QString &id)
{
    this->tableName = "collections";
    this->tableId = LightBird::ITable::Collections;
    if (!id.isEmpty())
        Table::setId(id);
}

TableCollections::~TableCollections()
{
}

TableCollections::TableCollections(const TableCollections &t) : Table(), TableObjects()
{
    *this = t;
}

TableCollections &TableCollections::operator=(const TableCollections &t)
{
    if (this != &t)
    {
        this->id = t.id;
        this->tableId = t.tableId;
        this->tableName = t.tableName;
    }
    return (*this);
}

QString TableCollections::getIdFromVirtualPath(const QString &virtualPath) const
{
    QSqlQuery                           query;
    QVector<QMap<QString, QVariant> >   result;
    QString                             path;
    QString                             id_collection = "";

    path = Tools::cleanPath(virtualPath);
    QStringListIterator it(path.split('/'));
    while (it.hasNext())
    {
        if (!it.peekNext().isEmpty())
        {
            result.clear();
            query.prepare(Database::instance()->getQuery("TableCollections", "getIdFromVirtualPath"));
            query.bindValue(":id_collection", id_collection);
            query.bindValue(":name", it.peekNext());
            if (!Database::instance()->query(query, result) || result.size() <= 0)
                return ("");
            id_collection = result[0]["id"].toString();
        }
        it.next();
    }
    return (id_collection);
}

bool        TableCollections::setIdFromVirtualPath(const QString &virtualPath)
{
    QString id;

    if ((id = this->getIdFromVirtualPath(virtualPath)).isEmpty())
        return (false);
    this->id = id;
    return (true);
}

bool    TableCollections::add(const QString &name, const QString &id_collection, const QString &id_account)
{
    QSqlQuery                           query;
    QVector<QMap<QString, QVariant> >   result;
    QString                             id;

    id = Tools::createUuid();
    query.prepare(Database::instance()->getQuery("TableCollections", "add"));
    query.bindValue(":id", id);
    query.bindValue(":name", name);
    query.bindValue(":id_collection", id_collection);
    query.bindValue(":id_account", id_account);
    if (!Database::instance()->query(query))
        return (false);
    this->id = id;
    return (true);
}

QString TableCollections::getIdCollection() const
{
    QSqlQuery                           query;
    QVector<QMap<QString, QVariant> >   result;

    query.prepare(Database::instance()->getQuery("TableCollections", "getIdCollection"));
    query.bindValue(":id", this->id);
    if (Database::instance()->query(query, result) && result.size() > 0)
        return (result[0]["id_collection"].toString());
    return ("");
}

bool    TableCollections::setIdCollection(const QString &id_collection)
{
    QSqlQuery   query;

    query.prepare(Database::instance()->getQuery("TableCollections", "setIdCollection"));
    query.bindValue(":id", this->id);
    query.bindValue(":id_collection", id_collection);
    return (Database::instance()->query(query));
}

QString TableCollections::getVirtualPath(bool initialSlash, bool finalSlash) const
{
    TableCollections    collection;
    QString             id_collection;
    QString             virtualPath;

    id_collection = this->getIdCollection();
    virtualPath = this->getName();
    while (!id_collection.isEmpty())
    {
        if (!collection.setId(id_collection))
            return ("");
        virtualPath.prepend(collection.getName() + "/");
        id_collection = collection.getIdCollection();
    }
    if (initialSlash)
        virtualPath.prepend("/");
    if (finalSlash)
        virtualPath.append("/");
    if (virtualPath == "//" && initialSlash && finalSlash)
        virtualPath = "/";
    return (virtualPath);
}

bool    TableCollections::setVirtualPath(const QString &virtualPath)
{
    QString id_collection;
    QString path;

    path = Tools::cleanPath(virtualPath);
    if (path.count('/') == path.size())
        return (this->setIdCollection(""));
    if ((id_collection = TableCollections().getIdFromVirtualPath(path)).isEmpty())
        return (false);
    return (this->setIdCollection(id_collection));
}

QStringList TableCollections::getCollections(const QString &id_accessor, const QString &right) const
{
    QSqlQuery                           query;
    QVector<QMap<QString, QVariant> >   result;
    QStringList                         collections;
    int                                 i;
    int                                 s;
    TablePermissions                    p;

    query.prepare(Database::instance()->getQuery("TableCollections", "getCollections"));
    query.bindValue(":id_collection", this->id);
    Database::instance()->query(query, result);
    for (i = 0, s = result.size(); i < s; ++i)
        if (id_accessor.isEmpty() || p.isAllowed(id_accessor, result[i]["id"].toString(), right))
            collections << result[i]["id"].toString();
    return (collections);
}

QStringList TableCollections::getFiles(const QString &id_accessor, const QString &right) const
{
    QSqlQuery                           query;
    QVector<QMap<QString, QVariant> >   result;
    QStringList                         files;
    int                                 i;
    int                                 s;
    TablePermissions                    p;

    query.prepare(Database::instance()->getQuery("TableCollections", "getFiles"));
    query.bindValue(":id_collection", this->id);
    Database::instance()->query(query, result);
    for (i = 0, s = result.size(); i < s; ++i)
        if (id_accessor.isEmpty() || p.isAllowed(id_accessor, result[i]["id_file"].toString(), right))
            files << result[i]["id_file"].toString();
    return (files);
}
