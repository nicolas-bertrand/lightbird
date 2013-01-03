#ifndef PLUGIN_H
# define PLUGIN_H

# include <QList>
# include <QObject>
# include <QPluginLoader>
# include <QString>
# include <QStringList>
# include <QReadWriteLock>

# include "IApi.h"
# include "IContexts.h"
# include "IPlugin.h"
# include "IPlugins.h"

# include "Api.h"
# include "Context.h"
# include "Defines.h"
# include "Extensions.h"
# include "Log.h"
# include "Mutex.h"

/// @brief Manages a plugin.
class Plugin : public QObject,
               public LightBird::IContexts
{
    Q_OBJECT
    Q_INTERFACES(LightBird::IContexts)

public:
    /// @brief Initializes the object. The plugin is not loaded until
    /// load() is called.
    /// @param id : The directory where the plugin is stored.
    Plugin(const QString &id, QObject *parent = NULL);
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
    /// use the return pointer afterward. Thus, one can be sure that the
    /// plugin will not be unloaded between the calls of get() and release().
    /// @return A pointer to an instance of the plugin of class T, or NULL
    /// if the plugin does not implement this interface.
    /// @see release
    template<class T>
    T       *getInstance();
    /// @brief Does the same job as getInstance, except that the instance of
    /// the first context that implements the interface and matches the
    /// validator is returned.
    /// @return A pointer to an instance of the plugin of class T, or NULL
    /// if the plugin does not implement this interface or no context matches
    /// the validator.
    /// @see getInstance
    /// @see release
    template<class T>
    T       *getInstance(const Context::Validator &validator);
    /// @brief Does the same job as getInstance, except that the instances of
    /// all the contexts that implement the interface and matches the
    /// validator are returned.
    /// @return A map of all the contexts of the plugin that implement the
    /// class T and matches the validator. The keys of the map are the id of
    /// the plugin (for convenience).
    /// @see getInstance
    /// @see release
    template<class T>
    QMultiMap<QString, T *> getInstances(const Context::Validator &validator);
    /// @brief Decrements this->used, which was incremented by get(), and
    /// unload the plugin if necessary.
    bool    release();
    /// @brief Returns the metadata of the plugin.
    LightBird::IMetadata getMetadata() const;
    /// @brief Returns the current state of the plugin.
    LightBird::IPlugins::State  getState() const;

    // LightBird::IContexts
    /// @see LightBird::IContexts::declareInstance
    bool    declareInstance(QString name, QObject *instance);
    /// @see LightBird::IContexts::get
    QMultiMap<QString, LightBird::IContext *> get(QStringList names = QStringList());
    /// @see LightBird::IContexts::add
    LightBird::IContext *add(const QString &name);
    /// @see LightBird::IContexts::clone
    LightBird::IContext *clone(LightBird::IContext *context, const QString &newName);
    /// @see LightBird::IContexts::remove
    void    remove(LightBird::IContext *context);

private:
    Plugin();
    Plugin(const Plugin &);
    Plugin &operator=(const Plugin &);

    /// @brief Initializes the plugin.
    void                _initialize();
    /// @brief Tries to find and load a valid library that implements IPlugin, in
    /// the directory of the plugin.
    /// @return True if the valid library has been found.
    bool                _loadLibrary();
    /// @brief Copies the default configuration of the plugin, if its configuration
    /// is empty in the server configuration.
    void                _loadDefaultConfiguration();
    /// @brief Loads the instance of the plugin, the api, the configuration,
    /// the contexts...
    /// @return True on success.
    bool                _load();
    /// @brief Load the api of the plugin.
    void                _loadApi();
    /// @brief Copies the resources of the plugin into its directory.
    void                _loadResources();
    /// @brief Copies all the resources of the plugin into its directory.
    void                _copyAllResources(const QString &resourcesPath, const QString &destDir, QString currentDir = QString());
    /// @brief Loads its contexts using the configuration.
    /// The plugin must be loaded before loading the contexts.
    void                _loadContexts();
    /// @brief Unloads the plugin.
    void                _unload();
    /// @brief Creates the configuration of the plugin inside the configuration
    /// of the server if it doesn't exists yet.
    /// @return False if an error occured while creating the configuration.
    bool                _createConfiguration();
    /// @brief Removes the configuration of the plugin in the configuration of
    /// the server, and calls _clean().
    void                _removeConfiguration();
    /// @brief Returns true if one of the contexts matches the validator.
    bool                _checkContexts(const QList<Context> &contexts, const Context::Validator &validator) const;
    /// @brief Deletes all the instances created when the plugin has been loaded.
    void                _clean();

    QString            id;              ///< The id of the plugin.
    QString            path;            ///< The path to the directory of the plugin.
    QString            libraryName;     ///< The name of the plugin library.
    Configuration      *configuration;  ///< The configuration of the plugin.
    Api                *api;            ///< The LightBird API.
    QPluginLoader      *loader;         ///< A pointer the the loaded plugin.
    LightBird::IPlugin *instance;       ///< An instance to the plugin.
    QObject            *instanceObject; ///< The QObject version of the instance of the plugin.
    int                used;            ///< A counter of used plugin instances, for a basic garbage collection.
    mutable QReadWriteLock mutex;       ///< Ensures that the class is thread safe.
    LightBird::IPlugins::State state;   ///< The current state of the plugin.
    QMap<QString, QObject *> contextsDeclared; ///< The instances of the contexts declared, sorted by name.
    QMap<QString, QList<Context> > contexts; ///< The the contexts of the plugin, sorted by name.

    // Allows Extensions to increment this->used
    friend class Extensions;
};

template<class T>
T   *Plugin::getInstance()
{
    T       *instance = NULL;
    Mutex   mutex(this->mutex, "Plugin", "getInstance");

    if (!mutex || this->state != LightBird::IPlugins::LOADED)
        return (NULL);
    if ((instance = qobject_cast<T *>(this->instanceObject)))
        this->used++;
    return (instance);
}

template<class T>
T   *Plugin::getInstance(const Context::Validator &validator)
{
    T       *instance = NULL;
    Mutex   mutex(this->mutex, "Plugin", "getInstance");

    if (!mutex || this->state != LightBird::IPlugins::LOADED)
        return (NULL);
    // Checks the default contexts first
    if (!(instance = qobject_cast<T *>(this->instanceObject)) || !this->_checkContexts(this->contexts.value(""), validator))
        instance = NULL;
    // Then the other contexts
    QMapIterator<QString, QList<Context> > it(this->contexts);
    while (!instance && it.hasNext())
        if (!it.next().key().isEmpty())
            if (!(instance = qobject_cast<T *>(it.value().first().getInstance())) || !this->_checkContexts(it.value(), validator))
                instance = NULL;
    if (instance)
        this->used++;
    return (instance);
}

template<class T>
QMultiMap<QString, T *> Plugin::getInstances(const Context::Validator &validator)
{
    QMultiMap<QString, T *> instances;
    T                       *instance = NULL;
    Mutex                   mutex(this->mutex, "Plugin", "getInstances");

    if (!mutex || this->state != LightBird::IPlugins::LOADED)
        return (instances);
    // Checks the default context first
    if ((instance = qobject_cast<T *>(instanceObject)) && this->_checkContexts(this->contexts.value(""), validator))
        instances.insert(this->id, instance);
    // Then the other contexts
    QMapIterator<QString, QList<Context> > it(this->contexts);
    while (it.hasNext())
        if (!it.next().key().isEmpty())
            if ((instance = qobject_cast<T *>(it.value().first().getInstance())) && this->_checkContexts(it.value(), validator))
                instances.insert(this->id, instance);
    this->used += instances.size();
    return (instances);
}

#endif // PLUGIN_H
