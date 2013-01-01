#ifndef PLUGINS_H
# define PLUGINS_H

# include <QMutex>
# include <QObject>
# include <QPair>
# include <QString>
# include <QStringList>
# include <QThread>
# include <QWaitCondition>

# include "IClient.h"
# include "INetwork.h"

# include "Defines.h"
# include "Plugin.hpp"
# include "Future.hpp"

/// @brief Manages the plugins of the server.
class Plugins : public QThread
{
    Q_OBJECT

public:
    Plugins();
    ~Plugins();

    /// @param id : The id of the plugin.
    Future<bool>            load(const QString &id);
    /// @param id : The id of the plugin.
    Future<bool>            unload(const QString &id);
    /// @param id : The id of the plugin.
    Future<bool>            install(const QString &id);
    /// @param id : The id of the plugin.
    Future<bool>            uninstall(const QString &id);
    /// @brief Unloads all the plugins synchronously. This function block until all
    /// plugins has been unloaded, and it is not possible to load a plugin after that.
    void                    shutdown();
    /// @brief A templated method used to get an instance of a plugin.
    /// Users must call release() after using this method, and must not
    /// use the return pointer afterward. Thus, one can be sure that the
    /// plugin will not be unloaded between the calls of get() and release().
    /// @param id : The id of the plugin.
    /// @return A pointer to an instance of the plugin of class T, or NULL
    /// if the plugin does not implement this interface.
    /// @see release
    template<class T>
    T                       *getInstance(const QString &id);
    /// @brief Does the same job as getInstance, except that the instance of
    /// the first context that implements the interface and matches the
    /// validator is returned.
    /// @param id : The id of the plugin from which the contexts are checked.
    /// If empty, all the plugin are checked.
    /// @return A QPair that contains the name and the instance of the first
    /// plugin that matches. The pair is empty if no plugin corresponds.
    /// @see getInstance
    /// @see release
    template<class T>
    QPair<QString, T *>     getInstance(const Context::Validator &validator, const QString &id = QString());
    /// @brief A templated method used to get the instances of all the plugins
    /// that implement an interface. Users must call release() for each plugins
    /// stored in the returned map, and must not use its instances after that.
    /// Thus, one can be sure that the plugins will not be unloaded between the
    /// calls of getInstances() and release().
    /// @return A map of the plugins that implement the interface. The keys
    /// are the id of the plugins.
    /// @see release
    template<class T>
    QMap<QString, T *>      getInstances();
    /// @brief Does the same job as getInstances, except that the instances
    /// of all the contexts that implement the interface and matches the
    /// validator are returned.
    /// @return A map of all the contexts of the plugins that implement the
    /// class T and matches the validator. The keys of the map are the id of
    /// the plugins.
    /// @see getInstances
    /// @see release
    template<class T>
    QMultiMap<QString, T *> getInstances(const Context::Validator &validator);
    /// @brief Returns the metadata of a plugin.
    /// @param id : The plugin id.
    LightBird::IMetadata    getMetadata(const QString &id) const;
    /// @brief Returns the resources path of a plugin, which is a directory that
    /// contains all the resources it uses. It is composed of PLUGINS_RESOURCES_PATH
    /// followed by the plugin id.
    /// @param id : The id of the plugin.
    static QString          getResourcesPath(const QString &id);
    /// @brief Ensures that the case of the id in parameter match with the case
    /// of a plugin directory name. Otherwise, the case of the id will be modified
    /// to match a directory name.
    /// @param id : The id of a plugin. Its case may be wrong.
    /// @return The new id, with the correct case.
    static QString          checkId(const QString &id);
    /// @brief Returns true if the plugin is installed, i.e if its configuration
    /// is stored in the configuration of the server.
    /// @param id : The id of the plugin.
    static bool             isInstalled(const QString &id);
    /// @brief Returns the list of the possible extensions that can have a plugin
    /// library, preceded by ".*".
    static QStringList      getLibraryExtensions();
    /// @brief Returns the instance of this class created by the Server.
    static Plugins          *instance();

    /// @see LightBird::IPlugins::getState
    LightBird::IPlugins::State getState(const QString &id) const;
    /// @see LightBird::IPlugins::getPlugins
    QStringList             getPlugins() const;
    /// @see LightBird::IPlugins::getLoadedPlugins
    QStringList             getLoadedPlugins() const;
    /// @see LightBird::IPlugins::getUnloadedPlugins
    QStringList             getUnloadedPlugins() const;
    /// @see LightBird::IPlugins::getInstalledPlugins
    QStringList             getInstalledPlugins() const;
    /// @see LightBird::IPlugins::getUninstalledPlugins
    QStringList             getUninstalledPlugins() const;

public slots:
    /// @brief Releases a plugin. Must be call after get().
    /// @param id : The id of the plugin.
    /// @return True on success, false otherwise.
    /// @see get
    bool                    release(const QString &id);

signals:
    void                    loadSignal(const QString &id, Future<bool> *future);
    void                    unloadSignal(const QString &id, Future<bool> *future);
    void                    installSignal(const QString &id, Future<bool> *future);
    void                    uninstallSignal(const QString &id, Future<bool> *future);
    /// @brief This signal is emitted when a plugin has been loaded.
    /// @param id : The id of the loaded plugin.
    void                    loaded(QString id);

private slots:
    void                    _load(const QString &id, Future<bool> *future);
    void                    _unload(const QString &id, Future<bool> *future);
    void                    _install(const QString &id, Future<bool> *future);
    void                    _uninstall(const QString &id, Future<bool> *future);

private:
    Plugins(const Plugins &);
    Plugins &operator=(const Plugins &);

    /// @brief Contains the event loop that manages all the operations on the plugins.
    void                    run();
    /// @brief This method run recursively through all the directories in the plugins path,
    /// in order to find all the plugins accessible by the server.
    /// @param pluginsPath : The root path of the plugins.
    /// @param path : The path of the current directory checked.
    /// @param plugins : The plugins found.
    void                    _findPlugins(const QString &pluginsPath, const QString &path, QStringList &plugins) const;
    /// @see getState
    LightBird::IPlugins::State _getState(const QString &id) const;

    QMap<QString, Plugin *> plugins;           ///< The list of loaded plugins.
    QStringList             orderedPlugins;    ///< Contains the id of the loaded plugins, and keep the order of their loading.
    QStringList             libraryExtensions; ///< The possible extensions of a plugin library
    QWaitCondition          wait;              ///< This condition is awakened when the thread is running for the first time, and when all plugin has been unloaded using unloadAll().
    bool                    awake;             ///< If the wait condition has been called.
    bool                    unloadAllPlugins;  ///< If true, no more plugins will be loaded. or installed.
    mutable QReadWriteLock  mutex;             ///< Used to secure the access to the plugins map when a thread use it.
};

template<class T>
T   *Plugins::getInstance(const QString &id)
{
    T       *instance = NULL;
    Mutex   mutex(this->mutex, Mutex::READ, "Plugins", "getInstance");

    if (!mutex)
        return (instance);
    if (this->plugins.contains(id))
        instance = this->plugins.value(id)->getInstance<T>();
    return (instance);
}

template<class T>
QPair<QString, T *> Plugins::getInstance(const Context::Validator &validator, const QString &id)
{
    T                   *instance;
    QPair<QString, T *> pair;
    Mutex               mutex(this->mutex, Mutex::READ, "Plugins", "getInstance");

    if (!mutex)
        return (pair);
    // Checks all the plugins
    if (id.isEmpty())
    {
        QStringListIterator it(this->orderedPlugins);
        while (it.hasNext() && !pair.second)
            if ((instance = this->plugins[it.next()]->getInstance<T>(validator)))
                pair = QPair<QString, T *>(it.peekPrevious(), instance);
    }
    // Checks the given plugin
    else if ((instance = this->plugins[id]->getInstance<T>(validator)))
        pair = QPair<QString, T *>(id, instance);
    return (pair);
}

template<class T>
QMap<QString, T *>  Plugins::getInstances()
{
    QMap<QString, T *> instances;
    T                  *instance = NULL;
    Plugin             *plugin;
    Mutex              mutex(this->mutex, Mutex::READ, "Plugins", "getInstances");

    if (!mutex)
        return (instances);
    QStringListIterator it(this->orderedPlugins);
    while (it.hasNext())
    {
        plugin = this->plugins[it.next()];
        if ((instance = plugin->getInstance<T>()))
            instances.insert(it.peekPrevious(), instance);
    }
    return (instances);
}

template<class T>
QMultiMap<QString, T *> Plugins::getInstances(const Context::Validator &validator)
{
    QMultiMap<QString, T *> instances;
    Plugin                  *plugin;
    Mutex                   mutex(this->mutex, Mutex::READ, "Plugins", "getInstances");

    if (!mutex)
        return (instances);
    QStringListIterator it(this->orderedPlugins);
    while (it.hasNext())
    {
        plugin = this->plugins[it.next()];
        instances += plugin->getInstances<T>(validator);
    }
    return (instances);
}

#endif // PLUGINS_H
