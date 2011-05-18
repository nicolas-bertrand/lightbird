#ifndef PLUGIN_H
# define PLUGIN_H

# include <QPluginLoader>

# include "Api.h"
# include "Log.h"
# include "Context.h"
# include "Defines.h"

# include "IApi.h"
# include "Extensions.h"
# include "IPlugin.h"
# include "IPlugins.h"

/// @brief Manage a plugin.
class Plugin : public QObject
{
    Q_OBJECT

public:
    /// @brief Initialize the object. The plugin is not loaded until
    /// load() is called.
    /// @param id : The directory where the plugin is stored.
    Plugin(const QString &id, QObject *parent = 0);
    ~Plugin();

    /// @param full : If true, the plugin will be completely loaded, i.e
    /// onLoad is called, the api is created, ... The plugin must be
    /// installed to be fully loaded.
    bool    load(bool full = true);
    /// @param full : If true, onUnload will be called.
    bool    unload(bool full = true);
    bool    install();
    bool    uninstall();
    /// @brief A templated method used to get an instance of a plugin.
    /// Users must call release() after using this method, and must not
    /// use the return pointer after that. Thus, one can be sure that the
    /// plugin will not be unloaded between the calls of get() and release().
    /// @return A pointer to an instance of the plugin of class T, or NULL
    /// if the plugin don't implements this interface.
    template<class T>
    T       *getInstance();
    /// @brief Decrements this->used, which was incremented by get(), and
    /// unload the plugin if necessary.
    bool    release();
    /// @brief Returns the metadata of the plugin.
    LightBird::IMetadata getMetadata() const;
    /// @brief Check if the given context is valid compared to the contexts of the plugin.
    bool    checkContext(const QString &transport, const QStringList &protocols,
                         unsigned short port, const QString &method, const QString &type, bool all);
    /// @brief Returns the current state of the plugin.
    LightBird::IPlugins::State  getState();

private:
    Plugin();
    Plugin(const Plugin &);
    Plugin &operator=(const Plugin &);

    /// @brief Initialize the plugin.
    void                _initialize();
    /// @brief Try to find and load a valid library that implements IPlugin, in
    /// the directory of the plugin.
    /// @return True if the valid library has been found.
    bool                _loadLibrary();
    /// @brief Copy the default configuration of the plugin, if its configuration
    /// is empty in the server configuration.
    void                _loadDefaultConfiguration();
    /// @brief Load the instance of the plugin, the api, the configuration,
    /// the contexts...
    /// @return True on success.
    bool                _load();
    /// @brief Load the api of the plugin.
    void                _loadApi();
    /// @brief Load its contexts using its configuration.
    void                _loadContexts();
    /// @brief Copy the resources of the plugin into its directory.
    void                _loadResources();
    /// @brief Unload the plugin.
    void                _unload();
    /// @brief Creates the configuration of the plugin inside the configuration
    /// of the server if it doesn't exists yet.
    /// @return False if an error occured while creating the configuration.
    bool                _createConfiguration();
    /// @brief Removes the configuration of the plugin in the configuration of
    /// the server, and calls _clean().
    void                _removeConfiguration();
    /// @brief Delete all the instances created when the plugin has been loaded.
    void                _clean();

    QString             id;             ///< The id of the plugin.
    QString             path;           ///< The path to the directory of the plugin.
    QString             libraryName;    ///< The name of the plugin library.
    Configuration       *configuration; ///< The configuration of the plugin.
    Api                 *api;           ///< The LightBird API.
    QPluginLoader       *loader;        ///< A pointer the the loaded plugin.
    LightBird::IPlugin  *instance;      ///< An instance to the plugin.
    QObject             *instanceObject;///< The QObject version of the instance of the plugin.
    int                 used;           ///< A counter of used plugin instances, for a basic garbage collection.
    QList<Context>      contexts;       ///< Contains the contexts of the plugin.
    mutable QReadWriteLock mutex;       ///< Ensure that the class is thread safe.
    LightBird::IPlugins::State state;   ///< The current state of the plugin.

    friend class Extensions;
};

template<class T>
T       *Plugin::getInstance()
{
    T   *instance = NULL;

    if (!this->mutex.tryLockForRead(MAXTRYLOCK))
    {
        Log::error("Deadlock", "Plugin", "getInstance");
        return (NULL);
    }
    if (this->state != LightBird::IPlugins::LOADED)
    {
        this->mutex.unlock();
        return (NULL);
    }
    if ((instance = qobject_cast<T *>(this->instanceObject)) != NULL)
        this->used++;
    this->mutex.unlock();
    return (instance);
}

#endif // PLUGIN_H
