#ifndef LIGHTBIRD_IPLUGINS_H
# define LIGHTBIRD_IPLUGINS_H

# include <QSharedPointer>
# include <QString>
# include <QStringList>

# include "IFuture.h"
# include "IMetadata.h"

namespace LightBird
{
    /// @brief Allows plugins to manage other plugins.
    /// The id of the plugin used bellow refers to the name of the directory
    /// where the plugin is stored. It may or not be the real name of the
    /// plugin. Its just a way to identified it.
    class IPlugins
    {
    public:
        virtual ~IPlugins() {}

        /// @brief List the possible states of a plugin.
        enum State
        {
            LOADED,      ///< The plugin id currently loaded and used.
            UNLOADING,   ///< The plugin is unloading. This mean that it is wanting that all its instances are released to be really unloaded.
            UNLOADED,    ///< The plugin is installed but not loaded.
            UNINSTALLED, ///< The plugin is uninstalled, and therefore can't be loaded before beeing installed.
            UNKNOW       ///< The plugin has not been found in the plugins directory.
        };

        /// @brief Allows to load an installed plugin. This operation is
        /// executed in a dedicated thread.
        /// Use getState() to get the current state of the plugin.
        /// @param id : The id of the plugin to load.
        /// @return The future result of the operation. True is returned if the
        /// plugin has been correctly loaded. Thanks to the shared pointer,
        /// plugins don't have to delete themself the IFuture.
        virtual QSharedPointer<LightBird::IFuture<bool> > load(const QString &id) = 0;
        /// @brief Unload a plugin. It may not be unloaded immediatly if the
        /// plugin is being used. A plugin can unload itself. This operation
        /// is executed in a dedicated thread.
        /// Use getState() to get the current state of the plugin.
        /// @param id : The id of the plugin.
        /// @return The future result of the operation. True is returned if the
        /// plugin has been unloaded, or is unloading (because it is still in use).
        /// Thanks to the shared pointer, plugins don't have to delete themself
        /// the IFuture.
        virtual QSharedPointer<LightBird::IFuture<bool> > unload(const QString &id) = 0;
        /// @brief Use this method to install a plugin. This operation is
        /// executed in a dedicated thread.
        /// Use getState() to get the current state of the plugin.
        /// @param id : The Plugin id.
        /// @return The future result of the operation. True is returned if the
        /// plugin has been correctly installed. Thanks to the shared pointer,
        /// plugins don't have to
        /// delete themself the IFuture.
        virtual QSharedPointer<LightBird::IFuture<bool> > install(const QString &id) = 0;
        /// @brief Try to uninstall a plugin. The plugin must be unloaded, or
        /// the uninstallation will fail. This operation is executed in a
        /// dedicated thread.
        /// Use getState() to get the current state of the plugin.
        /// @param id : The Plugin id.
        /// @return The future result of the operation. True is returned if the
        /// plugin has been correctly uninstalled. Thanks to the shared pointer,
        /// plugins don't have to delete themself the IFuture.
        virtual QSharedPointer<LightBird::IFuture<bool> > uninstall(const QString &id) = 0;
        /// @brief Returns the metadata of a plugin. This method can be called
        /// for any plugin accessible by the server, even if it is unloaded or
        /// uninstalled.
        /// @warning It can't be called from the methods defined in IPlugin.
        /// @param id : The Plugin id.
        /// @return The metadata of the plugin.
        virtual LightBird::IMetadata getMetadata(const QString &id) const = 0;
        /// @brief This method is used to get the current state of a plugin.
        /// @warning It can't be called from the methods defined in IPlugin.
        /// @param id : The id of the plugin.
        /// @return The current state of the plugin.
        virtual LightBird::IPlugins::State getState(const QString &id) const = 0;
        /// @brief Returns a list of the id of all the plugins available on the server.
        virtual QStringList     getPlugins() const = 0;
        /// @brief Returns a list of the id of the loaded plugins.
        /// @warning It can't be called from the methods defined in IPlugin.
        virtual QStringList     getLoadedPlugins() const = 0;
        /// @brief Returns a list of the id of the plugins that are installed
        /// but not loaded.
        /// @warning It can't be called from the methods defined in IPlugin.
        virtual QStringList     getUnloadedPlugins() const = 0;
        /// @brief Returns a list of the id of the installed plugins.
        virtual QStringList     getInstalledPlugins() const = 0;
        /// @brief Returns a list of the id of the uninstalled plugins.
        virtual QStringList     getUninstalledPlugins() const = 0;
    };
}

Q_DECLARE_INTERFACE(LightBird::IPlugins, "cc.lightbird.IPlugins")

#endif // LIGHTBIRD_IPLUGINS_H
