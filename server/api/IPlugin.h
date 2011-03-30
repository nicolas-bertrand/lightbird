#ifndef IPLUGIN_H
# define IPLUGIN_H

# include "IApi.h"
# include "IConfiguration.h"

namespace Streamit
{
    /// @brief The basic interface that all plugin must implement.
    class IPlugin
    {
    public:
        virtual ~IPlugin() {}

        /**
         * @brief Called when the plugin is loading.
         * @param api : The Streamit APIs. Allows plugins to access to the server APIs. This pointer
         * should be kept by the plugins to be used later. The address of the API is valid during
         * all time where the plugin is loaded, so it can be safely stored.
         * @return True if the plugin have been correclty loaded. If false, it will be unloaded.
         */
        virtual bool    onLoad(Streamit::IApi *api) = 0;
        /**
         * @brief Called when the plugin is unloading. The plugins must stop all its jobs as soon as possible, or
         * it can be killed.
         */
        virtual void    onUnload() = 0;
        /**
         * @brief Called when the plugin is installing.
         * @param api : The Streamit APIs.
         * @return True if the plugin have been correctly installed. If false, it will not be installed.
         */
        virtual bool    onInstall(Streamit::IApi *api) = 0;
        /**
         * @brief Called when the plugin is uninstalling. Plugin must clean all the data that they have
         * modified or created. For example, they have to DROP their own tables in the database.
         * @param api : The Streamit APIs.
         */
        virtual void    onUninstall(Streamit::IApi *api) = 0;
    };
}

Q_DECLARE_INTERFACE (Streamit::IPlugin, "fr.streamit.IPlugin");

#endif // IPLUGIN_H
