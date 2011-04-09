#ifndef IPLUGIN_H
# define IPLUGIN_H

# include "IApi.h"
# include "IConfiguration.h"
# include "IMetadata.h"

namespace LightBird
{
    /// @brief The basic interface that all plugin must implement.
    class IPlugin
    {
    public:
        virtual ~IPlugin() {}

        /// @brief Called when the plugin is loading.
        /// @param api : The LightBird APIs. Allows plugins to access to the server APIs. This pointer
        /// should be kept by the plugins to be used later. The address of the API is valid during
        /// all the time where the plugin is loaded, so it can be safely stored.
        /// @return True if the plugin have been correclty loaded. If false, it will be unloaded.
        virtual bool    onLoad(LightBird::IApi *api) = 0;
        /// @brief Called when the plugin is unloading. The plugins must stop all its jobs as soon as possible, or
        /// it can be killed.
        virtual void    onUnload() = 0;
        /// @brief Called when the plugin is installing.
        /// @param api : The LightBird APIs.
        /// @return True if the plugin have been correctly installed. If false, it will not be installed.
        virtual bool    onInstall(LightBird::IApi *api) = 0;
        /// @brief Called when the plugin is uninstalling. Plugins must clean all the data that they have
        /// modified or created. For example, they have to DROP their own tables in the database.
        /// @param api : The LightBird APIs.
        virtual void    onUninstall(LightBird::IApi *api) = 0;
        /// @brief Returns the metadata of the plugin, that can be used by the server, the plugins, or the
        /// users. This method can be called even if the plugin is unloaded or uninstalled, from any thread.
        /// @return The metadata of the plugin.
        virtual void    getMetadata(LightBird::IMetadata &metadata) const = 0;
    };
}

Q_DECLARE_INTERFACE(LightBird::IPlugin, "cc.lightbird.IPlugin");

#endif // IPLUGIN_H
