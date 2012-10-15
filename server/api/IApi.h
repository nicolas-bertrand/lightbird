#ifndef IAPI_H
# define IAPI_H

# include <QString>

# include "IConfiguration.h"
# include "IDatabase.h"
# include "IEvents.h"
# include "IExtensions.h"
# include "IGuis.h"
# include "ILogs.h"
# include "INetwork.h"
# include "IPlugins.h"
# include "ISessions.h"
# include "ITimers.h"

/// @brief Contains all the LightBird's API.
namespace LightBird
{
    /// @brief This interface includes all the APIs of the server for the plugins.
    class IApi
    {
    public:
        virtual ~IApi() {}

        /// @brief Allows to read and write on the configuration of the current
        /// plugin or the server.
        /// @param plugin : Whether the configuration of the plugin or the server
        /// will be returned. By default the server configuration is returned.
        virtual LightBird::IConfiguration   &configuration(bool plugin = false) = 0;
        /// @brief Returns an instance of a configuration from the file pointed
        /// by path, or the configuration of a plugin. In the latter case, path
        /// is the id of the plugin.
        /// @param path : The path of the XML configuration file to load,
        /// or the id of a plugin.
        /// @param alternative : The path of an alternative configuration file.
        /// It is used in the case where the file defined by path doesn't exists.
        /// The configuration pointed by path will be created using the alternative
        /// file. This is useful to create a XML file from the resources.
        /// @return NULL if the configuration file can't be loaded.
        virtual LightBird::IConfiguration   *configuration(const QString &path, const QString &alternative = "") = 0;
        /// @brief Return the database Api.
        virtual LightBird::IDatabase        &database() = 0;
        /// @brief Allows to access to the events system.
        virtual LightBird::IEvents          &events() = 0;
        /// @brief Allows to use the extensions.
        virtual LightBird::IExtensions      &extensions() = 0;
        /// @brief Allows to manage plugins GUIs.
        virtual LightBird::IGuis            &guis() = 0;
        /// @brief The log manager.
        virtual LightBird::ILogs            &log() = 0;
        /// @brief Allows plugins to access to the server network features.
        virtual LightBird::INetwork         &network() = 0;
        /// @brief Allows plugins to manage other plugins.
        virtual LightBird::IPlugins         &plugins() = 0;
        /// @brief Returns the session manager.
        virtual LightBird::ISessions        &sessions() = 0;
        /// @brief Manages the timers of the current plugin.
        virtual LightBird::ITimers          &timers() = 0;
        /// @brief Stops the server. The server will stop as soon as all its
        /// current tasks are finished, ie all the plugin has been unloaded.
        virtual void                        stop() = 0;
        /// @brief Restarts the server. The server will stop as soon as all
        /// its current tasks has been done.
        virtual void                        restart() = 0;
        /// @brief The id of the plugin. This id refered to the name of the
        /// directory where the plugin is stored. It may or not be the real
        /// name of the plugin. It is a way to identified it uniquely.
        virtual const QString               &getId() const = 0;
        /// @brief Returns the path of the current plugin which consists of
        /// the pluginsPath plus its id.
        virtual const QString               &getPluginPath() const = 0;
        /// @brief Returns the resources path of the current plugin which consists
        /// of the define PLUGINS_RESOURCES_PATH followed by the plugin id. The
        /// resources path stores all the resources used by the plugin.
        virtual const QString               &getResourcesPath() const = 0;
        /// @brief Returns the current version of the server.
        virtual QString                     getServerVersion() const = 0;
        /// @brief Returns the server language (en for english, fr for france...).
        virtual QString                     getLanguage() const = 0;
    };
}

Q_DECLARE_INTERFACE(LightBird::IApi, "cc.lightbird.IApi")

#endif // IAPI_H
