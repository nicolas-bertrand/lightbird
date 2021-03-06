#include "ApiDatabase.h"
#include "Database.h"

ApiDatabase::ApiDatabase(const QString &id)
{
    this->id = id;
}

ApiDatabase::~ApiDatabase()
{
}

QSqlDatabase    ApiDatabase::getDatabase()
{
    return (Database::instance()->getDatabase());
}

bool    ApiDatabase::query(QSqlQuery &query)
{
    return (Database::instance()->query(query));
}

bool    ApiDatabase::query(QSqlQuery &query, QVector<QVariantMap> &result)
{
    return (Database::instance()->query(query, result));
}

bool    ApiDatabase::query(QSqlQuery &query, QVariantMap &result)
{
    return (Database::instance()->query(query, result));
}

LightBird::Table    *ApiDatabase::getTable(LightBird::Table::Id table, const QString &id)
{
    return (Database::instance()->getTable(table, id));
}

QString ApiDatabase::getQuery(const QString &group, const QString &name)
{
    return (Database::instance()->getQuery(group, name, this->id));
}

bool    ApiDatabase::updates(LightBird::IDatabase::Updates &updates, const QDateTime &date, const QStringList &tables)
{
    return (Database::instance()->updates(updates, date, tables));
}
