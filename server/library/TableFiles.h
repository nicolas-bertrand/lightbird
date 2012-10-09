#ifndef LIGHTBIRD_TABLEFILES_H
# define LIGHTBIRD_TABLEFILES_H

# include <QMap>
# include <QString>
# include <QStringList>
# include <QVariant>

# include "TableObjects.h"

namespace LightBird
{
    /// @brief Handles the transactions with the database relating to a file.
    /// Each modifications done in this object is immediatly saved in the database.
    class LIB TableFiles : public LightBird::TableObjects
    {
    public:
        TableFiles(const QString &id = "");
        ~TableFiles();
        TableFiles(const TableFiles &table);
        TableFiles  &operator=(const TableFiles &table);

        /// @brief Returns the id of a file using its virtual path in the database.
        QString     getIdFromVirtualPath(const QString &virtualPath) const;
        /// @brief Set the id of a file using its virtual path in the database.
        bool        setIdFromVirtualPath(const QString &virtualPath);
        /// @brief Returns the id of a file using its path in the file system.
        /// The path must match exactly the path in the database.
        QString     getIdFromPath(const QString &path) const;
        /// @brief Sets the id of a file using its path in the file system.
        /// The path must match exactly the path in the database.
        bool        setIdFromPath(const QString &path);
        /// @brief Add a new file.
        /// @param name : The name of the file.
        /// @param path : The path of the file.
        /// @param Informations : A map that contains informations about the file.
        /// @param type : The type of the file.
        /// @param id_directory : The id of the directory of the file. If empty,
        /// it is at the root.
        /// @param id_account : The id of account that owns the file.
        /// @return True if the file has been created.
        bool        add(const QString &name, const QString &path, const QVariantMap &informations,
                        const QString &type = "other", const QString &id_directory = "", const QString &id_account = "");
        /// @see add
        bool        add(const QString &name, const QString &path, const QString &type = "other",
                        const QString &id_directory = "", const QString &id_account = "");
        /// @see LightBird::Table::remove
        bool        remove(const QString &id = "");
        /// @brief Removes the file from the database and the file system.
        /// @param removeFile : If true, the file is removed from the database
        /// and the file system. Otherwise it is only removed from the database.
        /// @return True if the file has been removed. If the file can't be removed
        /// immediatly from the file system, false is returned but it is removed
        /// from the database and the file will be deleted later.
        bool        remove(bool removeFile);

        // Fields
        /// @brief Returns the path of the file in the file system. This path
        /// can be relative to the filesPath or absolute.
        /// @see getFilesPath
        QString     getPath() const;
        /// @brief If the path is relative, getPath() returns the path from the
        /// filesPath directory, which is not the working directory of the server.
        /// The path returned by getFullPath() contains the filesPath if it is
        /// relative, or just the path if it is absolute. Nothing is returned
        /// if the file has not been found.
        /// @see getFilesPath
        QString     getFullPath() const;
        /// @brief Modifies the path of the file.
        bool        setPath(const QString &path);
        /// @brief Returns the type of the file, which can be image, audio, video,
        /// document, or other.
        QString     getType() const;
        /// @brief Modifies the type of the file.
        bool        setType(const QString &type = "");
        /// @brief Returns the id of the directory of the file.
        QString     getIdDirectory() const;
        /// @brief Modifies the id of the directory of the file.
        bool        setIdDirectory(const QString &id_directory = "");

        // Informations
        /// @brief Returns the value of an information of the file.
        /// @param name : The name of the information to return.
        QVariant    getInformation(const QString &name) const;
        /// @brief Returns all the informations of the file.
        QVariantMap getInformations() const;
        /// @brief Modifies the value of an information of the file, or create
        /// it if it doesn't exists.
        /// @param name : The name of the information to create or modify.
        /// @brief value : The new value of the information.
        bool        setInformation(const QString &name, const QVariant &value);
        /// @brief Modifies or creates multiple informations for the file.
        /// @param informations : The informations to modify or create.
        /// The keys of the map are the keys of the informations, and
        /// the values of the map are the values of the informations.
        bool        setInformations(const QVariantMap &informations);
        /// @brief Removes an information of the file.
        /// @param name : The name of the information to remove.
        bool        removeInformation(const QString &name);
        /// @brief Removes multiple informations of the file.
        /// @param informations : This list contains the name of each
        /// informations to remove. If empty all the informations are removed.
        bool        removeInformations(const QStringList &informations = QStringList());

        // Directories
        /// @brief Returns the virtual path of the file.
        /// @param initialSlash : If true, the first char of the result will be "/".
        /// @param fileName : If true, the file name is included in the result.
        QString     getVirtualPath(bool initialSlash = false, bool fileName = false) const;
        /// @brief Moves the file to the directory localized by the virtual
        /// path in parameter.
        bool        setVirtualPath(const QString &virtualPath);

        // Collections
        /// @brief Returns the list of the id collection of the file.
        QStringList getCollections() const;
        /// @brief Add the file to the given collection.
        bool        addCollection(const QString &id_collection);
        /// @brief Removes the file from the given collection.
        bool        removeCollection(const QString &id_collection);

    private:
        QStringList types; ///< The list of the possible types.
    };
}

#endif // LIGHTBIRD_TABLEFILES_H
