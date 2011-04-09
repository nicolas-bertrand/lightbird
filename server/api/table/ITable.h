#ifndef ITABLE_H
# define ITABLE_H

# include <QString>
# include <QDateTime>

namespace LightBird
{
    class ITableAccessors;
    class ITableAccounts;
    class ITableCollections;
    class ITableDirectories;
    class ITableEvents;
    class ITableFiles;
    class ITableGroups;
    class ITableLimits;
    class ITableObjects;
    class ITablePermissions;
    class ITableTags;

    /// @brief This interface is used as a common parent of the table abstractions interfaces.
    class ITable
    {
    public:
        virtual ~ITable() {}

         /// @brief List all the available tables.
        enum Tables
        {
            Accessor,       ///< The table is an unknow accessor.
            Accounts,
            Collections,
            Directories,
            Events,
            Files,
            Groups,
            Limits,
            Object,         ///< The table is an unknow object.
            Permissions,
            Tags,
            Unknow          ///< The table is unkown.
        };

        // Id
        /// @brief Returns the id of the row stored in the current instances.
        /// Notice that the row represented by this id may no longer exist.
        /// Use the method exists() to check that is has not been deleted.
        virtual const QString   &getId() = 0;
        /// @brief Changes the row stored in the current instance. This doesn't
        /// changes the id of the current rows (which is not permitted by the database).
        /// @param id : The id of the new row, which must exist.
        /// @return True if the id exists.
        virtual bool            setId(const QString &id) = 0;
        /// @brief If the parameter is empty, returns true if the current row
        /// represented by its id still exists in the database. Otherwise
        /// check if the given id exists.
        virtual bool            exists(const QString &id = "") = 0;
        /// @brief Clear the id of the instance.
        virtual void            clear() = 0;
        /// @brief Removes the current row if the parameter is empty, or the row pointed by the id.
        virtual bool            remove(const QString &id = "") = 0;

        // Dates
        /// @brief Returns the date of the last modification of the row.
        virtual QDateTime       getModified() = 0;
        /// @brief Returns the date of the creation of the row.
        virtual QDateTime       getCreated() = 0;

        // Table
        /// @brief Returns the name of the table represented by the instance.
        virtual const QString   &getTableName() = 0;
        /// @brief Returns the id of the table represented by the instance.
        virtual LightBird::ITable::Tables getTableId() = 0;
        /// @brief Returns true if the table corresponds to the paramater.
        virtual bool            isTable(const QString &tableName) = 0;
        /// @brief Returns true if the table corresponds to the paramater.
        virtual bool            isTable(LightBird::ITable::Tables tableId) = 0;

        // Casts
        /// @brief Cast the table if possible. Returns NULL otherwise.
        virtual LightBird::ITableAccessors   *toTableAccessors() = 0;
        /// @brief Cast the table if possible. Returns NULL otherwise.
        virtual LightBird::ITableAccounts    *toTableAccounts() = 0;
        /// @brief Cast the table if possible. Returns NULL otherwise.
        virtual LightBird::ITableCollections *toTableCollections() = 0;
        /// @brief Cast the table if possible. Returns NULL otherwise.
        virtual LightBird::ITableDirectories *toTableDirectories() = 0;
        /// @brief Cast the table if possible. Returns NULL otherwise.
        virtual LightBird::ITableEvents      *toTableEvents() = 0;
        /// @brief Cast the table if possible. Returns NULL otherwise.
        virtual LightBird::ITableFiles       *toTableFiles() = 0;
        /// @brief Cast the table if possible. Returns NULL otherwise.
        virtual LightBird::ITableGroups      *toTableGroups() = 0;
        /// @brief Cast the table if possible. Returns NULL otherwise.
        virtual LightBird::ITableLimits      *toTableLimits() = 0;
        /// @brief Cast the table if possible. Returns NULL otherwise.
        virtual LightBird::ITableObjects     *toTableObjects() = 0;
        /// @brief Cast the table if possible. Returns NULL otherwise.
        virtual LightBird::ITablePermissions *toTablePermissions() = 0;
        /// @brief Cast the table if possible. Returns NULL otherwise.
        virtual LightBird::ITableTags        *toTableTags() = 0;
    };
}

Q_DECLARE_INTERFACE(LightBird::ITable, "cc.lightbird.ITable");

#endif // ITABLE_H
