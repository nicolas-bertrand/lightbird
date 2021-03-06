#ifndef DATABASE_H
# define DATABASE_H

# include <QDateTime>
# include <QDomDocument>
# include <QMap>
# include <QMutex>
# include <QObject>
# include <QSqlDatabase>
# include <QString>
# include <QStringList>
# include <QVariant>
# include <QVector>

# include "IDatabase.h"

# include "Initialize.h"

/// @brief Manages all the operations made on the database by the server and
/// its plugins. Opens one database connection per thread using it.
class Database : public QObject,
                 public LightBird::Initialize
{
    Q_OBJECT

public:
    Database(QObject *parent = 0);
    ~Database();

    /// @see LightBird::IDatabase::getDatabase
    QSqlDatabase    getDatabase();
    /// @see LightBird::IDatabase::query
    bool            query(QSqlQuery &query);
    /// @see LightBird::IDatabase::query
    bool            query(QSqlQuery &query, QVector<QVariantMap> &result);
    /// @see LightBird::IDatabase::query
    bool            query(QSqlQuery &query, QVariantMap &result);
    /// @see LightBird::IDatabase::getTable
    LightBird::Table *getTable(LightBird::Table::Id table, const QString &id = "");
    /// @see LightBird::IDatabase::getQuery
    QString         getQuery(const QString &group, const QString &name, const QString &id = "");
    /// @see LightBird::IDatabase::updates
    bool            updates(LightBird::IDatabase::Updates &updates, const QDateTime &date = QDateTime(), const QStringList &tables = QStringList());
    /// @brief Returns the instance of this class created by the Server.
    static Database *instance();

private slots:
    /// @brief Removes the database connection of the caller thread. This slot
    /// is called when a thread that opened a database connection is finished.
    void            _removeDatabase();

private:
    Database(const Database &);
    Database &operator=(const Database &);

    /// @brief Etablishes a new connection between the server and the database.
    /// @return True if the connection to the database has succeeded.
    bool            _addDatabase(const QString &connectionName);
    /// @brief Gets the name of the database, and manage the database file if
    /// their is one (for example with SQLite).
    /// @param name : The name of the database, which can be the path to its
    /// file (with SQLite).
    /// @return If an error occurred while managing the database file.
    bool            _getDatabaseName(QString &databaseName);
    /// @brief Loads a query file.
    /// @param id : The id of the plugin for which the quety will be loaded.
    /// If empty, the queries of the server are loaded.
    /// @return True if the file is loaded.
    bool            _loadQueries(const QString &id);
    /// @brief Ensures that there is no NULL values in the bound values,
    /// by replacing them by an empty string.
    /// @param query : The query to check.
    void            _checkBoundValues(QSqlQuery &query) const;
    /// @brief Displays the updates. For debug purpose only.
    void            _displayUpdates(LightBird::IDatabase::Updates &updates) const;

    QMap<QString, QDomDocument> queries;     ///< Contains the doms representations of the queries.
    QStringList                 tablesNames; ///< Contains the names of the tables of the database.
    QMutex                      mutex;       ///< Makes this class thread safe.
};

#endif // DATABASE_H
