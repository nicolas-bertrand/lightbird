#include "Configurations.h"
#include "Database.h"
#include "TableDirectories.h"
#include "TablePermissions.h"
#include "Tools.h"

TableDirectories::TableDirectories(const QString &id)
{
    this->tableName = "directories";
    this->tableId = LightBird::ITable::Directories;
    this->setId(id);
}

TableDirectories::~TableDirectories()
{
}

TableDirectories::TableDirectories(const TableDirectories &table)
{
    *this = table;
}

TableDirectories &TableDirectories::operator=(const TableDirectories &table)
{
    TableObjects::operator=(table);
    return (*this);
}

QString TableDirectories::getIdFromVirtualPath(const QString &virtualPath) const
{
    QSqlQuery                           query;
    QVector<QMap<QString, QVariant> >   result;
    QString                             path;
    QString                             id_directory = "";

    path = Tools::cleanPath(virtualPath);
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

    id = Tools::createUuid();
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

QString TableDirectories::getIdDirectory() const
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

QString TableDirectories::getVirtualPath(bool initialSlash, bool finalSlash) const
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

    path = Tools::cleanPath(virtualPath);
    if (path.count('/') == path.size())
        return (this->setIdDirectory(""));
    if ((id_directory = TableDirectories().getIdFromVirtualPath(path)).isEmpty())
        return (false);
    return (this->setIdDirectory(id_directory));
}

QStringList TableDirectories::getDirectories(const QString &id_accessor, const QString &right) const
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

QStringList TableDirectories::getFiles(const QString &id_accessor, const QString &right) const
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
