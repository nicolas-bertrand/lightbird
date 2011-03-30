#ifndef DATABASE_H
# define DATABASE_H

# include <QDomDocument>
# include <QSqlDatabase>
# include <QSqlError>
# include <QMutex>

# include "IDatabase.h"

/**
 * @brief Manage all the operations made on the database, by the
 * server and its plugins.
 */
class Database : public QObject
{
    Q_OBJECT

public:
    static Database *instance(QObject *parent = 0);

    /// @see Streamit::IDatabase::query
    bool            query(QSqlQuery &query);
    /// @see Streamit::IDatabase::query
    bool            query(QSqlQuery &query, QVector<QMap<QString, QVariant> > &result);
    /// @see Streamit::IDatabase::getTable
    Streamit::ITable    *getTable(Streamit::ITable::Tables table, const QString &id = "");
    /// @see Streamit::IDatabase::getQuery
    QString         getQuery(const QString &group, const QString &name);
    /// @see Streamit::IDatabase::getQuery
    QString         getQuery(const QString &group, const QString &name, const QString &id);
    /// @see Streamit::IDatabase::updates
    bool            updates(Streamit::IDatabase::Updates &updates, const QDateTime &date = QDateTime(), const QStringList &tables = QStringList());
    /// @brief Returns the names of the tables of the database.
    QStringList     getTableNames();

private:
    Database(QObject *parent = 0);
    ~Database();
    Database(const Database &);
    Database        *operator=(const Database &);

    /**
     * @brief Get the name of the database, and manage the database file if their is one (for example with SQLite).
     * @param name : The name of the database, which can be the path to its file (with SQLite).
     * @return If an error occured while managing the database file
     */
    bool            _name(QString &name);
    /**
     * @brief Etablished the connection between the server and the database.
     * @return True if the connection to the database success
     */
    bool            _connection();
    /// @brief Display the updates. For debug purpose only.
    void            _displayUpdates(Streamit::IDatabase::Updates &updates);
    /**
     * @brief Load a query file.
     * @param id : The id of the plugin for which the quety will be loaded. If empty,
     * the queries of the server are loaded.
     * @return True if the file is loaded.
     */
    bool            _loadQueries(const QString &id);
    /// @brief Ensure that there is no NULL values in the bound values, and replace them by
    /// an empty string.
    /// @param query : The query to check.
    void            _checkBoundValues(QSqlQuery &query);

    static Database             *_instance;     ///< The instance of the singleton that manage the database.
    bool                        loaded;         ///< If the database has been correctly loaded.
    QMap<QString, QDomDocument> queries;        ///< Contains the doms representations of the queries.
    QMutex                      lockQueries;    ///< Ensure that the queries are modified by one thread at a time.
    QStringList                 tablesNames;    ///< Contains the names of the tables of the database.
};

#endif // DATABASE_H
