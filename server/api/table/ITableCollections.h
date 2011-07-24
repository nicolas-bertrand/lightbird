#ifndef ITABLECOLLECTIONS_H
# define ITABLECOLLECTIONS_H

# include <QString>
# include <QStringList>

# include "ITableObjects.h"

namespace LightBird
{
    /// @brief Handle the transactions with the database relating to a collection.
    /// Each modifications done in this object is immediatly saved in the database.
    class ITableCollections : virtual public LightBird::ITableObjects
    {
    public:
        virtual ~ITableCollections() {}

        /// @brief Returns the id of the collection from its virtual path.
        virtual QString     getIdFromVirtualPath(const QString &virtualPath) const = 0;
        /// @brief Set the id of the collection from its virtual path.
        virtual bool        setIdFromVirtualPath(const QString &virtualPath) = 0;
        /// @brief Creates a new collection.
        /// @param name : The name of the new collection.
        /// @param id_collection : The id of the parent collection, or empty
        /// if the new collection is at the root.
        /// @param id_account : The id of the account that owns the collection.
        /// @return True if the collection has been created.
        virtual bool        add(const QString &name, const QString &id_collection = "", const QString &id_account = "") = 0;

        // Fields
        /// @brief Returns the id of the parent of the collection, or empty if
        /// it is at the root.
        virtual QString     getIdCollection() const = 0;
        /// @brief Modifies the id of the parent of the collection.
        virtual bool        setIdCollection(const QString &id_collection = "") = 0;

        // Other
        /// @brief Returns the virtual path of the collection.
        /// @param initialSlash : If true, the first char of the result will be "/".
        /// @param finalSlash : If true, the last char of the result will be "/".
        virtual QString     getVirtualPath(bool initialSlash = false, bool finalSlash = false) const = 0;
        /// @brief Moves the collection to the collection localized by the virtual
        /// path in parameter.
        virtual bool        setVirtualPath(const QString &virtualPath) = 0;
        /// @brief Returns the id of all the collections in the current collection.
        /// If id_accessor and right are not empty, only the collections for
        /// which the accessor has the right will be returned.
        /// @param id_accessor : If not empty, only the files for which this
        /// accessor has the right will be returned.
        /// @param right : The name of the right that the accessor must have.
        virtual QStringList getCollections(const QString &id_accessor = "", const QString &right = "") const = 0;
        /// @brief Returns the id of all the files in the current collection.
        /// If id_accessor and right are not empty, only the files for which
        /// the accessor has the right will be returned.
        /// @param id_accessor : If not empty, only the files for which this
        /// accessor has the right will be returned.
        /// @param right : The name of the right that the accessor must have.
        virtual QStringList getFiles(const QString &id_accessor = "", const QString &right = "") const = 0;
    };
}

Q_DECLARE_INTERFACE(LightBird::ITableCollections, "cc.lightbird.ITableCollections");

#endif // ITABLECOLLECTIONS_H
