#ifndef LIGHTBIRD_TABLEDIRECTORIES_H
# define LIGHTBIRD_TABLEDIRECTORIES_H

# include <QString>
# include <QStringList>

# include "TableObjects.h"

namespace LightBird
{
    /// @brief Handles the transactions with the database relating to a directory.
    /// Each modifications done in this object is immediatly saved in the database.
    class LIB TableDirectories : public LightBird::TableObjects
    {
    public:
        TableDirectories(const QString &id = "");
        ~TableDirectories();
        TableDirectories(const TableDirectories &table);
        TableDirectories &operator=(const TableDirectories &table);

        /// @brief Returns the id of the directory from its virtual path.
        QString     getIdFromVirtualPath(const QString &virtualPath) const;
        /// @brief Set the id of the directory from its virtual path.
        bool        setIdFromVirtualPath(const QString &virtualPath);
        /// @brief Creates a new directory.
        /// @param name : The name of the new directory.
        /// @param id_directory : The id of the parent directory, or empty
        /// if the new directory is at the root.
        /// @param id_account : The id of the account that owns the directory.
        /// @return True if the directory has been created.
        bool        add(const QString &name, const QString &id_directory = "", const QString &id_account = "");
        /// @see LightBird::Table::remove
        bool        remove(const QString &id = "");
        /// @brief Removes the directory from the database and all the files it
        /// contains from the file system.
        /// @param removeFiles : If true, the directory is removed from the
        /// database and all the files it contains are deleted from the file
        /// system. Otherwise its files will be removed from the database but not
        /// from the file system.
        /// @return True if the directory have been removed. If some files can't
        /// be deleted from the file system now, they will be automatically
        /// removed later, but they will no longer exist in the database anyway.
        bool        remove(bool removeFiles);

        // Fields
        /// @brief Returns the id of the parent of the directory, or empty if
        /// it is at the root.
        QString     getIdDirectory() const;
        /// @brief Modifies the id of the parent of the directory.
        bool        setIdDirectory(const QString &id_directory = "");

        // Other
        /// @brief Returns the virtual path of the directory.
        /// @param initialSlash : If true, the first char of the result will be "/".
        /// @param finalSlash : If true, the last char of the result will be "/".
        QString     getVirtualPath(bool initialSlash = false, bool finalSlash = false) const;
        /// @brief Moves the directory to the directory localized by the virtual
        /// path in parameter.
        bool        setVirtualPath(const QString &virtualPath);
        /// @brief Returns the id of all the directories in the current directory.
        /// If id_accessor and right are not empty, only the directories for
        /// which the accessor has the right will be returned.
        /// @param id_accessor : If not empty, only the files for which this
        /// accessor has the right will be returned.
        /// @param right : The name of the right that the accessor must have.
        QStringList getDirectories(const QString &id_accessor = "", const QString &right = "") const;
        /// @brief Returns the id of all the files in the current directory.
        /// If id_accessor and right are not empty, only the files for which
        /// the accessor has the right will be returned.
        /// @param id_accessor : If not empty, only the files for which this
        /// accessor has the right will be returned.
        /// @param right : The name of the right that the accessor must have.
        QStringList getFiles(const QString &id_accessor = "", const QString &right = "") const;
        /// @brief Returns the id of all the files in the current directory
        /// and its subdirectories.
        /// If id_accessor and right are not empty, only the files for which
        /// the accessor has the right will be returned.
        /// @param id_accessor : If not empty, only the files for which this
        /// accessor has the right will be returned.
        /// @param right : The name of the right that the accessor must have.
        QStringList getAllFiles(const QString &id_accessor = "", const QString &right = "") const;
        /// @brief Returns the id of the directory with the given name in the
        /// current directory if it exists.
        /// @param name : The name of the directory to return.
        QString     getDirectory(const QString &name) const;
        /// @brief Returns the id of the file with the given name in the current
        /// directory if it exists.
        /// @param name : The name of the file to return.
        QString     getFile(const QString &name) const;
        /// @brief Returns the list of the parents of the directory.
        QStringList getParents() const;
        /// @brief Creates all the directories that are missing in the path from
        /// the current directory, or from the root if the directory is not set.
        /// @param path : The path to create from the current directory or the root.
        /// @param id_account : The owner of all the directories that will be created.
        /// @return The id of the last directory of the path.
        QString     createVirtualPath(const QString &virtualPath, const QString &id_account = "");
        /// @brief Changes the current directory based on the path.
        /// @param path : The path to the new directory, relative to the current
        /// directory. "/" can be used at the beginning to specify an absolute path.
        /// ".." is also understood.
        /// @return True if the path is valid.
        bool        cd(const QString &path);
    };
}

#endif // LIGHTBIRD_TABLEDIRECTORIES_H
