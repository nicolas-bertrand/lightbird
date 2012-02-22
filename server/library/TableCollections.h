#ifndef LIGHTBIRD_TABLECOLLECTIONS_H
# define LIGHTBIRD_TABLECOLLECTIONS_H

# include <QString>
# include <QStringList>

# include "TableObjects.h"

namespace LightBird
{
    /// @brief Handles the transactions with the database relating to a collection.
    /// Each modifications done in this object is immediatly saved in the database.
    class LIB TableCollections : public LightBird::TableObjects
    {
    public:
        TableCollections(const QString &id = "");
        ~TableCollections();
        TableCollections(const TableCollections &table);
        TableCollections &operator=(const TableCollections &table);

        /// @brief Returns the id of the collection from its virtual path.
        /// @param id_account : The id of the account of the collection to get.
        /// It is used if several accounts have the same collection name in the
        /// same parent collection.
        QString     getIdFromVirtualPath(const QString &virtualPath, const QString &id_account = "") const;
        /// @brief Sets the id of the collection from its virtual path.
        /// @param id_account : The id of the account of the collection to get.
        /// It is used if several accounts have the same collection name in the
        /// same parent collection.
        bool        setIdFromVirtualPath(const QString &virtualPath, const QString &id_account = "");
        /// @brief Creates a new collection.
        /// @param name : The name of the new collection.
        /// @param id_collection : The id of the parent collection, or empty
        /// if the new collection is at the root.
        /// @param id_account : The id of the account that owns the collection.
        /// @return True if the collection has been created.
        bool        add(const QString &name, const QString &id_collection = "", const QString &id_account = "");

        // Fields
        /// @brief Returns the id of the parent of the collection, or empty if
        /// it is at the root.
        QString     getIdCollection() const;
        /// @brief Modifies the id of the parent of the collection.
        bool        setIdCollection(const QString &id_collection = "");

        // Other
        /// @brief Returns the virtual path of the collection.
        /// @param initialSlash : If true, the first char of the result will be "/".
        /// @param finalSlash : If true, the last char of the result will be "/".
        QString     getVirtualPath(bool initialSlash = false, bool finalSlash = false) const;
        /// @brief Moves the collection to the collection localized by the virtual
        /// path in parameter.
        bool        setVirtualPath(const QString &virtualPath);
        /// @brief Returns the id of all the collections in the current collection.
        /// If id_accessor and right are not empty, only the collections for
        /// which the accessor has the right will be returned.
        /// @param id_accessor : If not empty, only the files for which this
        /// accessor has the right will be returned.
        /// @param right : The name of the right that the accessor must have.
        QStringList getCollections(const QString &id_accessor = "", const QString &right = "") const;
        /// @brief Returns the id of all the files in the current collection.
        /// If id_accessor and right are not empty, only the files for which
        /// the accessor has the right will be returned.
        /// @param id_accessor : If not empty, only the files for which this
        /// accessor has the right will be returned.
        /// @param right : The name of the right that the accessor must have.
        QStringList getFiles(const QString &id_accessor = "", const QString &right = "") const;
    };
}

#endif // LIGHTBIRD_TABLECOLLECTIONS_H
