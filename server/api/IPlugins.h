#ifndef IPLUGINS_H
# define IPLUGINS_H

# include <QList>
# include <QString>
# include <QSharedPointer>

# include "IFuture.h"

namespace Streamit
{
    /// @brief Allows plugins to manage other plugins.
    /// The id of the plugin used bellow refered to the name
    /// of the directory where the plugin is stored. It may or not be
    /// the real name of the plugin. Its just a way to identified it.
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

        /// @brief Load a plugin. Since a plugin must be installed to be loaded, call this method
        /// on an uninstalled plugin will automatically install it. This operation is executed in a
        /// dedicated thread. Use getState() to get the current state of the plugin.
        /// @param id : The id of the plugin.
        /// @return The future result of the operation. True is returned if the plugin has been correctly
        /// loaded. Thanks to the shared pointer, plugins don't have to delete themself the IFuture.
        virtual QSharedPointer<Streamit::IFuture<bool> > load(const QString &id) = 0;
        /// @brief Unload a plugin. It may not be unloaded immediatly if the plugin is being used.
        /// A plugin can unload itself. This operation is executed in a dedicated thread.
        /// Use getState() to get the current state of the plugin.
        /// @param id : The id of the plugin.
        /// @return The future result of the operation. True is returned if the plugin has been
        /// unloaded, or is unloading (because it is still in use). Thanks to the shared pointer,
        /// plugins don't have to delete themself the IFuture.
        virtual QSharedPointer<Streamit::IFuture<bool> > unload(const QString &id) = 0;
        /// @brief To install a plugin, use this method. This operation is executed in a
        /// dedicated thread. Use getState() to get the current state of the plugin.
        /// @param id : The Plugin id.
        /// @return The future result of the operation. True is returned if the plugin has been correctly
        /// installed. Thanks to the shared pointer, plugins don't have to delete themself the IFuture.
        virtual QSharedPointer<Streamit::IFuture<bool> > install(const QString &id) = 0;
        /// @brief Try to uninstall a plugin. If it is loaded, the plugin will be automatically
        /// unloaded before beeing uninstalled. A plugin can uninstall itself. This operation is
        /// executed in a dedicated thread. Use getState() to get the current state of the plugin.
        /// @param id : The Plugin id.
        /// @return The future result of the operation. True is returned if the plugin has been correctly
        /// uninstalled. Thanks to the shared pointer, plugins don't have to delete themself the IFuture.
        virtual QSharedPointer<Streamit::IFuture<bool> > uninstall(const QString &id) = 0;
        /// @brief This method is used to get the current state of a plugin.
        /// @param id : The id of the plugin.
        /// @return The current state of the plugin.
        virtual Streamit::IPlugins::State getState(const QString &id) = 0;
        /// @return A list of the ids of all the plugins available on the server.
        /// This method return a merged list of the methods getInstalledPlugins()
        /// and getUninstalledPlugins().
        virtual QStringList     getPlugins() = 0;
        /// @return A list of the ids of the loaded plugins.
        virtual QStringList     getLoadedPlugins() = 0;
        /// @return A list of the ids of the installed plugins.
        virtual QStringList     getInstalledPlugins() = 0;
        /// @return A list of the ids of the uninstalled plugins.
        virtual QStringList     getUninstalledPlugins() = 0;
    };
}

Q_DECLARE_INTERFACE(Streamit::IPlugins, "cc.lightbird.IPlugins");

#endif // IPLUGINS_H
