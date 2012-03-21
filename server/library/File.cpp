#include "File.h"
#include "Library.h"

using namespace LightBird;

File::File() : Node(Node::FileNode)
{
}

File File::byId(const QString &id)
{
    File f;

    f.setId(id);
    return (f);
}

File File::byPath(const QString &path, const Dir &parent)
{
    Node *n = Node::byPath(path, parent);
    File self;

    if (n && n->getNodeType() == Node::FileNode)
        self.setId(n->getId());
    delete n;
    return (self);
}

QString File::getTableName() const
{
    return ("files");
}

QString File::getPhysicalPath() const
{
    return (getColumn("path"));
}
