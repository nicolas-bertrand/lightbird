#include <QFileInfo>
#include <QString>

#include "ApiConfiguration.h"
#include "Configurations.h"
#include "Defines.h"
#include "Log.h"
#include "Plugins.hpp"
#include "Tools.h"

QMap<QString, Configuration *>  Configurations::instances;
QMutex                          Configurations::lockInstances(QMutex::Recursive);

Configuration       *Configurations::server(const QString &configurationPath, QObject *parent)
{
    QString         path = configurationPath;
    Configuration   *instance;

    if (!Configurations::lockInstances.tryLock(MAXTRYLOCK))
    {
        Log::error("Deadlock", "Configurations", "server");
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
    if (*Configurations::instances[""] == false)
    {
        Log::error("Failed to load the configuration", "Configurations", "instance");
        delete Configurations::instances[""];
        Configurations::instances.remove("");
        Configurations::lockInstances.unlock();
        return (NULL);
    }
    instance = Configurations::instances[""];

    //  Defines the default values
    if (instance->get("pluginsPath").isEmpty())
        instance->set("pluginsPath", DEFAULT_PLUGINS_PATH);
    if (instance->get("QtPluginsPath").isEmpty())
        instance->set("QtPluginsPath", DEFAULT_QT_PLUGINS_PATH);
    if (instance->get("filesPath").isEmpty())
        instance->set("filesPath", DEFAULT_FILES_PATH);
    if (instance->get("temporaryPath").isEmpty())
        instance->set("temporaryPath", DEFAULT_TEMPORARY_PATH);
    if (instance->get("cleanTemporaryPath").isEmpty())
        instance->set("cleanTemporaryPath", DEFAULT_CLEAN_TEMPORARY_PATH);
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
    if (instance->get("permissions/groupInheritance").isEmpty())
        instance->set("permissions/groupInheritance", DEFAULT_PERMISSIONS_GROUPINHERITANCE);
    Configurations::lockInstances.unlock();
    return (instance);
}

Configuration       *Configurations::instance(const QString &configuration, const QString &alternative)
{
    QString         cleaned;
    QString         path;
    Configuration   *instance = NULL;

    if (!Configurations::lockInstances.tryLock(MAXTRYLOCK))
    {
        Log::error("Deadlock", "Configurations", "instance");
        return (NULL);
    }
    // If the path is empty, the server configuration is returned
    if (configuration.isEmpty())
    {
        instance = Configurations::instances.value("");
        Configurations::lockInstances.unlock();
        return (instance);
    }
    // If the server configuration is not loaded
    if (!Configurations::instances.contains(""))
    {
        Log::error("The configuration of the server must be initialized first", Properties("path", configuration).add("alternative", alternative), "Configurations", "instance");
        Configurations::lockInstances.unlock();
        return (NULL);
    }
    cleaned = configuration;
    cleaned.replace('\\', '/');
    // If the file is not defined after the directories, we add the defaut configuration file name
    if (cleaned.at(cleaned.size() - 1) == '/')
        cleaned += DEFAULT_CONFIGURATION_FILE;
    // Otherwise it can be the id of a plugin
    else if (QFileInfo(Configurations::instances[""]->get("pluginsPath") + "/" + cleaned).isDir())
    {
        path = Plugins::checkId(Tools::cleanPath(cleaned));
        // Creates the configuration of the plugin if it doesn't exists
        if (!Configurations::instances.contains(path) && !*(instance = new ApiConfiguration(path)))
        {
            Log::error("Failed to load the configuration of the plugin", Properties("id", path), "Configurations", "instance");
            delete instance;
            Configurations::lockInstances.unlock();
            return (NULL);
        }
    }
    // Cleans the path
    if (path.isEmpty())
        path = Tools::cleanPath(cleaned);
    // Creates the configuration if it doesn't exists
    if (instance == NULL && !Configurations::instances.contains(path) &&
        !*(instance = new Configuration(path, alternative)))
    {
        Log::error("Failed to load the configuration", Properties("path", path).add("alternative", alternative), "Configurations", "instance");
        delete instance;
        Configurations::lockInstances.unlock();
        return (NULL);
    }
    // If a new instance has been created previously
    if (instance)
    {
        // Add it to the configurations
        Configurations::instances[path] = instance;
        // The living thread of the configuration must be the same as the server configuration to have the same parent
        Configurations::instances[path]->moveToThread(Configurations::instances[""]->thread());
        // Set the parent of the configuration (it will be set in the parent thread)
        Configurations::instances[path]->setParent(Configurations::instances[""]->parent());
    }
    // Otherwise get the configuration
    else
        instance = Configurations::instances.value(path);
    Configurations::lockInstances.unlock();
    return (instance);
}
