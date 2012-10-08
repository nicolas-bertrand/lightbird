#include "Library.h"
#include "LightBird.h"

using namespace LightBird;

TableCollections::TableCollections(const QString &id)
{
    this->tableName = "collections";
    this->tableId = Table::Collections;
    this->setId(id);
}

TableCollections::~TableCollections()
{
}

TableCollections::TableCollections(const TableCollections &table) : TableObjects()
{
    *this = table;
}

TableCollections &TableCollections::operator=(const TableCollections &table)
{
    TableObjects::operator=(table);
    return (*this);
}

QString TableCollections::getIdFromVirtualPath(const QString &virtualPath, const QString &id_account) const
{
    QSqlQuery               query;
    QVector<QVariantMap>    result;
    QString                 path;
    QString                 id_collection = "";
    int                     row;

    path = cleanPath(virtualPath);
    QStringListIterator it(path.split('/'));
    while (it.hasNext())
    {
        if (!it.peekNext().isEmpty())
        {
            query.prepare(Library::database().getQuery("TableCollections", "getIdFromVirtualPath"));
            query.bindValue(":id_collection", id_collection);
            query.bindValue(":name", it.peekNext());
            if (!Library::database().query(query, result) || result.size() <= 0)
                return ("");
            row = 0;
            for (int i = 1; i < result.size() && row == 0; ++i)
                if (result[i]["id_account"] == id_account)
                    row = i;
            id_collection = result[row]["id"].toString();

        }
        it.next();
    }
    return (id_collection);
}

bool        TableCollections::setIdFromVirtualPath(const QString &virtualPath, const QString &id_account)
{
    QString id;

    if ((id = this->getIdFromVirtualPath(virtualPath, id_account)).isEmpty())
        return (false);
    this->id = id;
    return (true);
}

bool    TableCollections::add(const QString &name, const QString &id_collection, const QString &id_account)
{
    QSqlQuery   query;
    QString     id;

    if (!LightBird::isValidName(name))
        return (false);
    id = createUuid();
    query.prepare(Library::database().getQuery("TableCollections", "add"));
    query.bindValue(":id", id);
    query.bindValue(":name", name);
    query.bindValue(":id_collection", id_collection);
    query.bindValue(":id_account", id_account);
    if (!Library::database().query(query))
        return (false);
    this->id = id;
    return (true);
}

QString TableCollections::getIdCollection() const
{
    QSqlQuery               query;
    QVector<QVariantMap>    result;

    query.prepare(Library::database().getQuery("TableCollections", "getIdCollection"));
    query.bindValue(":id", this->id);
    if (Library::database().query(query, result) && result.size() > 0)
        return (result[0]["id_collection"].toString());
    return ("");
}

bool    TableCollections::setIdCollection(const QString &id_collection)
{
    QSqlQuery   query;

    query.prepare(Library::database().getQuery("TableCollections", "setIdCollection"));
    query.bindValue(":id", this->id);
    query.bindValue(":id_collection", id_collection);
    return (Library::database().query(query));
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

    path = cleanPath(virtualPath);
    if (path.count('/') == path.size())
        return (this->setIdCollection(""));
    if ((id_collection = TableCollections().getIdFromVirtualPath(path)).isEmpty())
        return (false);
    return (this->setIdCollection(id_collection));
}

QStringList TableCollections::getCollections(const QString &id_accessor, const QString &right) const
{
    QSqlQuery               query;
    QVector<QVariantMap>    result;
    QStringList             collections;
    int                     i;
    int                     s;
    TablePermissions        p;

    query.prepare(Library::database().getQuery("TableCollections", "getCollections"));
    query.bindValue(":id_collection", this->id);
    Library::database().query(query, result);
    for (i = 0, s = result.size(); i < s; ++i)
        if (id_accessor.isEmpty() || p.isAllowed(id_accessor, result[i]["id"].toString(), right))
            collections << result[i]["id"].toString();
    return (collections);
}

QStringList TableCollections::getFiles(const QString &id_accessor, const QString &right) const
{
    QSqlQuery               query;
    QVector<QVariantMap>    result;
    QStringList             files;
    int                     i;
    int                     s;
    TablePermissions        p;

    query.prepare(Library::database().getQuery("TableCollections", "getFiles"));
    query.bindValue(":id_collection", this->id);
    Library::database().query(query, result);
    for (i = 0, s = result.size(); i < s; ++i)
        if (id_accessor.isEmpty() || p.isAllowed(id_accessor, result[i]["id_file"].toString(), right))
            files << result[i]["id_file"].toString();
    return (files);
}
