#ifndef APIDATABASE_H
# define APIDATABASE_H

# include "IDatabase.h"

class ApiDatabase : public QObject,
                    public Streamit::IDatabase
{
    Q_OBJECT
    Q_INTERFACES(Streamit::IDatabase)

public:
    ApiDatabase(const QString &id, QObject *parent = 0);
    ~ApiDatabase();

    // IDatabase
    /// @see Streamit::IPlugins::query
    bool                query(QSqlQuery &query);
    /// @see Streamit::IPlugins::query
    bool                query(QSqlQuery &query, QVector<QMap<QString, QVariant> > &result);
    /// @see Streamit::IPlugins::getTable
    Streamit::ITable    *getTable(Streamit::ITable::Tables table, const QString &id = "");
    /// @see Streamit::IPlugins::getQuery
    QString             getQuery(const QString &group, const QString &name);
    /// @see Streamit::IPlugins::getQuery
    QString             getQuery(const QString &group, const QString &name, const QString &id);
    /// @see Streamit::IPlugins::updates
    bool                updates(Streamit::IDatabase::Updates &updates, const QDateTime &date = QDateTime(), const QStringList &tables = QStringList());

private:
    ApiDatabase();
    ApiDatabase(const ApiDatabase &);
    ApiDatabase     *operator=(const ApiDatabase &);

    QString         id;
};

#endif // APIDATABASE_H
