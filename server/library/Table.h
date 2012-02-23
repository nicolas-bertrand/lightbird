#ifndef LIGHTBIRD_TABLE_H
# define LIGHTBIRD_TABLE_H

# include <QDateTime>
# include <QObject>
# include <QString>

# include "Export.h"

namespace LightBird
{
    class TableAccessors;
    class TableAccounts;
    class TableCollections;
    class TableDirectories;
    class TableEvents;
    class TableFiles;
    class TableGroups;
    class TableLimits;
    class TableObjects;
    class TablePermissions;
    class TableTags;

    /// @brief This class is used as a common parent of the table abstractions classes.
    class LIB Table : public QObject
    {
        Q_OBJECT

    public:
        ~Table();

        /// @brief List all the available tables.
        enum Id
        {
           Accessor,    ///< The table is an unknow accessor.
           Accounts,
           Collections,
           Directories,
           Events,
           Files,
           Groups,
           Limits,
           Object,      ///< The table is an unknow object.
           Permissions,
           Tags,
           Unknow       ///< The table is unkown.
        };

        // Id
        /// @brief Returns the id of the row stored in the current instance.
        /// Notice that the row represented by this id may no longer exist.
        /// Use the method exists() to check that it has not been deleted.
        const QString   &getId() const;
        /// @brief Changes the row stored in the current instance. This doesn't
        /// changes the id of the current rows (which is not permitted by the database).
        /// @param id : The id of the new row, which should exist.
        /// @return True if the id exists.
        bool            setId(const QString &id);
        /// @brief If the parameter is empty, returns true if the current row
        /// represented by its id still exists in the database. Otherwise
        /// check if the given id exists.
        bool            exists(const QString &id = "");
        /// @brief Returns true if the current row represented by its id still
        /// exists in the database.
        operator        bool();
        /// @brief Clears the id of the instance.
        void            clear();
        /// @brief Removes the current row if the parameter is empty, or the
        /// row pointed by the id.
        bool            remove(const QString &id = "");

        // Dates
        /// @brief Returns the date of the last modification of the row.
        QDateTime       getModified() const;
        /// @brief Returns the date of the creation of the row.
        QDateTime       getCreated() const;

        // Table
        /// @brief Returns the name of the table represented by the instance.
        const QString   &getTableName() const;
        /// @brief Returns the id of the table represented by the instance.
        LightBird::Table::Id getTableId() const;
        /// @brief Returns true if the table corresponds to the paramater.
        bool            isTable(const QString &tableName) const;
        /// @brief Returns true if the table corresponds to the paramater.
        bool            isTable(LightBird::Table::Id tableId) const;

        // Casts
        /// @brief Casts the table if possible. Returns NULL otherwise.
        LightBird::TableAccessors   *toAccessors();
        /// @brief Casts the table if possible. Returns NULL otherwise.
        LightBird::TableAccounts    *toAccounts();
        /// @brief Casts the table if possible. Returns NULL otherwise.
        LightBird::TableCollections *toCollections();
        /// @brief Casts the table if possible. Returns NULL otherwise.
        LightBird::TableDirectories *toDirectories();
        /// @brief Casts the table if possible. Returns NULL otherwise.
        LightBird::TableEvents      *toEvents();
        /// @brief Casts the table if possible. Returns NULL otherwise.
        LightBird::TableFiles       *toFiles();
        /// @brief Casts the table if possible. Returns NULL otherwise.
        LightBird::TableGroups      *toGroups();
        /// @brief Casts the table if possible. Returns NULL otherwise.
        LightBird::TableLimits      *toLimits();
        /// @brief Casts the table if possible. Returns NULL otherwise.
        LightBird::TableObjects     *toObjects();
        /// @brief Casts the table if possible. Returns NULL otherwise.
        LightBird::TablePermissions *toPermissions();
        /// @brief Casts the table if possible. Returns NULL otherwise.
        LightBird::TableTags        *toTags();

    protected:
        Table();
        Table(const Table &table);
        Table &operator=(const LightBird::Table &table);

        QString id;
        QString tableName;
        LightBird::Table::Id tableId;
    };
}

#endif // LIGHTBIRD_TABLE_H
