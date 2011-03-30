#include <QString>
#include <QReadWriteLock>
#include <QDir>

#include "Defines.h"
#include "Configurations.h"
#include "Log.h"

QMap<QString, Configuration *>  Configurations::instances;
QMutex                          Configurations::lockInstances;

Configuration       *Configurations::server(const QString &configurationPath, QObject *parent)
{
    QString         path = configurationPath;
    Configuration   *instance;

    if (!Configurations::lockInstances.tryLock(MAXTRYLOCK))
    {
        Log::error("Deadlock", "Configuration", "server");
        return (NULL);
    }

    // Return the instance if it exists
    if (Configurations::instances.contains(path))
    {
        instance = Configurations::instances[""];
        Configurations::lockInstances.unlock();
        return (instance);
    }

    // Instanciate the configuration of the server
    if (path.isEmpty())
        path = DEFAULT_CONFIGURATION_DIRECTORY + QString("/") + DEFAULT_CONFIGURATION_FILE;
    Configurations::instances[""] = new Configuration(path, DEFAULT_CONFIGURATION_RESSOURCE, parent);

    // If the configuration failed to load
    if (Configurations::instances[""]->isLoaded() == false)
    {
        Log::error("Failed to load the configuration", "Configuration", "instance");
        delete Configurations::instances[""];
        Configurations::instances.remove("");
        return (NULL);
    }
    instance = Configurations::instances[""];

    //  Defines the default values
    if (instance->get("pluginsPath").isEmpty())
        instance->set("pluginsPath", DEFAULT_PLUGINS_PATH);
    if (instance->get("filesPath").isEmpty())
        instance->set("filesPath", DEFAULT_FILES_PATH);
    if (instance->get("temporaryPath").isEmpty())
        instance->set("temporaryPath", DEFAULT_TEMPORARY_PATH);
    if (instance->get("languagesPath").isEmpty())
        instance->set("languagesPath", DEFAULT_LANGUAGES_PATH);
    if (instance->get("database/type").isEmpty())
        instance->set("database/type", DEFAULT_DATABASE_TYPE);
    if (instance->get("database/path").isEmpty())
        instance->set("database/path", DEFAULT_DATABASE_PATH);
    if (instance->get("database/file").isEmpty())
        instance->set("database/file", DEFAULT_DATABASE_FILE);
    if (instance->get("database/resource").isEmpty())
        instance->set("database/resource", DEFAULT_DATABASE_RESOURCE);
    if (instance->get("permissions/activate").isEmpty())
        instance->set("permissions/activate", DEFAULT_PERMISSIONS_ACTIVATE);
    if (instance->get("permissions/default").isEmpty())
        instance->set("permissions/default", DEFAULT_PERMISSIONS_DEFAULT);
    if (instance->get("permissions/inheritance").isEmpty())
        instance->set("permissions/inheritance", DEFAULT_PERMISSIONS_INHERITANCE);
    if (instance->get("permissions/ownerInheritance").isEmpty())
        instance->set("permissions/ownerInheritance", DEFAULT_PERMISSIONS_OWNERINHERITANCE);
    Configurations::lockInstances.unlock();
    return (instance);
}

Configuration       *Configurations::instance(const QString &configuration, const QString &alternative)
{
    QString         cleaned;
    QString         path;
    Configuration   *instance;

    if (!Configurations::lockInstances.tryLock(MAXTRYLOCK))
    {
        Log::error("Deadlock", "Configuration", "server");
        return (NULL);
    }
    // If the server configuration is not loaded
    if (!Configurations::instances.contains(""))
    {
        Log::error("The configuration of the server must be initialized first", Properties("path", configuration).add("alternative", alternative), "Configuration", "instance");
        Configurations::lockInstances.unlock();
        return (NULL);
    }
    // If the path is empty, the server configuration is returned
    if (configuration.isEmpty())
    {
        instance = Configurations::instances[""];
        Configurations::lockInstances.unlock();
        return (instance);
    }
    cleaned = configuration;
    cleaned.replace('\\', '/');
    // If the file is not defined after the directories, we add the defaut configuration file name
    if (cleaned.at(cleaned.size() - 1) == '/')
        cleaned += DEFAULT_CONFIGURATION_FILE;
    // Cleans the path
    path = QDir::cleanPath(cleaned);
    // If the configuration is not already loaded
    if (!Configurations::instances.contains(path))
    {
        // Creates the instance of the configuration
        Configurations::instances[path] = new Configuration(path, alternative);
        // If an error occured, NULL is returned
        if (!Configurations::instances[path]->isLoaded())
        {
            Log::error("Failed to load the configuration", Properties("path", path).add("alternative", alternative), "Configuration", "instance");
            delete Configurations::instances[path];
            Configurations::instances.remove(path);
            Configurations::lockInstances.unlock();
            return (NULL);
        }
        // The living thread of the configuration must be the same than of the server configuration to have the same parent
        Configurations::instances[path]->moveToThread(Configurations::instances[""]->thread());
        // Set the parent of the configuration (it will be set in the parent thread)
        Configurations::instances[path]->setParent(Configurations::instances[""]->parent());
    }
    // Returns the requested configuration
    instance = Configurations::instances[path];
    Configurations::lockInstances.unlock();
    return (instance);
}

bool        Configurations::copy(const QString &sourceName, const QString &destinationName)
{
    QFile   source(sourceName);
    QFile   destination(destinationName);

    if (source.open(QIODevice::ReadOnly) == false)
    {
        Log::error("Cannot open the source file", Properties("source", sourceName).add("destination", destinationName), "Configuration", "copy");
        return (false);
    }
    if (destination.open(QIODevice::WriteOnly | QIODevice::Truncate) == false)
    {
        Log::error("Cannot open the destination file", Properties("source", sourceName).add("destination", destinationName), "Configuration", "copy");
        return (false);
    }
    if (destination.write(source.readAll()) < 0)
    {
        Log::error("Cannot write on the destination file", Properties("source", sourceName).add("destination", destinationName), "Configuration", "copy");
        return (false);
    }
    source.close();
    destination.close();
    return (true);
}
