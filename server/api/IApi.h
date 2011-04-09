#ifndef IAPI_H
# define IAPI_H

# include <QString>

# include "IConfiguration.h"
# include "IDatabase.h"
# include "IGuis.h"
# include "ILogs.h"
# include "INetwork.h"
# include "IPlugins.h"
# include "IExtensions.h"
# include "ITimers.h"

namespace LightBird
{
    /// @brief This interface includes all the APIs of the server for the plugins.
    class IApi
    {
    public:
        virtual ~IApi() {}

        /// @brief Allows to read and write in the configuration of the current plugin or the server.
        /// @param plugin : Whether the configuration of the plugin or the server will be returned.
        /// By default, the server configuration is returned.
        virtual LightBird::IConfiguration   &configuration(bool plugin = false) = 0;
        /// @brief Returns an instance of a configuration from the file pointed by path.
        /// @param path : The path of the XML configuration file to load.
        /// @param alternative : The path of an alternative configuration file.
        /// It is used in the case where the file defined by path doesn't exists. The configuration
        /// pointed by path will be created using the alternative file.
        /// This is useful to create a XML file from the resources.
        virtual LightBird::IConfiguration   *configuration(const QString &path, const QString &alternative = "") = 0;
        /// @brief Return the database Api.
        virtual LightBird::IDatabase        &database() = 0;
        /// @brief Allows to manage plugins GUIs.
        virtual LightBird::IGuis            &guis() = 0;
        /// @brief The log manager.
        virtual LightBird::ILogs            &log() = 0;
        /// @brief Allows plugins to access to the server network features.
        virtual LightBird::INetwork         &network() = 0;
        /// @brief Allows plugins to manage other plugins.
        virtual LightBird::IPlugins         &plugins() = 0;
        /// @brief Allows to use the extensions.
        virtual LightBird::IExtensions      &extensions() = 0;
        /// @brief Manage the timers of the current plugin.
        virtual LightBird::ITimers          &timers() = 0;
        /// @brief Stop the server. The server will stop as soon as all its current tasks has been done.
        virtual void                        stop() = 0;
        /// @brief Restart the server. The server will stop as soon as all its current tasks has been done.
        /// THIS METHOD DOESN'T WORK YET !!! The server will stop, but not restart.
        virtual void                        restart() = 0;
        /// @brief The id of the plugin. This id refered to the name of the directory
        /// where the plugin is stored. It may or not be the real name of the plugin.
        /// It is a way to identified it uniquely.
        virtual const QString               &getId() = 0;
        /// @brief Returns the path of the current plugin which is the pluginsPath plus its id.
        virtual const QString               &getPluginPath() = 0;
        /// @brief Returns the current version of the server.
        virtual QString                     getServerVersion() = 0;
    };
}

Q_DECLARE_INTERFACE(LightBird::IApi, "cc.lightbird.IApi");

#endif // IAPI_H
