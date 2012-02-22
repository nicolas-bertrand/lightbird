#include <QFileInfo>

#include "Library.h"
#include "LightBird.h"

using namespace LightBird;

TableFiles::TableFiles(const QString &id)
{
    this->tableName = "files";
    this->tableId = Table::Files;
    this->setId(id);
    this->types << "image" << "audio" << "video" << "document";
}

TableFiles::~TableFiles()
{
}

TableFiles::TableFiles(const TableFiles &table) : TableObjects()
{
    *this = table;
}

TableFiles &TableFiles::operator=(const TableFiles &table)
{
    if (this != &table)
    {
        TableObjects::operator=(table);
        this->types = table.types;
    }
    return (*this);
}

QString TableFiles::getIdFromVirtualPath(const QString &virtualPath) const
{
    QSqlQuery               query;
    QVector<QVariantMap>    result;
    QString                 name;
    QString                 path;
    QString                 id_directory = "";

    path = cleanPath(virtualPath);
    name = path;
    // If the virtual path has directories, get the id of the parent directory
    if (name.contains('/'))
    {
        path = path.left(name.lastIndexOf('/'));
        name = name.right(name.size() - path.size() - 1);
        id_directory = TableDirectories().getIdFromVirtualPath(path);
    }
    // Find the id of the file using its name, and the id of its parent directory, if it has one
    query.prepare(Library::database().getQuery("TableFiles", "getIdFromVirtualPath"));
    query.bindValue(":id_directory", id_directory);
    query.bindValue(":name", name);
    if (!Library::database().query(query, result) || result.size() <= 0)
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

bool    TableFiles::add(const QString &name, const QString &path, const QVariantMap &informations,
                        const QString &type, const QString &id_directory, const QString &id_account)
{
    QSqlQuery   query;
    QString     id;

    id = createUuid();
    query.prepare(Library::database().getQuery("TableFiles", "add"));
    query.bindValue(":id", id);
    query.bindValue(":name", name);
    query.bindValue(":path", path);
    query.bindValue(":type", type);
    query.bindValue(":id_directory", id_directory);
    query.bindValue(":id_account", id_account);
    if (!Library::database().query(query))
        return (false);
    if (!informations.isEmpty())
        this->setInformations(informations);
    this->id = id;
    return (true);
}

bool    TableFiles::add(const QString &name, const QString &path, const QString &type,
                        const QString &id_directory, const QString &id_account)
{
    return (this->add(name, path, QVariantMap(), type, id_directory, id_account));
}

QString TableFiles::getPath() const
{
    QSqlQuery               query;
    QVector<QVariantMap>    result;

    query.prepare(Library::database().getQuery("TableFiles", "getPath"));
    query.bindValue(":id", this->id);
    if (Library::database().query(query, result) && result.size() > 0)
        return (result[0]["path"].toString());
    return ("");
}

QString TableFiles::getFullPath() const
{
    QString path = this->getPath();

    // Relative path
    if (QFileInfo(Library::configuration().get("filesPath") + "/" + path).isFile())
        return (Library::configuration().get("filesPath") + "/" + path);
    // Absolute path
    if (QFileInfo(path).isFile())
        return (path);
    return ("");
}

bool    TableFiles::setPath(const QString &path)
{
    QSqlQuery   query;

    query.prepare(Library::database().getQuery("TableFiles", "setPath"));
    query.bindValue(":id", this->id);
    query.bindValue(":path", path);
    return (Library::database().query(query));
}

QString TableFiles::getType() const
{
    QSqlQuery               query;
    QVector<QVariantMap>    result;

    query.prepare(Library::database().getQuery("TableFiles", "getType"));
    query.bindValue(":id", this->id);
    if (Library::database().query(query, result) && result.size() > 0)
        return (this->types.contains(result[0]["type"].toString()) ? result[0]["type"].toString() : "other");
    return ("");
}

bool    TableFiles::setType(const QString &type)
{
    QSqlQuery   query;

    query.prepare(Library::database().getQuery("TableFiles", "setType"));
    query.bindValue(":id", this->id);
    if (this->types.contains(type))
        query.bindValue(":type", type);
    else
        query.bindValue(":type", "other");
    return (Library::database().query(query));
}

QString TableFiles::getIdDirectory() const
{
    QSqlQuery               query;
    QVector<QVariantMap>    result;

    query.prepare(Library::database().getQuery("TableFiles", "getIdDirectory"));
    query.bindValue(":id", this->id);
    if (Library::database().query(query, result) && result.size() > 0)
        return (result[0]["id_directory"].toString());
    return ("");
}

bool    TableFiles::setIdDirectory(const QString &id_directory)
{
    QSqlQuery   query;

    query.prepare(Library::database().getQuery("TableFiles", "setIdDirectory"));
    query.bindValue(":id", this->id);
    query.bindValue(":id_directory", id_directory);
    return (Library::database().query(query));
}

QVariant TableFiles::getInformation(const QString &name) const
{
    QSqlQuery               query;
    QVector<QVariantMap>    result;

    query.prepare(Library::database().getQuery("TableFiles", "getInformation"));
    query.bindValue(":id_file", this->id);
    query.bindValue(":name", name);
    if (Library::database().query(query, result) && result.size() > 0)
        return (result[0]["value"]);
    return ("");
}

QMap<QString, QVariant> TableFiles::getInformations() const
{
    QSqlQuery               query;
    QVector<QVariantMap>    result;
    QVariantMap             informations;
    int                     i;
    int                     s;

    query.prepare(Library::database().getQuery("TableFiles", "getInformations"));
    query.bindValue(":id_file", this->id);
    Library::database().query(query, result);
    for (i = 0, s = result.size(); i < s; ++i)
        informations[result[i]["name"].toString()] = result[i]["value"];
    return (informations);
}

bool    TableFiles::setInformation(const QString &name, const QVariant &value)
{
    QVector<QVariantMap>    result;
    QSqlQuery               query;

    query.prepare(Library::database().getQuery("TableFiles", "setInformation_select"));
    query.bindValue(":id_file", this->id);
    query.bindValue(":name", name);
    if (!Library::database().query(query, result))
        return (false);
    if (result.size() < 1)
    {
        query.prepare(Library::database().getQuery("TableFiles", "setInformation_insert"));
        query.bindValue(":id", createUuid());
        query.bindValue(":id_file", this->id);
        query.bindValue(":name", name);
        query.bindValue(":value", value);
    }
    else
    {
        query.prepare(Library::database().getQuery("TableFiles", "setInformation_update"));
        query.bindValue(":id_file", this->id);
        query.bindValue(":value", value);
        query.bindValue(":name", name);
    }
    return (Library::database().query(query));
}

bool    TableFiles::setInformations(const QVariantMap &informations)
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

    query.prepare(Library::database().getQuery("TableFiles", "removeInformation"));
    query.bindValue(":id_file", this->id);
    query.bindValue(":name", name);
    return (Library::database().query(query) && query.numRowsAffected() > 0);
}

bool    TableFiles::removeInformations(const QStringList &informations)
{
    QSqlQuery   query;
    bool        result = true;

    if (informations.isEmpty())
    {
        query.prepare(Library::database().getQuery("TableFiles", "removeInformations"));
        query.bindValue(":id_file", this->id);
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

QString TableFiles::getVirtualPath(bool initialSlash, bool fileName) const
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

    path = cleanPath(virtualPath);
    // The file is an the root of the virtual path
    if (path.count('/') == path.size())
        return (this->setIdDirectory(""));
    // Get the id of the new directory of the file
    if ((id_directory = TableDirectories().getIdFromVirtualPath(path)).isEmpty())
        return (false);
    return (this->setIdDirectory(id_directory));
}

QStringList TableFiles::getCollections() const
{
    QSqlQuery               query;
    QVector<QVariantMap>    result;
    QStringList             collections;
    int                     i;
    int                     s;

    query.prepare(Library::database().getQuery("TableFiles", "getCollections"));
    query.bindValue(":id_file", this->id);
    Library::database().query(query, result);
    for (i = 0, s = result.size(); i < s; ++i)
        collections << result[i]["id_collection"].toString();
    return (collections);
}

bool            TableFiles::addCollection(const QString &id_collection)
{
    QSqlQuery   query;

    query.prepare(Library::database().getQuery("TableFiles", "addCollection"));
    query.bindValue(":id", createUuid());
    query.bindValue(":id_file", this->id);
    query.bindValue(":id_collection", id_collection);
    return (Library::database().query(query));
}

bool            TableFiles::removeCollection(const QString &id_collection)
{
    QSqlQuery   query;

    query.prepare(Library::database().getQuery("TableFiles", "removeCollection"));
    query.bindValue(":id_file", this->id);
    query.bindValue(":id_collection", id_collection);
    return (Library::database().query(query));
}
