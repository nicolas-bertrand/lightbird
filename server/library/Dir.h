#ifndef LIGHTBIRD_DIR_H
# define LIGHTBIRD_DIR_H

# include <QStringList>

# include "Node.h"

namespace LightBird
{
    /// @brief This class represents a directory in the VFS.
    class LIB Dir : public Node
    {
        public:
            /// @brief Instanciates a Dir object pointing to the VFS's root.
            static Dir root();

            /// @brief Instanciates a Dir object, based on its id.
            /// If no dir node with this id is found, an invalid file is returned.
            /// Unlike Node::byId, this method returns the object and not a pointer to it.
            /// Therefore there is no need to delete manually afterwards.
            /// @see Node::byId Node::isValid
            static Dir byId(const QString &id);

            /// @brief Instanciates a File object, based on it's path.
            /// If no dir node corresponds to this path, an invalid file is returned.
            /// Unlike Node::byPath, this method returns the object and not a pointer to it.
            /// Therefore there is no need to delete manually afterwards.
            /// @see Node::byId Node::isValid
            static Dir byPath(const QString &path, const Dir &parent = Dir::root());

            /// @brief Determines wether the directory node pointed is the VFS's root.
            bool isRoot();
            /// @see Node::getTableName
            QString getTableName() const;
            /// @see Node::setId
            void setId(const QString &id);

        protected:
            Dir(); // Named constructors should be used instead

        private:
            QStringList getStringList(NodeType nodeType = AnyNode); ///< For DirIterator
            friend class DirIterator;
            friend class Node; // This is required because Node::byId() uses our private constructor
    };

    /// @brief Provides a java-style iterator over a directory's contents.
    /// A "snapshot" of the list of child nodes of dir is taken when the iterator
    /// is created. This means, if a new node is added to dir, an iterator created
    /// before won't iterate over it.
    /// In the same way, if a child node is deleted or moved, an iterator created before
    /// will iterate over it. However, a NULL pointer is returned in this case.
    ///
    /// As for Node::byId and Node::byPath, the returned objects must be deleted,
    /// even the ones returned from peek* methods. Calling methods multiple times
    /// returning Node instances pointing to the same node will result in different
    /// instances, which must all be deleted.
    /// You may use QSharedPointer to manage the pointers.
    /// @see Node::byId Node::byPath
    class LIB DirIterator
    {
        public:
            /// @brief Creates a new iterator.
            /// @param dir : The dir to iterate over.
            /// @param nodeType : If different than AnyNode, only child nodes of
            /// this type will be listed.
            explicit DirIterator(Dir &dir, Node::NodeType nodeType = Node::AnyNode);

            bool hasPrevious();
            bool hasNext();

            Node *previous();
            Node *next();
            Node *peekPrevious();
            Node *peekNext();

        private:
            QStringListIterator it;
    };
}

#endif // LIGHTBIRD_DIR_H
