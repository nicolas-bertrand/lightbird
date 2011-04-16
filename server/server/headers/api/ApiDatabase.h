#ifndef APIDATABASE_H
# define APIDATABASE_H

# include "IDatabase.h"

/// @brief The server implementation of the IDatabase interface.
class ApiDatabase : public QObject,
                    public LightBird::IDatabase
{
    Q_OBJECT
    Q_INTERFACES(LightBird::IDatabase)

public:
    ApiDatabase(const QString &id);
    ~ApiDatabase();

    bool                query(QSqlQuery &query);
    bool                query(QSqlQuery &query, QVector<QMap<QString, QVariant> > &result);
    LightBird::ITable   *getTable(LightBird::ITable::Tables table, const QString &id = "");
    QString             getQuery(const QString &group, const QString &name);
    QString             getQuery(const QString &group, const QString &name, const QString &id);
    bool                updates(LightBird::IDatabase::Updates &updates, const QDateTime &date = QDateTime(), const QStringList &tables = QStringList());

private:
    ApiDatabase();
    ApiDatabase(const ApiDatabase &);
    ApiDatabase         *operator=(const ApiDatabase &);

    QString             id; ///< The id of the plugin for which the object has been created.
};

#endif // APIDATABASE_H
