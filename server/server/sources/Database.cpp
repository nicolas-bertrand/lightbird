#include <QDir>
#include <QSqlRecord>
#include <iostream>

#include "Defines.h"
#include "Log.h"
#include "Configurations.h"
#include "Table.h"
#include "Plugins.hpp"
#include "TableAccounts.h"
#include "TableCollections.h"
#include "TableDirectories.h"
#include "TableEvents.h"
#include "TableFiles.h"
#include "TableGroups.h"
#include "TableLimits.h"
#include "TablePermissions.h"
#include "TableTags.h"
#include "Database.h"

Database    *Database::_instance = NULL;

Database    *Database::instance(QObject *parent)
{
    if (Database::_instance == NULL)
    {
        Database::_instance = new Database(parent);
        if (!Database::_instance->loaded)
        {
            delete Database::_instance;
            Database::_instance = NULL;
        }
    }
    return (Database::_instance);
}

Database::Database(QObject *parent) : QObject(parent)
{
    this->loaded = false;
    // Stores the names of the tables of the database
    this->tablesNames << "accounts" << "accounts_groups" << "accounts_informations" << "collections"
                      << "directories" << "events" << "events_informations" << "files" << "files_collections"
                      << "files_informations" << "groups" << "limits" << "permissions" << "tags";
    // Connect the server to the database
    if (!this->_connection())
    {
        Log::error("Connection to the database failed", "Database", "Database");
        return ;
    }
    this->loaded = true;
}

Database::~Database()
{
    QSqlDatabase::database().close();
    Log::trace("Database destroyed!", "Database", "~Database");
}

bool        Database::query(QSqlQuery &query)
{
    bool    error;

    this->_checkBoundValues(query);
    // Execute the request
    if ((error = query.exec()) == false)
        Log::error("An SQL error occured", Properties("query", query.lastQuery()).add("error", query.lastError().text()).add(query.boundValues()), "Database", "query");
    // If the log is in trace, all the informations on the query are saved
    else if (Log::instance()->isTrace())
        Log::trace("SQL query executed", Properties("query", query.lastQuery()).add(query.boundValues()).add("rows", query.numRowsAffected()), "Database", "query");
    return (error);
}

bool                        Database::query(QSqlQuery &query, QVector<QMap<QString, QVariant> > &result)
{
    bool                    error;
    int                     count;
    int                     r;
    QMap<QString, QVariant> row;

    this->_checkBoundValues(query);
    // Execute the query
    if ((error = query.exec()) == false)
        Log::error("An SQL error occured", Properties("query", query.lastQuery()).add("error", query.lastError().text()).add(query.boundValues()), "Database", "query");
    // Put the result in the reference
    else
    {
        result.clear();
        count = query.record().count();
        while (query.next())
        {
            for (r = 0; r < count; ++r)
                row.insert(query.record().fieldName(r), query.value(r));
            if (!row.empty())
                result.push_back(row);
            row.clear();
        }
        // If the log is in trace, all the informations on the query are saved
        if (Log::instance()->isTrace())
            Log::trace("SQL query executed", Properties("query", query.lastQuery()).add(query.boundValues()).add("rows", result.size()), "Database", "query");
    }
    return (error);
}

LightBird::ITable   *Database::getTable(LightBird::ITable::Tables table, const QString &id)
{
    QStringList                         tables;
    QString                             name;
    QSqlQuery                           query;
    QVector<QMap<QString, QVariant> >   result;

    // If the table is unknow, tries to find it using the id of the row
    if (!id.isEmpty() && (table == LightBird::ITable::Unknow || table == LightBird::ITable::Accessor || table == LightBird::ITable::Object))
    {
        // Defines the names of the tables where the existance of the id will be checked
        if (table == LightBird::ITable::Accessor)
            tables << "accounts" << "groups";
        else if (table == LightBird::ITable::Object)
            tables << "files" << "directories" << "collections";
        else
            tables = this->tablesNames;
        // Search the id
        QStringListIterator it(tables);
        while (it.hasNext() && name.isEmpty())
        {
            query.prepare(Database::instance()->getQuery("Table", "getTable").replace(":table", it.peekNext()));
            query.bindValue(":id", id);
            if (!Database::instance()->query(query, result))
                return (NULL);
            // The table has been found
            if (result.size() > 0)
                name = it.peekNext();
            it.next();
        }
        // If the table name has not been found
        if (name.isEmpty())
            return (NULL);
    }
    if (table == LightBird::ITable::Accounts || name == "accounts")
        return (new TableAccounts(id));
    if (table == LightBird::ITable::Collections || name == "collections")
        return (new TableCollections(id));
    if (table == LightBird::ITable::Directories || name == "directories")
        return (new TableDirectories(id));
    if (table == LightBird::ITable::Events || name == "events")
        return (new TableEvents(id));
    if (table == LightBird::ITable::Files || name == "files")
        return (new TableFiles(id));
    if (table == LightBird::ITable::Groups || name == "groups")
        return (new TableGroups(id));
    if (table == LightBird::ITable::Limits || name == "limits")
        return (new TableLimits(id));
    if (table == LightBird::ITable::Permissions || name == "permissions")
        return (new TablePermissions(id));
    if (table == LightBird::ITable::Tags || name == "tags")
        return (new TableTags(id));
    return (NULL);
}

QString Database::getQuery(const QString &group, const QString &name)
{
    return (this->getQuery(group, name, ""));
}

QString         Database::getQuery(const QString &group, const QString &name, const QString &id)
{
    QString     result;

    if (!this->lockQueries.tryLock(MAXTRYLOCK))
    {
        Log::error("Deadlock", "Database", "getQuery");
        return ("");
    }
    if (!this->queries.contains(id))
        if (!this->_loadQueries(id))
        {
            lockQueries.unlock();
            return ("");
        }
    if (!(result = this->queries[id].firstChildElement().firstChildElement(group).firstChildElement(name).text()).isEmpty())
    {
        this->lockQueries.unlock();
        return (result);
    }
    Log::warning("Query not found", Properties("group", group).add("name", name).add("id", id), "Database", "getQuery");
    this->lockQueries.unlock();
    return ("");
}

bool                                    Database::updates(LightBird::IDatabase::Updates &updates, const QDateTime &d, const QStringList &t)
{
    QVector<QMap<QString, QVariant> >   result;
    LightBird::IDatabase::State         state;
    QString                             date;
    QSqlQuery                           query;
    QStringList                         tables;
    QString                             table;
    int                                 i;
    int                                 s;

    date = d.toString(DATE_FORMAT);
    tables = t;
    // If empty, all the tables are checked
    if (tables.isEmpty())
        tables = this->tablesNames;
    // Get the updated row of the requested tables
    QStringListIterator it(tables);
    while (it.hasNext())
    {
        // If the date is not defined, all the rows are listed as ADDED
        if (!d.isValid())
            query.prepare(this->getQuery("Database", "update_select_all").replace(":table", it.peekNext()));
        // Otherwise, we take only the updated rows
        else
        {
            query.prepare(this->getQuery("Database", "update_select_modified_created").replace(":table", it.peekNext()));
            query.bindValue(":modified", date);
            query.bindValue(":created", date);
        }
        this->query(query, result);
        for (i = 0, s = result.size(); i < s; ++i)
        {
            if (result[i]["created"].toString() >= date || !d.isValid())
                state = LightBird::IDatabase::ADDED;
            else
                state = LightBird::IDatabase::MODIFIED;
            updates[it.peekNext()][state].push_back(result[i]);
        }
        result.clear();
        it.next();
    }
    // Get the deleted rows
    if (d.isValid())
    {
        query.prepare(this->getQuery("Database", "update_select_deleted"));
        query.bindValue(":date", date);
        this->query(query, result);
        for (i = 0, s = result.size(); i < s; ++i)
        {
            table = result[i]["table"].toString();
            state = LightBird::IDatabase::DELETED;
            result[i].remove("table");
            updates[table][state].push_back(result[i]);
        }
    }
    if (updates.size())
        return (true);
    return (false);
}

bool                Database::_connection()
{
    QSqlDatabase    database;
    QString         name;
    QDomElement     e;
    QString         options;

    // Creates the database manager
    if (!(database = QSqlDatabase::addDatabase(Configurations::instance()->get("database/type"))).isValid())
    {
        Log::error("Unvalid database type", Properties("type", Configurations::instance()->get("database/type")), "Database", "_connection");
        return (false);
    }
    // Set the database informations from the configutation
    database.setUserName(Configurations::instance()->get("database/user"));
    database.setPassword(Configurations::instance()->get("database/password"));
    database.setHostName(Configurations::instance()->get("database/host"));
    database.setPort(Configurations::instance()->get("database/port").toInt());
    if (!this->_name(name))
        return (false);
    database.setDatabaseName(name);
    // Set the connection options
    e = Configurations::instance()->readDom().firstChildElement("database").firstChildElement("options").firstChildElement();
    while (!e.isNull())
    {
        if (!options.isEmpty())
            options += ";";
        options += e.tagName() + "=" + e.text().trimmed();
        e = e.nextSiblingElement();
    }
    Configurations::instance()->release();
    database.setConnectOptions(options);
    // Open the connection
    if (!database.open())
    {
        Log::error("Cannot open the database", Properties("name", database.databaseName()).add("type", Configurations::instance()->get("database/type")).add("port", QString::number(database.port()))
                   .add("user", database.userName()).add("password", database.password()).add("host", database.hostName()).add("options", options), "Database", "_connection");
        return (false);
    }
    Log::debug("Database connected", Properties("name", database.databaseName()).add("type", Configurations::instance()->get("database/type"))
               .add("port", QString::number(database.port())).add("user", database.userName()).add("host", database.hostName()).add("options", options), "Database", "_connection");
    // Execute pragmas
    e = Configurations::instance()->readDom().firstChildElement("database").firstChildElement("pragmas").firstChildElement();
    while (!e.isNull())
    {
        if (e.tagName() == "pragma")
        {
            QSqlQuery query;
            query.prepare(e.text());
            this->query(query);
        }
        e = e.nextSiblingElement();
    }
    Configurations::instance()->release();
    database.setConnectOptions(options);
    return (true);
}

bool        Database::_name(QString &name)
{
    QDir    dir;
    QString file;
    QString databasePath;
    QString databaseFile;
    QString databaseResource;

    // If the name is defined, the database is server based (like MySQL), not file based (like SQLite)
    if (!(name = Configurations::instance()->get("database/name")).isEmpty())
    {
        Log::debug("The database is server based", "Database", "_name");
        return (true);
    }
    Log::debug("The database is file based", "Database", "_name");

    // Get the path and the file name of the database
    databasePath = Configurations::instance()->get("database/path");
    databaseFile = Configurations::instance()->get("database/file");
    file = databasePath + "/" + databaseFile;

    // If the database directory doesn't exists, we creates it
    if (dir.exists(databasePath) == false && dir.mkpath(databasePath) == false)
    {
        Log::error("Cannot creates the database directory", Properties("directory", databasePath), "Database", "_name");
        return (false);
    }

    // If the database file doesn't exists, we creates it using the resource
    if (!QFileInfo(file).isFile())
    {
        // Get the resource of the database
        databaseResource = Configurations::instance()->get("database/resource");
        Log::debug("The database file doesn't exists, so it will be created using the alternative file", Properties("file", file).add("alternative", databaseResource), "Database", "_name");
        // If the resource file doesn't exists either
        if (!QFileInfo(databaseResource).isFile())
        {
            Log::error("The alternative database file doesn't exists either", Properties("alternative", databaseResource), "Database", "Database");
            return (false);
        }
        // Copy the resource file to the database file
        if (Configurations::copy(databaseResource, file) == false)
        {
            Log::error("Cannot creates the database file from the alternative file", Properties("file", file).add("alternative", databaseResource), "Database", "_name");
            return (false);
        }
    }
    name = file;
    return (true);
}

bool        Database::_loadQueries(const QString &id)
{
    QString type;
    QString path;
    QFile   file;
    QString errorMsg;
    int     errorLine;
    int     errorColumn;
    QString resourcePath;

    if (this->queries.contains(id))
        return (false);
    type = Configurations::instance()->get("database/type");
    // Defines the path to the queries file
    if (id.isEmpty())
    {
        path = Configurations::instance()->get("database/path");
        if (path.isEmpty())
            path = DEFAULT_DATABASE_PATH;
    }
    else
        path = id;
    // Search the queries file using the database type
    if (QFileInfo(path + "/" + type + ".xml").isFile())
        file.setFileName(path + "/" + type + ".xml");
    // Using the default file name
    else if (QFileInfo(path + "/Queries.xml").isFile())
        file.setFileName(path + "/Queries.xml");
    // In the resources of the server
    else if (id.isEmpty() && QFileInfo(":queries").isFile())
        file.setFileName(":queries");
    // In the resources the plugin
    else if (!id.isEmpty() && !(resourcePath = Plugins::instance()->getResourcesPath(id)).isEmpty() && QFileInfo(resourcePath + "/" + type).isFile())
        file.setFileName(resourcePath + "/" + type);
    // In the default resources the plugin
    else if (!id.isEmpty() && !resourcePath.isEmpty() && QFileInfo(resourcePath + "/queries").isFile())
        file.setFileName(resourcePath + "/queries");
    // The file is not found
    else
    {
        Log::error("Unable to find a valid query file", Properties("id", id), "Database", "_loadQueries");
        return (false);
    }

    // Open the queties file
    if (!file.open(QIODevice::ReadOnly))
    {
        Log::error("Unable to load the file" + file.fileName(), "Database", "_loadQueries");
        return (false);
    }
    // Parse the XML file
    if (this->queries[id].setContent(&file, false, &errorMsg, &errorLine, &errorColumn) == false)
    {
        Log::error("An error occured while parsing the configuration file", Properties("message", errorMsg).add("file", file.fileName())
                   .add("line", QString::number(errorLine)).add("column", QString::number(errorColumn)), "Configuration", "_load");
        return (false);
    }
    Log::debug("Queries loaded", Properties("file", file.fileName()), "Database", "_loadQueries");
    file.close();
    return (true);
}

void    Database::_displayUpdates(LightBird::IDatabase::Updates &updates)
{
    QMapIterator<QString, QMap<LightBird::IDatabase::State, QList<QMap<QString, QVariant> > > > i1 (updates);
    while (i1.hasNext())
    {
        i1.next();
        std::cout << "===== " << i1.key().toStdString() << " =====" << std::endl;
        QMapIterator<LightBird::IDatabase::State, QList<QMap<QString, QVariant> > > i2(i1.value());
        while (i2.hasNext())
        {
            i2.next();
            if (i2.key() == LightBird::IDatabase::ADDED)
                std::cout << "> ADDED" << std::endl;
            else if (i2.key() == LightBird::IDatabase::MODIFIED)
                std::cout << "> MODIFIED" << std::endl;
            else if (i2.key() == LightBird::IDatabase::DELETED)
                std::cout << "> DELETED" << std::endl;
            QListIterator<QMap<QString, QVariant> > i3(i2.value());
            while (i3.hasNext())
            {
                QMapIterator<QString, QVariant> i4(i3.peekNext());
                while (i4.hasNext())
                {
                    i4.next();
                    std::cout << i4.key().toStdString() << "=" << i4.value().toString().toStdString() << " ";
                }
                i3.next();
                std::cout << std::endl;
            }
        }
        std::cout << "==========" << std::endl << std::endl;
    }
}

void    Database::_checkBoundValues(QSqlQuery &query)
{
    QMapIterator<QString, QVariant> it(query.boundValues());

    while (it.hasNext())
    {
        it.next();
        if (it.value().isNull())
            query.bindValue(it.key(), "");
    }
}
