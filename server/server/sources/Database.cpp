#include <iostream>
#include <QDir>
#include <QSqlError>
#include <QSqlRecord>

#include "ApiDatabase.h"
#include "Configurations.h"
#include "Database.h"
#include "Defines.h"
#include "Library.h"
#include "LightBird.h"
#include "Log.h"
#include "Plugins.hpp"
#include "SmartMutex.h"
#include "Server.h"

Database::Database(QObject *parent) : QObject(parent)
{
    // Stores the names of the tables of the database
    this->tablesNames << "accounts" << "accounts_groups" << "accounts_informations" << "collections"
                      << "directories" << "events" << "events_informations" << "files" << "files_collections"
                      << "files_informations" << "groups" << "limits" << "permissions" << "tags";
    // Connects the server to the database
    if (this->_connection())
        this->isInitialized();
    else
        Log::error("Connection to the database failed", "Database", "Database");
    // Allows the library to use the database
    LightBird::Library::setDatabase(new ApiDatabase());
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

bool            Database::query(QSqlQuery &query, QVector<QVariantMap> &result)
{
    bool        error;
    int         count;
    int         r;
    QVariantMap row;

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

bool        Database::query(QSqlQuery &query, QVariantMap &result)
{
    bool    error;
    int     count;

    this->_checkBoundValues(query);
    // Execute the query
    if ((error = query.exec()) == false)
        Log::error("An SQL error occured", Properties("query", query.lastQuery()).add("error", query.lastError().text()).add(query.boundValues()), "Database", "query");
    // Put the result in the reference
    else
    {
        result.clear();
        count = query.record().count();
        if (query.next())
            for (int r = 0; r < count; ++r)
                result.insert(query.record().fieldName(r), query.value(r));
        else
            error = false;
        // If the log is in trace, all the informations on the query are saved
        if (Log::instance()->isTrace())
            Log::trace("SQL query executed", Properties("query", query.lastQuery()).add(query.boundValues()).add("rows", result.size()), "Database", "query");
    }
    return (error);
}

LightBird::Table            *Database::getTable(LightBird::Table::Id table, const QString &id)
{
    QStringList             tables;
    QString                 name;
    QSqlQuery               query;
    QVector<QVariantMap>    result;

    // If the table is unknow, tries to find it using the id of the row
    if (!id.isEmpty() && (table == LightBird::Table::Unknow || table == LightBird::Table::Accessor || table == LightBird::Table::Object))
    {
        // Defines the names of the tables where the existance of the id will be checked
        if (table == LightBird::Table::Accessor)
            tables << "accounts" << "groups";
        else if (table == LightBird::Table::Object)
            tables << "files" << "directories" << "collections";
        else
            tables = this->tablesNames;
        // Search the id
        QStringListIterator it(tables);
        while (it.hasNext() && name.isEmpty())
        {
            query.prepare(this->getQuery("Table", "exists").replace(":table", it.peekNext()));
            query.bindValue(":id", id);
            if (!this->query(query, result))
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
    if (table == LightBird::Table::Accounts || name == "accounts")
        return (new LightBird::TableAccounts(id));
    if (table == LightBird::Table::Collections || name == "collections")
        return (new LightBird::TableCollections(id));
    if (table == LightBird::Table::Directories || name == "directories")
        return (new LightBird::TableDirectories(id));
    if (table == LightBird::Table::Events || name == "events")
        return (new LightBird::TableEvents(id));
    if (table == LightBird::Table::Files || name == "files")
        return (new LightBird::TableFiles(id));
    if (table == LightBird::Table::Groups || name == "groups")
        return (new LightBird::TableGroups(id));
    if (table == LightBird::Table::Limits || name == "limits")
        return (new LightBird::TableLimits(id));
    if (table == LightBird::Table::Permissions || name == "permissions")
        return (new LightBird::TablePermissions(id));
    if (table == LightBird::Table::Tags || name == "tags")
        return (new LightBird::TableTags(id));
    return (NULL);
}

QString         Database::getQuery(const QString &group, const QString &name, const QString &id)
{
    SmartMutex  mutex(this->mutex, "Database", "getQuery");
    QString     result;

    if (!mutex)
        return ("");
    if (!this->queries.contains(id))
        if (!this->_loadQueries(id))
            return ("");
    if (!(result = this->queries[id].firstChildElement().firstChildElement(group).firstChildElement(name).text()).isEmpty())
        return (result);
    Log::warning("Query not found", Properties("group", group).add("name", name).add("id", id), "Database", "getQuery");
    return ("");
}

bool                            Database::updates(LightBird::IDatabase::Updates &updates, const QDateTime &d, const QStringList &t)
{
    LightBird::IDatabase::State state;
    QString                     date;
    QSqlQuery                   query;
    QVector<QVariantMap>        result;
    QStringList                 tables;
    QString                     table;
    int                         i;
    int                         s;

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
    e = Configurations::instance()->readDom().firstChildElement("database").firstChildElement("pragmas").firstChildElement("pragma");
    while (!e.isNull())
    {
        QSqlQuery query;
        query.prepare(e.text());
        this->query(query);
        e = e.nextSiblingElement("pragma");
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
        if (LightBird::copy(databaseResource, file) == false)
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
    // In the resources of the plugin
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

    // Open the queries file
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
        this->queries.remove(id);
        return (false);
    }
    Log::debug("Queries loaded", Properties("file", file.fileName()), "Database", "_loadQueries");
    file.close();
    return (true);
}

void    Database::_checkBoundValues(QSqlQuery &query) const
{
    QMapIterator<QString, QVariant> it(query.boundValues());

    while (it.hasNext())
    {
        it.next();
        if (it.value().isNull())
            query.bindValue(it.key(), "");
    }
}

void    Database::_displayUpdates(LightBird::IDatabase::Updates &updates) const
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

Database    *Database::instance()
{
    return (Server::instance().getDatabase());
}
