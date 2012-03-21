#ifndef APIDATABASE_H
# define APIDATABASE_H

# include <QObject>

# include "IDatabase.h"

/// @brief The server implementation of the IDatabase interface.
class ApiDatabase : public QObject,
                    public LightBird::IDatabase
{
    Q_OBJECT
    Q_INTERFACES(LightBird::IDatabase)

public:
    ApiDatabase(const QString &id = "");
    ~ApiDatabase();

    bool                query(QSqlQuery &query);
    bool                query(QSqlQuery &query, QVector<QVariantMap> &result);
    bool                query(QSqlQuery &query, QVariantMap &result);
    LightBird::Table    *getTable(LightBird::Table::Id table, const QString &id = "");
    QString             getQuery(const QString &group, const QString &name);
    bool                updates(LightBird::IDatabase::Updates &updates, const QDateTime &date = QDateTime(), const QStringList &tables = QStringList());

private:
    ApiDatabase(const ApiDatabase &);
    ApiDatabase &operator=(const ApiDatabase &);

    QString id; ///< The id of the plugin for which the object has been created.
};

#endif // APIDATABASE_H
