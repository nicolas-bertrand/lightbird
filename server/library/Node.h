#ifndef LIGHTBIRD_NODE_H
# define LIGHTBIRD_NODE_H

# include <QSharedPointer>

# include "Export.h"
# include "Table.h"

namespace LightBird
{
    class Dir;

    /// @brief This abstract class represents a node of the VFS, either a directory
    /// or a file. It only stores the node's id. Every requested to other data may
    /// generate requests to the database.
    /// The File and Dir classes inherit from it.
    class LIB Node
    {
        public:
            /// @brief The type of the node.
            /// In most functions, only FileNode and DirNode are used.
            enum NodeType
            {
                AnyNode,
                FileNode,
                DirNode
            };

            /// @brief Creates a new invalid node without any id assigned.
            /// @param type : The node's type. Cannot be AnyNode.
            /// The returned instance may only be used as set by the type parameter.
            Node(NodeType type);
            virtual ~Node() {}

            /// @brief Sets the node's id.
            /// This method does not change the node's id in the database but only
            /// the id stored in the object. Therefore, the object points to a
            /// different node. The type must stay the same. If the id wasn't found,
            /// the object doesn't point to anything and isValid returns false.
            virtual void setId(const QString &id);

            /// @brief Gets the node's type.
            /// The instance may then be casted to the corresponding class, File or Dir.
            NodeType getNodeType() const;

            /// @brief Gets the id of the pointed node.
            QString getId() const;
            /// @brief Gets the virtual path of the pointed node.
            /// The returned path is absolute.
            /// It starts with '/' and ends with '/' if the node is a directory.
            QString getPath() const;
            /// @brief Gets the virtual filename of the pointed node.
            QString getName() const;
            /// @brief Gets the id of the node's parent directory.
            /// @return the node's parent's id or an empty string if the node is
            /// at the root of the VFS.
            QString getParentId() const;
            /// @brief Gets an instance of the node's parent directory.
            Dir     getParent() const;
            /// @brief Gets the name of the node's owner.
            QString getOwner() const;

            /// @brief Determines whether the node is valid. A node is valid if at
            /// the time of the instance's creation the node existed in the database.
            bool    isValid();
            /// @see Node::isValid
            operator bool();

            /// @brief Gets the name of the database's table storing the node.
            /// @return the exact name as used in the database.
            virtual QString getTableName() const = 0;
            /// @brief Gets a column from the node's corresponding row.
            /// @param column : The column's exact name as used in the database.
            QString getColumn(QString column) const;

            /// @brief Creates a new node instance, based on it's id.
            /// An empty id returns the root Node.
            /// The caller must delete the instance.
            /// An easy way of doing it is by using a QSharedPointer.
            /// Uses this function if you are looking for a node of any type.
            /// If you are looking for a specific type, use File::byId() or Dir::byId().
            /// @return A pointer to the node instance or NULL if no node with the id exists.
            /// @see Node::setId File::byId Dir::byId
            static Node *byId(const QString &id);
            /// @brief Creates a new node instance, based on it's virtual path.
            /// The caller must delete the instance.
            /// An easy way of doing it is by using a QSharedPointer.
            /// Uses this function if you are looking for a node of any type.
            /// If you are looking for a specific type, use File::byId() or Dir::byId().
            /// @param path : The node's path. May be absolute, starting with '/'
            /// or relative to the parent parameter.
            /// If it ends with a '/', only directories are looked for.
            /// @param parent : The directory used as starting directory in case
            /// of an relative path. It is ignored if the path is absolute.
            /// @return A pointer to the node instance or NULL if no node corresponds to this path.
            /// @see File::byPath Dir::byPath
            static Node *byPath(const QString &path, const Dir &parent);

        protected:
            QString  id;
            NodeType type;
            bool     valid;
    };
}

#endif // LIGHTBIRD_NODE_H
