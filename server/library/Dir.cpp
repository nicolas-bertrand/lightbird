#include "Dir.h"
#include "Library.h"
#include "IDatabase.h"

using namespace LightBird;

Dir::Dir() : Node(Node::DirNode)
{
}

Dir Dir::root()
{
    // Root is an empty id
    return (Dir::byId(QString()));
}

Dir Dir::byId(const QString &id)
{
    Dir self;

    self.setId(id);
    return (self);
}

Dir Dir::byPath(const QString &path, const Dir &parent)
{
    Node *n = Node::byPath(path, parent);
    Dir  self;

    if (n && n->getNodeType() == Node::DirNode)
        self.setId(n->getId());
    delete n;
    return (self);
}

QString Dir::getTableName() const
{
    return ("directories");
}

QStringList Dir::getStringList(Node::NodeType nodeType)
{
    QStringList tables;
    QStringList ids;

    switch (nodeType)
    {
        case Node::FileNode:
            tables << "files";
            break;
        case Node::DirNode:
            tables << "directories";
            break;
        case Node::AnyNode:
            tables << "directories" << "files";
            break;
    }


    LightBird::IDatabase &db = LightBird::Library::database();
    QVector<QVariantMap> result;
    QSqlQuery query;

    QStringListIterator tableIt(tables);
    while (tableIt.hasNext())
    {
        result.clear();
        query.prepare(db.getQuery("VFS", "select_child_node").replace(":table", tableIt.next()));
        query.bindValue(":id", this->getId());
        db.query(query, result);
        QVectorIterator<QVariantMap> resultIt(result);
        while (resultIt.hasNext())
            ids << resultIt.next()["id"].toString();
    }
    return (ids);
}

bool Dir::isRoot()
{
    return (id.isEmpty() && this->isValid());
}

// Handles the special case of an empty id, the vfs's root
void Dir::setId(const QString &id)
{
    // Node::setId() looks for the id in the db. Don't do this for the root.
    if (id.isEmpty())
    {
        valid = true;
        this->id = QString();
    }
    else
        Node::setId(id);
}

DirIterator::DirIterator(Dir &dir, Node::NodeType nodeType) : it(dir.getStringList(nodeType))
{
}

bool DirIterator::hasPrevious()
{
    return (it.hasPrevious());
}

bool DirIterator::hasNext()
{
    return (it.hasNext());
}

Node *DirIterator::previous()
{
    return (Node::byId(it.previous()));
}

Node *DirIterator::next()
{
    return (Node::byId(it.next()));
}

Node *DirIterator::peekPrevious()
{
    return (Node::byId(it.peekPrevious()));
}

Node *DirIterator::peekNext()
{
    return (Node::byId(it.peekNext()));
}
