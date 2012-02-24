#include "Library.h"
#include "LightBird.h"

using namespace LightBird;

TableDirectories::TableDirectories(const QString &id)
{
    this->tableName = "directories";
    this->tableId = Table::Directories;
    this->setId(id);
}

TableDirectories::~TableDirectories()
{
}

TableDirectories::TableDirectories(const TableDirectories &table) : TableObjects()
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
    QSqlQuery               query;
    QVector<QVariantMap>    result;
    QString                 path;
    QString                 id_directory = "";

    path = cleanPath(virtualPath);
    QStringListIterator it(path.split('/'));
    while (it.hasNext())
    {
        if (!it.peekNext().isEmpty())
        {
            query.prepare(Library::database().getQuery("TableDirectories", "getIdFromVirtualPath"));
            query.bindValue(":id_directory", id_directory);
            query.bindValue(":name", it.peekNext());
            if (!Library::database().query(query, result) || result.size() <= 0)
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
    QSqlQuery               query;
    QVector<QVariantMap>    result;
    QString                 id;

    id = createUuid();
    query.prepare(Library::database().getQuery("TableDirectories", "add"));
    query.bindValue(":id", id);
    query.bindValue(":name", name);
    query.bindValue(":id_directory", id_directory);
    query.bindValue(":id_account", id_account);
    if (!Library::database().query(query))
        return (false);
    this->id = id;
    return (true);
}

QString TableDirectories::getIdDirectory() const
{
    QSqlQuery               query;
    QVector<QVariantMap>    result;

    query.prepare(Library::database().getQuery("TableDirectories", "getIdDirectory"));
    query.bindValue(":id", this->id);
    if (Library::database().query(query, result) && result.size() > 0)
        return (result[0]["id_directory"].toString());
    return ("");
}

bool    TableDirectories::setIdDirectory(const QString &id_directory)
{
    QSqlQuery   query;

    query.prepare(Library::database().getQuery("TableDirectories", "setIdDirectory"));
    query.bindValue(":id", this->id);
    query.bindValue(":id_directory", id_directory);
    return (Library::database().query(query));
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

    path = cleanPath(virtualPath);
    if (path.count('/') == path.size())
        return (this->setIdDirectory(""));
    if ((id_directory = TableDirectories().getIdFromVirtualPath(path)).isEmpty())
        return (false);
    return (this->setIdDirectory(id_directory));
}

QStringList TableDirectories::getDirectories(const QString &id_accessor, const QString &right) const
{
    QSqlQuery               query;
    QVector<QVariantMap>    result;
    QStringList             directories;
    int                     i;
    int                     s;
    TablePermissions        p;

    query.prepare(Library::database().getQuery("TableDirectories", "getDirectories"));
    query.bindValue(":id_directory", this->id);
    Library::database().query(query, result);
    for (i = 0, s = result.size(); i < s; ++i)
        if (id_accessor.isEmpty() || p.isAllowed(id_accessor, result[i]["id"].toString(), right))
            directories << result[i]["id"].toString();
    return (directories);
}

QStringList TableDirectories::getFiles(const QString &id_accessor, const QString &right) const
{
    QSqlQuery               query;
    QVector<QVariantMap>    result;
    QStringList             files;
    int                     i;
    int                     s;
    TablePermissions        p;

    query.prepare(Library::database().getQuery("TableDirectories", "getFiles"));
    query.bindValue(":id_directory", this->id);
    Library::database().query(query, result);
    for (i = 0, s = result.size(); i < s; ++i)
        if (id_accessor.isEmpty() || p.isAllowed(id_accessor, result[i]["id"].toString(), right))
            files << result[i]["id"].toString();
    return (files);
}

QString     TableDirectories::getDirectory(const QString &name) const
{
    QSqlQuery               query;
    QVector<QVariantMap>    result;

    query.prepare(Library::database().getQuery("TableDirectories", "getDirectory"));
    query.bindValue(":id_directory", this->id);
    query.bindValue(":name", name);
    if (Library::database().query(query, result) && result.size() > 0)
        return (result[0]["id"].toString());
    return (QString());
}

QString     TableDirectories::getFile(const QString &name) const
{
    QSqlQuery               query;
    QVector<QVariantMap>    result;

    query.prepare(Library::database().getQuery("TableDirectories", "getFile"));
    query.bindValue(":id_directory", this->id);
    query.bindValue(":name", name);
    if (Library::database().query(query, result) && result.size() > 0)
        return (result[0]["id"].toString());
    return (QString());
}

QString                 TableDirectories::createVirtualPath(const QString &virtualPath, const QString &id_account)
{
    TableDirectories    directory;
    QString             id;

    directory.setId(this->getId());
    QStringListIterator it(cleanPath(virtualPath).split('/'));
    while (it.hasNext())
    {
        if (!it.peekNext().isEmpty())
        {
            if (!(id = directory.getDirectory(it.peekNext())).isEmpty())
                directory.setId(id);
            else if (!directory.add(it.peekNext(), directory.getId(), id_account))
                return (QString());
        }
        it.next();
    }
    return (directory.getId());
}