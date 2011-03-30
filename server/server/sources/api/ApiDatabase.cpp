#include "Log.h"
#include "Database.h"
#include "ApiDatabase.h"

ApiDatabase::ApiDatabase(const QString &id, QObject *parent) : QObject(parent)
{
    this->id = id;
}

ApiDatabase::~ApiDatabase()
{
    Log::trace("ApiDatabase destroyed!", Properties("id", this->id), "ApiDatabase", "~ApiDatabase");
}

bool    ApiDatabase::query(QSqlQuery &query)
{
    return (Database::instance()->query(query));
}

bool    ApiDatabase::query(QSqlQuery &query, QVector<QMap<QString, QVariant> > &result)
{
    return (Database::instance()->query(query, result));
}

Streamit::ITable    *ApiDatabase::getTable(Streamit::ITable::Tables table, const QString &id)
{
    return (Database::instance()->getTable(table, id));
}

QString ApiDatabase::getQuery(const QString &group, const QString &name)
{
    return (Database::instance()->getQuery(group, name, this->id));
}

bool    ApiDatabase::updates(Streamit::IDatabase::Updates &updates, const QDateTime &date, const QStringList &tables)
{
    return (Database::instance()->updates(updates, date, tables));
}
