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

/// @brief A class class used by the server to manipulate plugins.
/// This class is a thread-safe singleton.
class Plugins : public QThread
{
    Q_OBJECT

public:
    /// @brief Initialize the singleton and launch its thread
    /// at the first call. Then returns the singleton instance.
    /// @param parent : The parent of the instance of the class.
    /// @return The instance to the class Plugins.
    static Plugins          *instance(QObject *parent = 0);
    /// @brief If the Plugins instance is loaded
    static bool             isLoaded();

    /// @brief Contains the event loop that manage all the operations
    /// on the plugins.
    void                    run();

    /// @param id : The id of the plugin.
    Future<bool>            load(const QString &id);
    /// @param id : The id of the plugin.
    Future<bool>            unload(const QString &id);
    /// @param id : The id of the plugin.
    Future<bool>            install(const QString &id);
    /// @param id : The id of the plugin.
    Future<bool>            uninstall(const QString &id);
    /// @brief Unload all the plugins synchronously. This function block until all
    /// plugins has been unloaded, and it is not possible to load a plugin after that.
    void                    unloadAll();
    /// @brief This templated method is used to get all the instances of the plugins
    /// that implements the interface T, and whose context matches the parameters.
    /// Users must call release() for each plugins stored in the returned map,
    /// and must not use its instances after that. Thus, one can be sure that the
    /// plugins will not be unloaded between the calls of getInstances() and release().
    /// @param transport : The transport protocol of the context.
    /// @param protocol : The list of the protocols of the context.
    /// @param port : The port of the context.
    /// @param method : The method of the context.
    /// @param type : The type of the context.
    /// @param all : If true, the parameters method and type are used to check the context.
    /// @return A map of the plugins that match the filters. The key is the id of the plugin.
    template<class T>
    QMap<QString, T *>  getInstances(LightBird::IClient::Mode mode, LightBird::INetwork::Transport transport,
                                     const QStringList &protocols, unsigned short port, const QString &method = "",
                                     const QString &type = "", bool all = false);
    /// @brief This method do the same job as getInstances, except that only the first
    /// plugin that implements the interface ans maches the context is returned. Users
    /// must release it as soon as possible.
    /// @return A QPair that contains the name and the instance of the first plugin that
    /// matches. The pair is empty if no plugin corresponds.
    /// @see getInstances
    template<class T>
    QPair<QString, T *> getInstance(LightBird::IClient::Mode mode, LightBird::INetwork::Transport transport,
                                    const QStringList &protocols, unsigned short port, const QString &method = "",
                                    const QString &type = "", bool all = false);
    /// @brief A templated method used to get an instance of a plugin.
    /// Users must call release() after using this method, and must not
    /// use the return pointer after that. Thus, one can be sure that the
    /// plugin will not be unloaded between the calls of get() and release().
    /// @param id : The id of the plugin.
    /// @return A pointer to an instance of the plugin of class T, or NULL
    /// if the plugin don't implements this interface.
    template<class T>
    T       *getInstance(const QString &id);
    /// @brief A convenance method for getInstances.
    /// @see getInstances
    template<class T>
    QMap<QString, T *>  getInstances(LightBird::IClient::Mode mode, LightBird::INetwork::Transport transport,
                                     const QString &protocol, unsigned short port, const QString &method = "",
                                     const QString &type = "", bool all = false);
    /// @brief A convenance method for getInstance.
    /// @see getInstance
    template<class T>
    QPair<QString, T *> getInstance(LightBird::IClient::Mode mode, LightBird::INetwork::Transport transport,
                                    const QString &protocol, unsigned short port, const QString &method = "",
                                    const QString &type = "", bool all = false);
    /// @brief Releases a plugin. Must be call after get().
    /// @param id : The id of the plugin.
    /// @return True on success, false otherwise.
    /// @see get
    bool                    release(const QString &id);
    /// @brief Returns the metadata of a plugin.
    /// @param id : The plugin id.
    LightBird::IMetadata    getMetadata(const QString &id) const;
    /// @brief Returns the resources path of a plugin, which is a directory that
    /// contains all the resources it uses. It is composed of PLUGINS_RESOURCES_PATH
    /// followed by the plugin id.
    /// @param id : The id of the plugin.
    static QString          getResourcesPath(const QString &id);
    /// @brief Ensure that the case of the id in parameter match with the case
    /// of a plugin directory name. Otherwise, the case of the id will be modified
    /// to match a directory name.
    /// @param id : The id of a plugin. Its case may be wrong.
    /// @return The new id, with the correct case.
    static QString          checkId(const QString &id);
    /// @brief Returns true if the plugin is installed, i.e if its configuration
    /// is stored in the configuration of the server.
    /// @param id : The id of the plugin.
    static bool             isInstalled(const QString &id);

    /// @see LightBird::IPlugins::getState
    LightBird::IPlugins::State getState(const QString &id);
    /// @see LightBird::IPlugins::getPlugins
    QStringList             getPlugins();
    /// @see LightBird::IPlugins::getLoadedPlugins
    QStringList             getLoadedPlugins();
    /// @see LightBird::IPlugins::getUnloadedPlugins
    QStringList             getUnloadedPlugins();
    /// @see LightBird::IPlugins::getInstalledPlugins
    QStringList             getInstalledPlugins();
    /// @see LightBird::IPlugins::getUninstalledPlugins
    QStringList             getUninstalledPlugins();

signals:
    void                    loadSignal(const QString &id, Future<bool> *future);
    void                    unloadSignal(const QString &id, Future<bool> *future);
    void                    installSignal(const QString &id, Future<bool> *future);
    void                    uninstallSignal(const QString &id, Future<bool> *future);

    /// @brief This signal is emited when a plugin has been loaded.
    /// @param id : The id of the loaded plugin.
    void                    loaded(QString id);


private:
    Plugins(QObject *parent = 0);
    ~Plugins();
    Plugins(const Plugins &);
    Plugins                 *operator=(const Plugins &);

private slots:
    void                    _load(const QString &id, Future<bool> *future);
    void                    _unload(const QString &id, Future<bool> *future);
    void                    _install(const QString &id, Future<bool> *future);
    void                    _uninstall(const QString &id, Future<bool> *future);

private:
    /// @brief This method run recursively through all the directories in the plugins path,
    /// in order to find all the plugins accessible by the server.
    /// @param pluginsPath : The root path of the plugins.
    /// @param path : The path of the current directory checked.
    /// @param plugins : The plugins found.
    void                    _findPlugins(const QString &pluginsPath, const QString &path, QStringList &plugins);
    /// @see getState
    LightBird::IPlugins::State _getState(const QString &id);

    static Plugins          *_instance;         ///< The only instance of the class.
    QMap<QString, Plugin *> plugins;            ///< The list of loaded plugins.
    QStringList             orderedPlugins;     ///< Contains the id of the loaded plugins, and keep the order of their loading.
    mutable QReadWriteLock  mutex;              ///< Used to secure the access to the plugins map when a thread use it.
    QWaitCondition          wait;               ///< This condition is awakened when the thread is running for the first time, and when all plugin has been unloaded using unloadAll().
    bool                    awake;              ///< If the wait condition has been called.
    QObject                 *parent;            ///< The parent of the Plugins.
    bool                    unloadAllPlugins;   ///< If true, no more plugins will be loaded. or installed.
};

template<class T>
QMap<QString, T *>  Plugins::getInstances(LightBird::IClient::Mode mode, LightBird::INetwork::Transport transport,
                                          const QStringList &protocols, unsigned short port,
                                          const QString &method, const QString &type, bool all)
{
    QMap<QString, T *>  instances;
    T                   *instance = NULL;
    QString             modeText = "server";
    QString             transportText = "TCP";
    Plugin              *plugin;

    if (mode == LightBird::IClient::CLIENT)
        modeText = "client";
    if (transport == LightBird::INetwork::UDP)
        transportText = "UDP";
    if (!this->mutex.tryLockForRead(MAXTRYLOCK))
    {
        Log::error("Deadlock", "Plugins", "getInstances");
        return (instances);
    }
    QStringListIterator it(this->orderedPlugins);
    while (it.hasNext())
    {
        plugin = this->plugins[it.next()];
        if ((instance = plugin->getInstance<T>()))
        {
            if (plugin->checkContext(modeText, transportText, protocols, port, method, type, all))
                instances.insert(it.peekPrevious(), instance);
            else
                plugin->release();
        }
    }
    this->mutex.unlock();
    return (instances);
}

template<class T>
QPair<QString, T *> Plugins::getInstance(LightBird::IClient::Mode mode, LightBird::INetwork::Transport transport,
                                         const QStringList &protocols, unsigned short port,
                                         const QString &method, const QString &type, bool all)
{
    T                   *instance;
    QPair<QString, T *> pair;
    QString             modeText = "server";
    QString             transportText = "TCP";
    Plugin              *plugin;

    if (mode == LightBird::IClient::CLIENT)
        modeText = "client";
    if (transport == LightBird::INetwork::UDP)
        transportText = "UDP";
    if (!this->mutex.tryLockForRead(MAXTRYLOCK))
    {
        Log::error("Deadlock", "Plugins", "getInstance");
        return (pair);
    }
    QStringListIterator it(this->orderedPlugins);
    while (it.hasNext() && !pair.second)
    {
        plugin = this->plugins[it.next()];
        if ((instance = plugin->getInstance<T>()))
        {
            if (plugin->checkContext(modeText, transportText, protocols, port, method, type, all))
            {
                pair.first = it.peekPrevious();
                pair.second = instance;
            }
            else
                plugin->release();
        }
    }
    this->mutex.unlock();
    return (pair);
}

template<class T>
T       *Plugins::getInstance(const QString &id)
{
    T   *instance = NULL;

    if (!this->mutex.tryLockForRead(MAXTRYLOCK))
    {
        Log::error("Deadlock", "Plugins", "getInstance");
        return (instance);
    }
    if (this->plugins.contains(id))
        instance = this->plugins.value(id)->getInstance<T>();
    this->mutex.unlock();
    return (instance);
}

template<class T>
QMap<QString, T *>  Plugins::getInstances(LightBird::IClient::Mode mode, LightBird::INetwork::Transport transport,
                                          const QString &protocol, unsigned short port,
                                          const QString &method, const QString &type, bool all)
{
    return (this->getInstances<T>(mode, transport, QStringList(protocol), port, method, type, all));
}

template<class T>
QPair<QString, T *> Plugins::getInstance(LightBird::IClient::Mode mode, LightBird::INetwork::Transport transport,
                                         const QString &protocol, unsigned short port,
                                         const QString &method, const QString &type, bool all)
{
    return (this->getInstance<T>(mode, transport, QStringList(protocol), port, method, type, all));
}

#endif // PLUGINS_H
