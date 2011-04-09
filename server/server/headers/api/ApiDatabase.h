#ifndef APIDATABASE_H
# define APIDATABASE_H

# include "IDatabase.h"

class ApiDatabase : public QObject,
                    public LightBird::IDatabase
{
    Q_OBJECT
    Q_INTERFACES(LightBird::IDatabase)

public:
    ApiDatabase(const QString &id, QObject *parent = 0);
    ~ApiDatabase();

    // IDatabase
    /// @see LightBird::IPlugins::query
    bool                query(QSqlQuery &query);
    /// @see LightBird::IPlugins::query
    bool                query(QSqlQuery &query, QVector<QMap<QString, QVariant> > &result);
    /// @see LightBird::IPlugins::getTable
    LightBird::ITable   *getTable(LightBird::ITable::Tables table, const QString &id = "");
    /// @see LightBird::IPlugins::getQuery
    QString             getQuery(const QString &group, const QString &name);
    /// @see LightBird::IPlugins::getQuery
    QString             getQuery(const QString &group, const QString &name, const QString &id);
    /// @see LightBird::IPlugins::updates
    bool                updates(LightBird::IDatabase::Updates &updates, const QDateTime &date = QDateTime(), const QStringList &tables = QStringList());

private:
    ApiDatabase();
    ApiDatabase(const ApiDatabase &);
    ApiDatabase     *operator=(const ApiDatabase &);

    QString         id;
};

#endif // APIDATABASE_H
