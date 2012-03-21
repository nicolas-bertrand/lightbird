#ifndef LIGHTBIRD_FILE_H
# define LIGHTBIRD_FILE_H

# include "Dir.h"
# include "Node.h"

namespace LightBird
{
    /// @brief This class represents a file in the VFS.
    class LIB File : public Node
    {
        public:
            /// @brief Instanciates a File object, based on it's id.
            /// If no file node with this id is found, an invalid file is returned.
            /// Unlike Node::byId, this method returns the object and not a pointer to it.
            /// Therefore there is no need to delete manually afterwards.
            /// @see Node::byId Node::isValid
            static File byId(const QString &id);
            /// @brief Instanciates a File object, based on it's path.
            /// If no file node corresponding to this path, an invalid file is returned.
            /// Unlike Node::byPath, this method returns the object and not a pointer to it.
            /// Therefore there is no need to delete manually afterwards.
            /// @see Node::byId Node::isValid
            static File byPath(const QString &path, const Dir &parent = Dir::root());

            /// @brief Gets the path to the "real" file.
            /// The "real" file is the one on the hard drive, containing the data.
            QString getPhysicalPath() const;

            /// @see Node::getTableName
            QString getTableName() const;

        protected:
            File(); // Named constructors should be used instead

        private:
            // This is required because Node::byId() uses our protected constructor
            friend class Node;
    };
}

#endif // LIGHTBIRD_FILE_H
