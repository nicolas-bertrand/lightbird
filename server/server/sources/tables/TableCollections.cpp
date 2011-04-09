#include <QUuid>
#include <QDir>

#include "Configurations.h"
#include "Database.h"
#include "Defines.h"
#include "Log.h"
#include "TableAccounts.h"
#include "TableFiles.h"
#include "TablePermissions.h"
#include "TableCollections.h"

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

QString TableCollections::getIdFromVirtualPath(const QString &virtualPath)
{
    QSqlQuery                           query;
    QVector<QMap<QString, QVariant> >   result;
    QString                             path;
    QString                             id_collection = "";

    path = QDir::cleanPath(virtualPath);
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

    id = QUuid::createUuid().toString().remove(0, 1).remove(36, 1);
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

QString TableCollections::getIdCollection()
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

QString TableCollections::getVirtualPath(bool initialSlash, bool finalSlash)
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

    path = QDir::cleanPath(virtualPath);
    if (path.count('/') == path.size())
        return (this->setIdCollection(""));
    if ((id_collection = TableCollections().getIdFromVirtualPath(path)).isEmpty())
        return (false);
    return (this->setIdCollection(id_collection));
}

QStringList TableCollections::getCollections(const QString &id_accessor, const QString &right)
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

QStringList TableCollections::getFiles(const QString &id_accessor, const QString &right)
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

bool                    TableCollections::unitTests()
{
    QSqlQuery           query;
    TableCollections    c1;
    QStringList         l;
    TableAccounts       a;
    TableFiles          f;
    TablePermissions    p;

    Log::instance()->debug("Running unit tests...", "TableCollections", "unitTests");
    query.prepare("DELETE FROM collections WHERE name IN('videos', 'images', 'egypte', 'spiders')");
    Database::instance()->query(query);
    query.prepare("DELETE FROM accounts WHERE name IN('a')");
    Database::instance()->query(query);
    query.prepare("DELETE FROM files WHERE name IN('toto.xml', 'titi.xml')");
    Database::instance()->query(query);
    Configurations::instance()->set("permissions/activate", "true");
    Configurations::instance()->set("permissions/default", "false");
    Configurations::instance()->set("permissions/inheritance", "true");
    Configurations::instance()->set("permissions/ownerInheritance", "true");
    try
    {
        ASSERT(c1.add("videos"));
        ASSERT(c1.exists());
        ASSERT(c1.exists(c1.getId()));
        ASSERT(!c1.exists("toto"));
        ASSERT(c1.add("pictures"));
        ASSERT(c1.getName() == "pictures");
        TableCollections c2(c1.getId());
        ASSERT(c2.getName() == "pictures");
        ASSERT(c1.setName("images"));
        ASSERT(c2.getName() == "images");
        ASSERT(c1.getIdCollection().isEmpty());
        ASSERT(c2.getIdAccount().isEmpty());
        ASSERT(c2.add("bahrain", c1.getId()));
        ASSERT(c2.getIdCollection() == c1.getId());
        ASSERT(c2.add("france", c1.getId()));
        ASSERT(c2.add("spiders", c2.getId()));
        ASSERT(c2.add("egypte", c2.getIdCollection()));
        ASSERT(c2.getVirtualPath() == "images/france/egypte");
        ASSERT(c2.getVirtualPath(true, true) == "/images/france/egypte/");
        ASSERT(c1.getVirtualPath(true) == "/images");
        ASSERT(c2.setIdCollection(c1.getId()));
        ASSERT(c2.getIdCollection() == c1.getId());
        ASSERT(c2.setIdCollection(c1.getIdCollection()));
        ASSERT(c2.getIdCollection().isEmpty());
        ASSERT(c2.setVirtualPath("////\\\\/images///france/\\\\"));
        ASSERT(c2.getIdCollection() == c1.getIdFromVirtualPath("///\\images///france\\"));
        ASSERT(c2.setVirtualPath("images"));
        ASSERT(c2.getIdCollection() == c1.getId());
        ASSERT(f.add("toto.xml", "/", ""));
        ASSERT(!f.getCollections().size());
        ASSERT(f.addCollection(c1.getId()));
        ASSERT((l = f.getCollections()).size() == 1);
        ASSERT(l.contains(c1.getId()));
        ASSERT(f.add("titi.xml", "/", ""));
        ASSERT(f.addCollection(c1.getId()));
        ASSERT((l = c1.getCollections()).size() == 3);
        ASSERT(l.contains(c1.getIdFromVirtualPath("images/france")));
        ASSERT(l.contains(c1.getIdFromVirtualPath("images/egypte")));
        ASSERT(l.contains(c1.getIdFromVirtualPath("images/bahrain")));
        ASSERT((l = c1.getFiles()).size() == 2);
        ASSERT(l.contains(f.getIdFromVirtualPath("toto.xml")));
        ASSERT(l.contains(f.getIdFromVirtualPath("\\///titi.xml")));
        ASSERT(a.add("a"));
        ASSERT(c1.getCollections(a.getId(), "read").size() == 0);
        ASSERT(c1.getFiles(a.getId(), "read").size() == 0);
        ASSERT(p.add(a.getId(), c2.getId(), "read"));
        ASSERT(p.add(a.getId(), f.getId(), "read"));
        ASSERT((l = c1.getCollections(a.getId(), "read")).size() == 1);
        ASSERT(l.contains(c2.getId()));
        ASSERT((l = c1.getFiles(a.getId(), "read")).size() == 1);
        ASSERT(l.contains(f.getId()));
        ASSERT(f.removeCollection(c1.getId()));
        ASSERT(c2.remove(c1.getId()));
        ASSERT(!c2.remove());
    }
    catch (Properties properties)
    {
        Log::instance()->error("Unit test failed", properties, "TableCollections", "unitTests");
        return (false);
    }
    Log::instance()->debug("Unit tests successful!", "TableCollections", "unitTests");
    return (true);
}
