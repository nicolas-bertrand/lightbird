#ifndef LIGHTBIRD_IDATABASE_H
# define LIGHTBIRD_IDATABASE_H

# include <QDateTime>
# include <QMap>
# include <QSharedPointer>
# include <QSqlDatabase>
# include <QSqlQuery>
# include <QString>
# include <QStringList>
# include <QVariant>
# include <QVector>

# include "Table.h"

namespace LightBird
{
    /// @brief Manages the access to the database.
    /// This interface is threade safe.
    class IDatabase
    {
    public:
        virtual ~IDatabase() {}

        /// @brief The state of the entry.
        enum State
        {
            ADDED,      ///< The entry has been added.
            MODIFIED,   ///< The entry has been modified.
            DELETED     ///< The entry has been deleted.
        };

        /// Lists all the modifications of the database. The first dimension is
        /// the name of the table, the second is the state of the rows, the third
        /// are the rows, and the last is the field/value pairs.
        typedef QMap<QString, QMap<LightBird::IDatabase::State, QList<QMap<QString, QVariant> > > > Updates;

        /// @brief Returns the database connection of the current thread. Each
        /// thread has its own database connection which must be given to
        /// QSqlQuery during its creation.
        virtual QSqlDatabase getDatabase() = 0;
        /// @brief Executes a SQL query. The QSqlQuery in parameter allows to secure
        /// the variables into the request by calling prepare() and bindValue(). This
        /// prevent SQL injections. You can also use it to get the error of the last
        /// query. This method can be used for all the statements, except for SELECT.
        /// @param request : The SQL query.
        /// @return False if an error occurred, true otherwise.
        virtual bool    query(QSqlQuery &query) = 0;
        /// @brief Executes the SELECT statement of a SQL query, and returns its results.
        /// @param request : The SQL query.
        /// @param result : The result of the query is stored in this variable (only
        /// for SELECT). The vector represents the rows, and the map represents the
        /// fields. Plugins can access to a value like this : result[0]["field"].
        /// But first, plugins have to test that the row 1 exists.
        /// @return False if an error occurred, true otherwise.
        virtual bool    query(QSqlQuery &query, QVector<QVariantMap> &result) = 0;
        /// @brief Executes the SELECT statement of a SQL query, and returns the first result.
        /// @param request : The SQL query.
        /// @param result : The first result of the query is stored in this variable (only for SELECT).
        /// The map represents the fields. Plugins can access to a value like this : result["field"]. 
        /// @return False if an error occurred or no result was found, true otherwise.
        virtual bool    query(QSqlQuery &query, QVariantMap &result) = 0;
        /// @brief Returns an instance of the table requested in parameter. Users
        /// MUST delete themself the instance returned, or a memory leak will occur.
        /// @param table : The table to get. If the table is accessor, object or unknow
        /// this method will search the table in the database based on the provided id.
        /// @param id : The id of the row to put by default in the table instance.
        /// @return NULL if the table has not been found.
        /// @example getTable
        /// This example shows how to cast Table into TableAccounts :
        /// LightBird::TableAccounts *account = getTable(LightBird::Table::Accounts)->toAccounts();
        /// delete account; // Do not forget to delete the instance.
        ///
        /// Users can also use a shared pointer, so that the instance is deleted
        /// automatically when it goes out of scope :
        /// QSharedPointer<LightBird::TableAccounts> account(getTable(LightBird::Table::Accounts)->toAccounts());
        virtual LightBird::Table    *getTable(LightBird::Table::Id table, const QString &id = "") = 0;
        /// @brief This method helps to make the queries independent of the database
        /// type used. The SQL queries of the plugins are stored in a XML file named
        /// QtSqlDriverName.xml, instead of being hard-coded. Each query has a unique
        /// name in his group. getQuery() allows plugins to load a query.
        /// @param group : The name of the group of the query.
        /// @param name : The name of the query.
        /// @return The query or an empty string if it is not found.
        virtual QString getQuery(const QString &group, const QString &name) = 0;
        /// @brief Checks if the database has been updated since the date in parameter,
        /// and returns the list of the modifications.
        /// @param updates : Contains all the modification made on the database
        /// that match the filters. The delete state only stores the id of the row.
        /// @param date : The date in local time from which to check the updates.
        /// If null, all the rows are listed as ADDED.
        /// @param tables : The list of the tables for which the updates are checked.
        /// If empty, all the tables are taken.
        /// @return True if the database has been modified.
        virtual bool    updates(LightBird::IDatabase::Updates &updates, const QDateTime &date = QDateTime(), const QStringList &tables = QStringList()) = 0;
    };
}

Q_DECLARE_INTERFACE(LightBird::IDatabase, "cc.lightbird.IDatabase")

#endif // LIGHTBIRD_IDATABASE_H
