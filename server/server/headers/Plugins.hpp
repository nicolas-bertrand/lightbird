#ifndef PLUGINS_H
# define PLUGINS_H

# include <QPair>
# include <QWaitCondition>
# include <QMutex>

# include "Defines.h"
# include "Plugin.hpp"
# include "Plugins.hpp"
# include "Future.hpp"

# include "INetwork.h"

/**
 * @brief A class class used by the server to manipulate plugins.
 * This class is a thread-safe singleton.
 */
class Plugins : public QThread
{
    Q_OBJECT

public:
    /**
     * @brief Initialize the singleton and launch its thread
     * at the first call. Then returns the singleton instance.
     * @param parent : The parent of the instance of the class.
     * @return The instance to the class Plugins.
     */
    static Plugins          *instance(QObject *parent = 0);
    /// @brief If the Plugins instance is loaded
    static bool             isLoaded();

    /**
     * @brief Contains the event loop that manage all the operations
     * on the plugins.
     */
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
    /**
     * @brief This templated method is used to get all the instances of the plugins
     * that implements the interface T, and whose context matches the parameters.
     * Users must call release() for each plugins stored in the returned map,
     * and must not use its instances after that. Thus, one can be sure that the
     * plugins will not be unloaded between the calls of getInstances() and release().
     * @param transport : The transport protocol of the context.
     * @param protocol : The list of the protocols of the context.
     * @param port : The port of the context.
     * @param method : The method of the context.
     * @param type : The type of the context.
     * @param all : If true, the parameters method and type are used to check the context.
     * @return A map of the plugins that match the filters. The key is the id of the plugin.
     */
    template<class T>
    QMap<QString, T *>      getInstances(Streamit::INetwork::Transports transport, const QStringList &protocols,
                                         unsigned short port, const QString &method = "",
                                         const QString &type = "", bool all = false)
    {
        QMap<QString, T *>  instances;
        T                   *instance = NULL;
        QString             transportText = "TCP";
        Plugin              *plugin;

        if (transport == Streamit::INetwork::UDP)
            transportText = "UDP";
        if (!this->lock.tryLockForRead(MAXTRYLOCK))
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
                if (plugin->checkContext(transportText, protocols, port, method, type, all))
                    instances.insert(it.peekPrevious(), instance);
                else
                    plugin->release();
            }
        }
        this->lock.unlock();
        return (instances);
    }
    /**
     * @brief This method do the same job as getInstances, except that only the first
     * plugin that implements the interface ans maches the context is returned. Users
     * must release it as soon as possible.
     * @return A QPair that contains the name and the instance of the first plugin that
     * matches. The pair is empty if no plugin corresponds.
     * @see getInstances
     */
    template<class T>
    QPair<QString, T *> getInstance(Streamit::INetwork::Transports transport, const QStringList &protocols,
                                    unsigned short port, const QString &method = "",
                                    const QString &type = "", bool all = false)
    {
        T                   *instance;
        QPair<QString, T *> pair;
        QString             transportText = "TCP";
        Plugin              *plugin;

        if (transport == Streamit::INetwork::UDP)
            transportText = "UDP";
        if (!this->lock.tryLockForRead(MAXTRYLOCK))
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
                if (plugin->checkContext(transportText, protocols, port, method, type, all))
                {
                    pair.first = it.peekPrevious();
                    pair.second = instance;
                }
                else
                    plugin->release();
            }
        }
        this->lock.unlock();
        return (pair);
    }
    /**
     * @brief A templated method used to get an instance of a plugin.
     * Users must call release() after using this method, and must not
     * use the return pointer after that. Thus, one can be sure that the
     * plugin will not be unloaded between the calls of get() and release().
     * @param id : The id of the plugin.
     * @return A pointer to an instance of the plugin of class T, or NULL
     * if the plugin don't implements this interface.
     */
    template<class T>
    T       *getInstance(const QString &id)
    {
        T   *instance = NULL;

        if (!this->lock.tryLockForRead(MAXTRYLOCK))
        {
            Log::error("Deadlock", "Plugins", "getInstance");
            return (instance);
        }
        if (this->plugins.contains(id))
            instance = this->plugins.value(id)->getInstance<T>();
        this->lock.unlock();
        return (instance);
    }
    /// @brief A convenance method for getInstances.
    /// @see getInstances
    template<class T>
    QMap<QString, T *>  getInstances(Streamit::INetwork::Transports transport, const QString &protocol,
                                     unsigned short port, const QString &method = "",
                                     const QString &type = "", bool all = false)
    {
        return (this->getInstances<T>(transport, QStringList(protocol), port, method, type, all));
    }
    /// @brief A convenance method for getInstance.
    /// @see getInstance
    template<class T>
    QPair<QString, T *> getInstance(Streamit::INetwork::Transports transport, const QString &protocol,
                                    unsigned short port, const QString &method = "",
                                    const QString &type = "", bool all = false)
    {
        return (this->getInstance<T>(transport, QStringList(protocol), port, method, type, all));
    }
    /**
     * @brief Releases a plugin. Must be call after get().
     * @param id : The id of the plugin.
     * @return True on success, false otherwise.
     * @see get
     */
    bool                    release(const QString &id);
    /// @brief Returns the resources path of a plugin.
    /// @param id : The id of the plugin.
    QString                 getResourcesPath(const QString &id);

    /// @see Streamit::IPlugins::getState
    Streamit::IPlugins::State getState(const QString &id);
    /// @see Streamit::IPlugins::getPlugins
    QStringList             getPlugins();
    /// @see Streamit::IPlugins::getLoadedPlugins
    QStringList             getLoadedPlugins();
    /// @see Streamit::IPlugins::getInstalledPlugins
    QStringList             getInstalledPlugins();
    /// @see Streamit::IPlugins::getUninstalledPlugins
    QStringList             getUninstalledPlugins();

signals:
    void                    loadSignal(const QString &id, Future<bool> *future);
    void                    unloadSignal(const QString &id, Future<bool> *future);
    void                    installSignal(const QString &id, Future<bool> *future);
    void                    uninstallSignal(const QString &id, Future<bool> *future);

    /**
     * @brief This signal is emited when a plugin has been loaded.
     * @param id : The id of the loaded plugin.
     */
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
    /// in order to find the installed and the uninstalled plugins.
    /// @param pluginsPath : The root path of the plugins.
    /// @param path : The path of the current directory checked.
    /// @param plugins : The plugins found.
    /// @param installed : If the method search for installed or uninstalled plugins
    void                    _findPlugins(const QString &pluginsPath, const QString &path, QStringList &plugins, bool installed = true);
    /// @see getState
    Streamit::IPlugins::State _getState(const QString &id);
    /**
     * @brief Ensure that the case of the id in parameter match with the case
     * of a plugin directory name. Otherwise, the case of the id will be modifier
     * to match a directory name.
     * @param id : The id of a plugin. Its case may be wrong.
     * @return The new id, with the correct case.
     */
    QString                 _checkId(const QString &id);

    static Plugins          *_instance;         ///< The only instance of the class.
    QMap<QString, Plugin *> plugins;            ///< The list of loaded plugins.
    QStringList             orderedPlugins;     ///< Contains the id of the loaded plugins, and keep the order of their loading.
    QReadWriteLock          lock;               ///< Used to lock access to the plugins map when a thread use it.
    QWaitCondition          wait;               ///< This condition is awakened when the thread is running for the first time, and when all plugin has been unloaded using unloadAll().
    bool                    awake;              ///< If the wait condition has been called.
    QObject                 *parent;            ///< The parent of the Plugins.
    bool                    unloadAllPlugins;   ///< If true, no more plugins will be loaded. or installed.
    QMap<QString, QString>  resourcesPath;      ///< Stores the resources path of the plugins.
    QMutex                  lockResourcesPath;  ///< Make resourcesPath thread safe.
};

#endif // PLUGINS_H
