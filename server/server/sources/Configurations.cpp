#include <QFileInfo>

#include "ApiConfiguration.h"
#include "Configurations.h"
#include "Defines.h"
#include "Log.h"
#include "Plugins.hpp"
#include "Server.h"
#include "SmartMutex.h"
#include "Tools.h"

Configurations::Configurations(const QString &configurationPath, QObject *parent) : QObject(parent),
                                                                                    mutex(QMutex::Recursive)
{
    QString         path = configurationPath;
    Configuration   *instance;
    SmartMutex      mutex(this->mutex, "Configurations", "server");

    if (!mutex)
        return ;
    // Creates the configuration of the server
    if (path.isEmpty())
        path = DEFAULT_CONFIGURATION_DIRECTORY + QString("/") + DEFAULT_CONFIGURATION_FILE;
    this->instances[""] = new Configuration(path, DEFAULT_CONFIGURATION_RESSOURCE, parent);
    // If the configuration failed to load
    if (*this->instances[""] == false)
    {
        Log::error("Failed to load the configuration", "Configurations", "instance");
        delete this->instances[""];
        this->instances.remove("");
        return ;
    }
    instance = this->instances[""];
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
    if (instance->get("threadsNumber").isEmpty())
        instance->set("threadsNumber", QString::number(DEFAULT_THREADS_NUMBER));
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
    this->isInitialized();
}

Configuration       *Configurations::getConfiguration(const QString &configuration, const QString &alternative)
{
    QString         cleaned;
    QString         path;
    Configuration   *instance = NULL;
    SmartMutex      mutex(this->mutex, "Configurations", "instance");

    if (!mutex)
        return (NULL);
    // If the path is empty, the server configuration is returned
    if (configuration.isEmpty())
    {
        instance = this->instances.value("");
        return (instance);
    }
    // If the server configuration is not loaded
    if (!this->instances.contains(""))
    {
        Log::error("The configuration of the server must be initialized first", Properties("path", configuration).add("alternative", alternative), "Configurations", "instance");
        return (NULL);
    }
    cleaned = configuration;
    cleaned.replace('\\', '/');
    // If the file is not defined after the directories, we add the defaut configuration file name
    if (cleaned.at(cleaned.size() - 1) == '/')
        cleaned += DEFAULT_CONFIGURATION_FILE;
    // Otherwise it can be the id of a plugin
    else if (QFileInfo(this->instances[""]->get("pluginsPath") + "/" + cleaned).isDir())
    {
        path = Plugins::checkId(Tools::cleanPath(cleaned));
        // Creates the configuration of the plugin if it doesn't exists
        if (!this->instances.contains(path) && !*(instance = new ApiConfiguration(path)))
        {
            Log::error("Failed to load the configuration of the plugin", Properties("id", path), "Configurations", "instance");
            delete instance;
            return (NULL);
        }
    }
    // Cleans the path
    if (path.isEmpty())
        path = Tools::cleanPath(cleaned);
    // Creates the configuration if it doesn't exists
    if (instance == NULL && !this->instances.contains(path) &&
        !*(instance = new Configuration(path, alternative)))
    {
        Log::error("Failed to load the configuration", Properties("path", path).add("alternative", alternative), "Configurations", "instance");
        delete instance;
        return (NULL);
    }
    // If a new instance has been created previously
    if (instance)
    {
        // Add it to the configurations
        this->instances[path] = instance;
        // The living thread of the configuration must be the same as the server configuration to have the same parent
        this->instances[path]->moveToThread(this->instances[""]->thread());
        // Set the parent of the configuration (it will be set in the parent thread)
        this->instances[path]->setParent(this->instances[""]->parent());
    }
    // Otherwise get the configuration
    else
        instance = this->instances.value(path);
    return (instance);
}

Configuration *Configurations::instance(const QString &path, const QString &alternative)
{
    return (Server::instance().getConfiguration(path, alternative));
}
