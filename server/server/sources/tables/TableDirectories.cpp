#include <QUuid>
#include <QDir>

#include "Configurations.h"
#include "Database.h"
#include "Defines.h"
#include "Log.h"
#include "TableAccounts.h"
#include "TableFiles.h"
#include "TablePermissions.h"
#include "TableDirectories.h"

TableDirectories::TableDirectories(const QString &id)
{
    this->tableName = "directories";
    this->tableId = Streamit::ITable::Directories;
    if (!id.isEmpty())
        Table::setId(id);
}

TableDirectories::~TableDirectories()
{
}

TableDirectories::TableDirectories(const TableDirectories &t) : Table(), TableObjects()
{
    *this = t;
}

TableDirectories &TableDirectories::operator=(const TableDirectories &t)
{
    if (this != &t)
    {
        this->id = t.id;
        this->tableId = t.tableId;
        this->tableName = t.tableName;
    }
    return (*this);
}

QString TableDirectories::getIdFromVirtualPath(const QString &virtualPath)
{
    QSqlQuery                           query;
    QVector<QMap<QString, QVariant> >   result;
    QString                             path;
    QString                             id_directory = "";

    path = QDir::cleanPath(virtualPath);
    QStringListIterator it(path.split('/'));
    while (it.hasNext())
    {
        if (!it.peekNext().isEmpty())
        {
            result.clear();
            query.prepare(Database::instance()->getQuery("TableDirectories", "getIdFromVirtualPath"));
            query.bindValue(":id_directory", id_directory);
            query.bindValue(":name", it.peekNext());
            if (!Database::instance()->query(query, result) || result.size() <= 0)
                return ("");
            id_directory = result[0]["id"].toString();
        }
        it.next();
    }
    return (id_directory);
}

bool        TableDirectories::setIdFromVirtualPath(const QString &virtualPath)
{
    QString id;

    if ((id = this->getIdFromVirtualPath(virtualPath)).isEmpty())
        return (false);
    this->id = id;
    return (true);
}

bool    TableDirectories::add(const QString &name, const QString &id_directory, const QString &id_account)
{
    QSqlQuery                           query;
    QVector<QMap<QString, QVariant> >   result;
    QString                             id;

    id = QUuid::createUuid().toString().remove(0, 1).remove(36, 1);
    query.prepare(Database::instance()->getQuery("TableDirectories", "add"));
    query.bindValue(":id", id);
    query.bindValue(":name", name);
    query.bindValue(":id_directory", id_directory);
    query.bindValue(":id_account", id_account);
    if (!Database::instance()->query(query))
        return (false);
    this->id = id;
    return (true);
}

QString TableDirectories::getIdDirectory()
{
    QSqlQuery                           query;
    QVector<QMap<QString, QVariant> >   result;

    query.prepare(Database::instance()->getQuery("TableDirectories", "getIdDirectory"));
    query.bindValue(":id", this->id);
    if (Database::instance()->query(query, result) && result.size() > 0)
        return (result[0]["id_directory"].toString());
    return ("");
}

bool    TableDirectories::setIdDirectory(const QString &id_directory)
{
    QSqlQuery   query;

    query.prepare(Database::instance()->getQuery("TableDirectories", "setIdDirectory"));
    query.bindValue(":id", this->id);
    query.bindValue(":id_directory", id_directory);
    return (Database::instance()->query(query));
}

QString TableDirectories::getVirtualPath(bool initialSlash, bool finalSlash)
{
    TableDirectories    directory;
    QString             id_directory;
    QString             virtualPath;

    id_directory = this->getIdDirectory();
    virtualPath = this->getName();
    while (!id_directory.isEmpty())
    {
        if (!directory.setId(id_directory))
            return ("");
        virtualPath.prepend(directory.getName() + "/");
        id_directory = directory.getIdDirectory();
    }
    if (initialSlash)
        virtualPath.prepend("/");
    if (finalSlash)
        virtualPath.append("/");
    if (virtualPath == "//" && initialSlash && finalSlash)
        virtualPath = "/";
    return (virtualPath);
}

bool        TableDirectories::setVirtualPath(const QString &virtualPath)
{
    QString id_directory;
    QString path;

    path = QDir::cleanPath(virtualPath);
    if (path.count('/') == path.size())
        return (this->setIdDirectory(""));
    if ((id_directory = TableDirectories().getIdFromVirtualPath(path)).isEmpty())
        return (false);
    return (this->setIdDirectory(id_directory));
}

QStringList TableDirectories::getDirectories(const QString &id_accessor, const QString &right)
{
    QSqlQuery                           query;
    QVector<QMap<QString, QVariant> >   result;
    QStringList                         directories;
    int                                 i;
    int                                 s;
    TablePermissions                    p;

    query.prepare(Database::instance()->getQuery("TableDirectories", "getDirectories"));
    query.bindValue(":id_directory", this->id);
    Database::instance()->query(query, result);
    for (i = 0, s = result.size(); i < s; ++i)
        if (id_accessor.isEmpty() || p.isAllowed(id_accessor, result[i]["id"].toString(), right))
            directories << result[i]["id"].toString();
    return (directories);
}

QStringList TableDirectories::getFiles(const QString &id_accessor, const QString &right)
{
    QSqlQuery                           query;
    QVector<QMap<QString, QVariant> >   result;
    QStringList                         files;
    int                                 i;
    int                                 s;
    TablePermissions                    p;

    query.prepare(Database::instance()->getQuery("TableDirectories", "getFiles"));
    query.bindValue(":id_directory", this->id);
    Database::instance()->query(query, result);
    for (i = 0, s = result.size(); i < s; ++i)
        if (id_accessor.isEmpty() || p.isAllowed(id_accessor, result[i]["id"].toString(), right))
            files << result[i]["id"].toString();
    return (files);
}

bool                    TableDirectories::unitTests()
{
    QSqlQuery           query;
    TableDirectories    d1;
    TableFiles          f;
    TableAccounts       a;
    TablePermissions    p;
    QStringList         l;

    Log::instance()->debug("Running unit tests...", "TableDirectories", "unitTests");
    query.prepare("DELETE FROM directories WHERE name IN('videos', 'images', 'egypte', 'spiders')");
    Database::instance()->query(query);
    query.prepare("DELETE FROM files WHERE name IN('toto.png', 'titi.png')");
    Database::instance()->query(query);
    query.prepare("DELETE FROM accounts WHERE name IN('a')");
    Database::instance()->query(query);
    Configurations::instance()->set("permissions/activate", "true");
    Configurations::instance()->set("permissions/default", "false");
    Configurations::instance()->set("permissions/inheritance", "true");
    Configurations::instance()->set("permissions/ownerInheritance", "true");
    try
    {
        ASSERT(d1.add("videos"));
        ASSERT(d1.exists());
        ASSERT(d1.exists(d1.getId()));
        ASSERT(!d1.exists("toto"));
        ASSERT(d1.add("pictures"));
        ASSERT(d1.getName() == "pictures");
        TableDirectories d2(d1.getId());
        ASSERT(d2.getName() == "pictures");
        ASSERT(d1.setName("images"));
        ASSERT(d2.getName() == "images");
        ASSERT(d1.getIdDirectory().isEmpty());
        ASSERT(d2.getIdAccount().isEmpty());
        ASSERT(d2.add("bahrain", d1.getId()));
        ASSERT(d2.getIdDirectory() == d1.getId());
        ASSERT(d2.add("france", d1.getId()));
        ASSERT(d2.add("spiders", d2.getId()));
        ASSERT(d2.add("egypte", d2.getIdDirectory()));
        ASSERT(d2.getVirtualPath() == "images/france/egypte");
        ASSERT(d2.getVirtualPath(true, true) == "/images/france/egypte/");
        ASSERT(d1.getVirtualPath(true) == "/images");
        ASSERT(d2.setIdDirectory(d1.getId()));
        ASSERT(d2.getIdDirectory() == d1.getId());
        ASSERT(d2.setIdDirectory(d1.getIdDirectory()));
        ASSERT(d2.getIdDirectory().isEmpty());
        ASSERT(d2.setVirtualPath("////\\\\/images///france/\\\\"));
        ASSERT(d2.getIdDirectory() == d1.getIdFromVirtualPath("///\\images///france\\"));
        ASSERT(d2.setVirtualPath("images"));
        ASSERT(d2.getIdDirectory() == d1.getId())
        ASSERT(f.add("toto.png", "/", "", d1.getId()));
        ASSERT(f.add("titi.png", "/", "", d1.getId()));
        ASSERT(a.add("a"));
        ASSERT((l = d1.getDirectories()).size() == 3);
        ASSERT(l.contains(d1.getIdFromVirtualPath("images/france")));
        ASSERT(l.contains(d1.getIdFromVirtualPath("images/egypte")));
        ASSERT(l.contains(d1.getIdFromVirtualPath("images/bahrain")));
        ASSERT((l = d1.getFiles()).size() == 2);
        ASSERT(l.contains(f.getIdFromVirtualPath("images/toto.png")));
        ASSERT(l.contains(f.getIdFromVirtualPath("images/titi.png")));
        ASSERT(!d1.getDirectories(a.getId(), "read").size());
        ASSERT(!d1.getFiles(a.getId(), "read").size());
        ASSERT(p.add(a.getId(), d1.getId(), "read"));
        ASSERT(d1.getDirectories(a.getId(), "read").size() == 3);
        ASSERT(d1.getFiles(a.getId(), "read").size() == 2);
        ASSERT(d1.isAllowed(a.getId(), "read"));
        ASSERT((l = d1.getRights(a.getId())).size() == 1);
        ASSERT(l.contains("read"));
        ASSERT(p.add(a.getId(), d1.getIdFromVirtualPath("images/france"), "read", false));
        ASSERT((l = d1.getDirectories(a.getId(), "read")).size() == 2);
        ASSERT(!l.contains(d1.getIdFromVirtualPath("images/france")));
        ASSERT(l.contains(d1.getIdFromVirtualPath("images/bahrain")));
        ASSERT(p.add(a.getId(), f.getIdFromVirtualPath("images/toto.png"), "read", false));
        ASSERT((l = d1.getFiles(a.getId(), "read")).size() == 1);
        ASSERT(!l.contains(f.getIdFromVirtualPath("images/toto.png")));
        ASSERT(l.contains(f.getIdFromVirtualPath("images/titi.png")));
        ASSERT(d2.remove(d1.getId()));
        ASSERT(!d2.remove());
    }
    catch (Properties properties)
    {
        Log::instance()->error("Unit test failed", properties, "TableDirectories", "unitTests");
        return (false);
    }
    Log::instance()->debug("Unit tests successful!", "TableDirectories", "unitTests");
    return (true);
}
