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
        /// @brief Returns the id of the directory with the given name in the
        /// current directory if it exists.
        /// @param name : The name of the directory to return.
        QString     getDirectory(const QString &name) const;
        /// @brief Returns the id of the file with the given name in the current
        /// directory if it exists.
        /// @param name : The name of the file to return.
        QString     getFile(const QString &name) const;
        /// @brief Creates all the directories that are missing in the path from
        /// the current directory, or from the root if the directory is not set.
        /// @param path : The path to create from the current directory or the root.
        /// @param id_account : The owner of all the directories that will be created.
        /// @return The id of the last directory of the path.
        QString     createVirtualPath(const QString &virtualPath, const QString &id_account = "");
    };
}

#endif // LIGHTBIRD_TABLEDIRECTORIES_H
