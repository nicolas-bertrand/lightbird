#ifndef IDATABASE_H
# define IDATABASE_H

# include <QVector>
# include <QMap>
# include <QString>
# include <QStringList>
# include <QVariant>
# include <QSqlError>
# include <QSqlQuery>
# include <QDateTime>
# include <QSharedPointer>

#include "ITable.h"

namespace Streamit
{
    /**
     * @brief Handle the access to the database.
     * This interface is threade safe.
     */
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

        /// List all the modifications of the database. The first dimension is the name of the table, the second
        /// is the state of the rows, the third are the rows, and the last is the field/value pairs.
        typedef QMap<QString, QMap<Streamit::IDatabase::State, QList<QMap<QString, QVariant> > > > Updates;

        /**
         * @brief Execute a SQL query. The QSqlQuery in parameter allows to secure
         * the variables into the request by calling prepare() and bindValue(). This
         * prevent SQL injections. You can also use it to get the error of the last
         * query. This method can be used for all the statements, except for SELECT.
         * @param request : The SQL query.
         * @return False if an error occured, true otherwise.
         */
        virtual bool    query(QSqlQuery &query) = 0;
        /**
         * @brief Execute the SELECT statement of a SQL query, and returns its results.
         * @param request : The SQL query.
         * @param result : The result of the query is stored in this variable (only for SELECT).
         * The vector represents the rows, and the map represents the fields. Plugins can access
         * to a value like this : result[1]["field"]. But first, plugins have to test that the
         * row 1 exists.
         * @return False if an error occured, true otherwise.
         */
        virtual bool    query(QSqlQuery &query, QVector<QMap<QString, QVariant> > &result) = 0;
        /**
         * @brief Returns an instance of the table gived in parameter. Users MUST delete themself the
         * instance returned, or a memory leak will occure.
         * @param table : The table to get. If the table is unknow, this
         * method will search in the database the table of the id.
         * @param id : The id of the row to put by default in the table instance.
         *
         * @example This example shows how to cast ITable into ITableAccounts :
         * Streamit::ITableAccounts *account = getTable(Streamit::ITable::Accounts)->toTableAccounts();
         * delete account; // Do not forget to delete the instance.
         *
         * Users can also use a shared pointer, so that the instance is delete automatically when it goes out of scope :
         * QSharedPointer<Streamit::ITableAccounts> account(getTable(Streamit::ITable::Accounts)->toTableAccounts());
         */
        virtual Streamit::ITable    *getTable(Streamit::ITable::Tables table, const QString &id = "") = 0;
        /**
         * @brief This method helps to make the queries independent of the database type used. The
         * SQL queries of the plugins are stored in a XML file named QtSqlDriverName.xml, instead of being
         * hard-coded. Each query has a unique name in his group. getQuery() allows plugins to load a query.
         * @param group : The name of the group of the query.
         * @param name : The name of the query.
         * @return The query or an empty string if it is not found.
         */
        virtual QString getQuery(const QString &group, const QString &name) = 0;
        /**
         * @brief Check if the database has been updated since the date in parameter, and returns the
         * list of the modifications.
         * @param updates : Contains all the modification made on the database that match the filters.
         * modified rows. The delete state only stores the id of the row.
         * @param date : The date from which to check the updates. If null, all the rows are listed as ADDED.
         * @param tables : The list of the tables for which the updates are checked. If empty, all
         * the tables are taken.
         * @return True if the database has been modified.
         */
        virtual bool    updates(Streamit::IDatabase::Updates &updates, const QDateTime &date = QDateTime(), const QStringList &tables = QStringList()) = 0;
    };
}

Q_DECLARE_INTERFACE (Streamit::IDatabase, "fr.streamit.IDatabase");

#endif // IDATABASE_H
