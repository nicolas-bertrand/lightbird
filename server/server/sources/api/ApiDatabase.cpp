#include "ApiDatabase.h"
#include "Database.h"
#include "Log.h"

#include "ITableAccessors.h"
#include "ITableObjects.h"

ApiDatabase::ApiDatabase(const QString &id)
{
    this->id = id;
}

ApiDatabase::~ApiDatabase()
{
}

bool    ApiDatabase::query(QSqlQuery &query)
{
    return (Database::instance()->query(query));
}

bool    ApiDatabase::query(QSqlQuery &query, QVector<QMap<QString, QVariant> > &result)
{
    return (Database::instance()->query(query, result));
}

LightBird::ITable   *ApiDatabase::getTable(LightBird::ITable::Tables table, const QString &id)
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

QSharedPointer<LightBird::ITableAccounts>    ApiDatabase::getAccounts(const QString &id)
{
    return (QSharedPointer<LightBird::ITableAccounts>(Database::instance()->getTable(LightBird::ITable::Accounts, id)->toAccounts()));
}

QSharedPointer<LightBird::ITableCollections> ApiDatabase::getCollections(const QString &id)
{
    return (QSharedPointer<LightBird::ITableCollections>(Database::instance()->getTable(LightBird::ITable::Collections, id)->toCollections()));
}

QSharedPointer<LightBird::ITableDirectories> ApiDatabase::getDirectories(const QString &id)
{
    return (QSharedPointer<LightBird::ITableDirectories>(Database::instance()->getTable(LightBird::ITable::Directories, id)->toDirectories()));
}

QSharedPointer<LightBird::ITableEvents>      ApiDatabase::getEvents(const QString &id)
{
    return (QSharedPointer<LightBird::ITableEvents>(Database::instance()->getTable(LightBird::ITable::Events, id)->toEvents()));
}

QSharedPointer<LightBird::ITableFiles>       ApiDatabase::getFiles(const QString &id)
{
    return (QSharedPointer<LightBird::ITableFiles>(Database::instance()->getTable(LightBird::ITable::Files, id)->toFiles()));
}

QSharedPointer<LightBird::ITableGroups>      ApiDatabase::getGroups(const QString &id)
{
    return (QSharedPointer<LightBird::ITableGroups>(Database::instance()->getTable(LightBird::ITable::Groups, id)->toGroups()));
}

QSharedPointer<LightBird::ITableLimits>      ApiDatabase::getLimits(const QString &id)
{
    return (QSharedPointer<LightBird::ITableLimits>(Database::instance()->getTable(LightBird::ITable::Limits, id)->toLimits()));
}

QSharedPointer<LightBird::ITablePermissions> ApiDatabase::getPermissions(const QString &id)
{
    return (QSharedPointer<LightBird::ITablePermissions>(Database::instance()->getTable(LightBird::ITable::Permissions, id)->toPermissions()));
}

QSharedPointer<LightBird::ITableTags>        ApiDatabase::getTags(const QString &id)
{
    return (QSharedPointer<LightBird::ITableTags>(Database::instance()->getTable(LightBird::ITable::Tags, id)->toTags()));
}
