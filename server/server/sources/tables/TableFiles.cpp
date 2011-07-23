#include <QFileInfo>

#include "Configurations.h"
#include "Database.h"
#include "TableFiles.h"
#include "TableDirectories.h"
#include "Tools.h"

TableFiles::TableFiles(const QString &id)
{
    this->tableName = "files";
    this->tableId = LightBird::ITable::Files;
    if (!id.isEmpty())
        Table::setId(id);
    this->types << "image" << "audio" << "video" << "document";
}

TableFiles::~TableFiles()
{
}

TableFiles::TableFiles(const TableFiles &t) : Table(), TableObjects()
{
    *this = t;
}

TableFiles &TableFiles::operator=(const TableFiles &t)
{
    if (this != &t)
    {
        this->id = t.id;
        this->tableId = t.tableId;
        this->tableName = t.tableName;
        this->types = t.types;
    }
    return (*this);
}

QString TableFiles::getIdFromVirtualPath(const QString &virtualPath)
{
    QSqlQuery                           query;
    QVector<QMap<QString, QVariant> >   result;
    QString                             name;
    QString                             path;
    QString                             id_directory = "";

    path = Tools::cleanPath(virtualPath);
    name = path;
    // If the virtual path has directories, get the id of the parent directory
    if (name.contains('/'))
    {
        path = path.left(name.lastIndexOf('/'));
        name = name.right(name.size() - path.size() - 1);
        id_directory = TableDirectories().getIdFromVirtualPath(path);
    }
    // Find the id of the file using its name, and the id of its parent directory, if it has one
    query.prepare(Database::instance()->getQuery("TableFiles", "getIdFromVirtualPath"));
    query.bindValue(":id_directory", id_directory);
    query.bindValue(":name", name);
    if (!Database::instance()->query(query, result) || result.size() <= 0)
        return ("");
    return (result[0]["id"].toString());
}

bool        TableFiles::setIdFromVirtualPath(const QString &virtualPath)
{
    QString id;

    if ((id = this->getIdFromVirtualPath(virtualPath)).isEmpty())
        return (false);
    this->id = id;
    return (true);
}

bool    TableFiles::add(const QString &name, const QString &path, const QMap<QString, QVariant> &informations,
                        const QString &type, const QString &id_directory, const QString &id_account)
{
    QSqlQuery   query;
    QString     id;

    id = Tools::createUuid();
    query.prepare(Database::instance()->getQuery("TableFiles", "add"));
    query.bindValue(":id", id);
    query.bindValue(":name", name);
    query.bindValue(":path", path);
    query.bindValue(":type", type);
    query.bindValue(":id_directory", id_directory);
    query.bindValue(":id_account", id_account);
    if (!Database::instance()->query(query))
        return (false);
    if (!informations.isEmpty())
        this->setInformations(informations);
    this->id = id;
    return (true);
}

bool    TableFiles::add(const QString &name, const QString &path, const QString &type,
                        const QString &id_directory, const QString &id_account)
{
    return (this->add(name, path, QMap<QString, QVariant>(), type, id_directory, id_account));
}

QString TableFiles::getPath()
{
    QSqlQuery                           query;
    QVector<QMap<QString, QVariant> >   result;

    query.prepare(Database::instance()->getQuery("TableFiles", "getPath"));
    query.bindValue(":id", this->id);
    if (Database::instance()->query(query, result) && result.size() > 0)
        return (result[0]["path"].toString());
    return ("");
}

QString TableFiles::getFullPath()
{
    QString path = this->getPath();

    // Relative path
    if (QFileInfo(Configurations::instance()->get("filesPath") + "/" + path).isFile())
        return (Configurations::instance()->get("filesPath") + "/" + path);
    // Absolute path
    if (QFileInfo(path).isFile())
        return (path);
    return ("");
}

bool    TableFiles::setPath(const QString &path)
{
    QSqlQuery   query;

    query.prepare(Database::instance()->getQuery("TableFiles", "setPath"));
    query.bindValue(":id", this->id);
    query.bindValue(":path", path);
    return (Database::instance()->query(query));
}

QString TableFiles::getType()
{
    QSqlQuery                           query;
    QVector<QMap<QString, QVariant> >   result;

    query.prepare(Database::instance()->getQuery("TableFiles", "getType"));
    query.bindValue(":id", this->id);
    if (Database::instance()->query(query, result) && result.size() > 0)
        return (this->types.contains(result[0]["type"].toString()) ? result[0]["type"].toString() : "other");
    return ("");
}

bool    TableFiles::setType(const QString &type)
{
    QSqlQuery   query;

    query.prepare(Database::instance()->getQuery("TableFiles", "setType"));
    query.bindValue(":id", this->id);
    if (this->types.contains(type))
        query.bindValue(":type", type);
    else
        query.bindValue(":type", "other");
    return (Database::instance()->query(query));
}

QString TableFiles::getIdDirectory()
{
    QSqlQuery                           query;
    QVector<QMap<QString, QVariant> >   result;

    query.prepare(Database::instance()->getQuery("TableFiles", "getIdDirectory"));
    query.bindValue(":id", this->id);
    if (Database::instance()->query(query, result) && result.size() > 0)
        return (result[0]["id_directory"].toString());
    return ("");
}

bool    TableFiles::setIdDirectory(const QString &id_directory)
{
    QSqlQuery   query;

    query.prepare(Database::instance()->getQuery("TableFiles", "setIdDirectory"));
    query.bindValue(":id", this->id);
    query.bindValue(":id_directory", id_directory);
    return (Database::instance()->query(query));
}

QVariant TableFiles::getInformation(const QString &name)
{
    QSqlQuery                           query;
    QVector<QMap<QString, QVariant> >   result;

    query.prepare(Database::instance()->getQuery("TableFiles", "getInformation"));
    query.bindValue(":id_file", this->id);
    query.bindValue(":name", name);
    if (Database::instance()->query(query, result) && result.size() > 0)
        return (result[0]["value"]);
    return ("");
}

QMap<QString, QVariant> TableFiles::getInformations()
{
    QSqlQuery                           query;
    QVector<QMap<QString, QVariant> >   result;
    QMap<QString, QVariant>             informations;
    int                                 i;
    int                                 s;

    query.prepare(Database::instance()->getQuery("TableFiles", "getInformations"));
    query.bindValue(":id_file", this->id);
    Database::instance()->query(query, result);
    for (i = 0, s = result.size(); i < s; ++i)
        informations[result[i]["name"].toString()] = result[i]["value"];
    return (informations);
}

bool    TableFiles::setInformation(const QString &name, const QVariant &value)
{
    QVector<QMap<QString, QVariant> >   result;
    QSqlQuery                           query;

    query.prepare(Database::instance()->getQuery("TableFiles", "setInformation_select"));
    query.bindValue(":id_file", this->id);
    query.bindValue(":name", name);
    if (!Database::instance()->query(query, result))
        return (false);
    if (result.size() < 1)
    {
        query.prepare(Database::instance()->getQuery("TableFiles", "setInformation_insert"));
        query.bindValue(":id", Tools::createUuid());
        query.bindValue(":id_file", this->id);
        query.bindValue(":name", name);
        query.bindValue(":value", value);
    }
    else
    {
        query.prepare(Database::instance()->getQuery("TableFiles", "setInformation_update"));
        query.bindValue(":id_file", this->id);
        query.bindValue(":value", value);
        query.bindValue(":name", name);
    }
    return (Database::instance()->query(query));
}

bool    TableFiles::setInformations(const QMap<QString, QVariant> &informations)
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

bool    TableFiles::removeInformation(const QString &name)
{
    QSqlQuery   query;

    query.prepare(Database::instance()->getQuery("TableFiles", "removeInformation"));
    query.bindValue(":id_file", this->id);
    query.bindValue(":name", name);
    return (Database::instance()->query(query) && query.numRowsAffected() > 0);
}

bool    TableFiles::removeInformations(const QStringList &informations)
{
    QStringListIterator it(informations);
    bool                result = true;

    while (it.hasNext())
    {
        it.next();
        if (!this->removeInformation(it.peekPrevious()))
            result = false;
    }
    return (result);
}

QString TableFiles::getVirtualPath(bool initialSlash, bool fileName)
{
    TableDirectories    directory;
    QString             virtualPath;

    // Get the path of the directory of the file
    directory.setId(this->getIdDirectory());
    virtualPath = directory.getVirtualPath(initialSlash);
    // And add the file name at the end
    if (fileName)
    {
        if (!virtualPath.isEmpty() && virtualPath != "/")
            virtualPath.append("/");
        virtualPath.append(this->getName());
    }
    return (virtualPath);
}

bool    TableFiles::setVirtualPath(const QString &virtualPath)
{
    QString id_directory;
    QString path;

    path = Tools::cleanPath(virtualPath);
    // The file is an the root of the virtual path
    if (path.count('/') == path.size())
        return (this->setIdDirectory(""));
    // Get the id of the new directory of the file
    if ((id_directory = TableDirectories().getIdFromVirtualPath(path)).isEmpty())
        return (false);
    return (this->setIdDirectory(id_directory));
}

QStringList TableFiles::getCollections()
{
    QSqlQuery                           query;
    QVector<QMap<QString, QVariant> >   result;
    QStringList                         collections;
    int                                 i;
    int                                 s;

    query.prepare(Database::instance()->getQuery("TableFiles", "getCollections"));
    query.bindValue(":id_file", this->id);
    Database::instance()->query(query, result);
    for (i = 0, s = result.size(); i < s; ++i)
        collections << result[i]["id_collection"].toString();
    return (collections);
}

bool            TableFiles::addCollection(const QString &id_collection)
{
    QSqlQuery   query;

    query.prepare(Database::instance()->getQuery("TableFiles", "addCollection"));
    query.bindValue(":id", Tools::createUuid());
    query.bindValue(":id_file", this->id);
    query.bindValue(":id_collection", id_collection);
    return (Database::instance()->query(query));
}

bool            TableFiles::removeCollection(const QString &id_collection)
{
    QSqlQuery   query;

    query.prepare(Database::instance()->getQuery("TableFiles", "removeCollection"));
    query.bindValue(":id_file", this->id);
    query.bindValue(":id_collection", id_collection);
    return (Database::instance()->query(query));
}
