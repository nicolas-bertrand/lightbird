#include "Dir.h"
#include "File.h"
#include "Library.h"
#include "Node.h"
#include "Table.h"

using namespace LightBird;

Node::Node(Node::NodeType type)
    : id(QString())
    , type(type)
    , valid(false)
{
}

QString Node::getId() const
{
    return (id);
}

void            Node::setId(const QString &id)
{
    IDatabase   &db = Library::database();
    QSqlQuery   sqlQuery(db.getQuery("VFS", "select_node").replace(":table", getTableName()), db.getDatabase());
    QVariantMap result;

    sqlQuery.bindValue(":id", id);
    if (db.query(sqlQuery, result))
    {
        this->id = id;
        this->valid = true;
    }
    else
    {
        this->id = QString();
        this->valid = false;
    }
}

Node::NodeType  Node::getNodeType() const
{
    return (this->type);
}

QString         Node::getColumn(QString column) const
{
    IDatabase   &db = Library::database();
    QSqlQuery   sqlQuery(db.getQuery("VFS", "select_node").replace(":table", getTableName()), db.getDatabase());
    QVariantMap result;

    sqlQuery.bindValue(":id", this->getId());
    if (!db.query(sqlQuery, result))
        return (QString());
    return (result[column].toString());
}

QString Node::getName() const
{
    return (this->getColumn("name"));
}

QString Node::getParentId() const
{
    return (this->getColumn("id_directory"));
}

Dir Node::getParent() const
{
    return (Dir::byId(this->getParentId()));
}

QString Node::getPath() const
{
    if (this->id.isEmpty())
        return ("/");
    return (this->getParent().getPath() + this->getName() + ((this->getNodeType() == Node::DirNode) ? "/" : ""));
}

Node     *Node::byId(const QString &id)
{
    QSharedPointer<Table> table = QSharedPointer<Table>(Library::database().getTable(Table::Object, id));
    Node *n = NULL;

    if (table && table->isTable(Table::Files))
        n = new File();
    // An empty id means this the the root
    else if ((table && table->isTable(Table::Directories)) || id.isEmpty())
        n = new Dir();
    if (n)
        n->setId(id);
    return n;
}

Node            *Node::byPath(const QString &p, const Dir &parent)
{
    IDatabase   &db = Library::database();
    QVariantMap result;
    QSqlQuery   query(db.getDatabase());
    QString     path = p;
    QString     id = parent.getId();
    bool        ok = true;

    if (path.startsWith('/'))
    {
        id = ""; // This is an absolute path. Starts at root thus empty id
        path.remove(0,1); // Remove the first "/"
    }
    QStringListIterator it(path.split('/'));
    while (it.hasNext())
    {
        QString segment = it.next();
        if (segment.isEmpty())
            continue;
        else if (segment == "..")
        {
            // No need to get the parent if we're already at root
            if (!id.isEmpty())
            {
                result.clear();
                query.clear();
                query.prepare(db.getQuery("VFS", "select_node").replace(":table", "directories"));
                query.bindValue(":id", id);
                if (db.query(query, result))
                    id = result["id_directory"].toString();
            }
        }
        else
        {
            bool found = false;
            QStringList tables;
            tables << "directories";
            // Check in the files table only if it is the last part of the path
            if (!it.hasNext())
                tables << "files";
            QStringListIterator tablesIt(tables);
            while(!found && tablesIt.hasNext())
            {
                result.clear();
                query.clear();
                query.prepare(db.getQuery("VFS", "select_node_by_name_parent").replace(":table", tablesIt.next()));
                query.bindValue(":name", segment);
                query.bindValue(":parent", id);
                if (db.query(query, result))
                {
                    found = true;
                    id = result["id"].toString();
                }
            }
            if (!found)
            {
                ok = false;
                break;
            }
        }
    }
    if (ok)
        return (Node::byId(id));
    return (NULL);
}

QString         Node::getOwner() const
{
    IDatabase   &db = Library::database();
    QSqlQuery   sqlQuery(db.getQuery("VFS", "select_node_owner").replace(":table", getTableName()), db.getDatabase());
    QVariantMap result;

    sqlQuery.bindValue(":id", this->getId());
    if (!db.query(sqlQuery, result))
        return (QString());
    return (result["name"].toString());
}

bool Node::isValid()
{
    return (this->valid);
}

Node::operator bool()
{
    return (this->isValid());
}
