#include <QCoreApplication>
#include <QDir>

#include "ApiPlugins.h"
#include "Configurations.h"
#include "Events.h"
#include "Extensions.h"
#include "LightBird.h"
#include "Log.h"
#include "Plugins.hpp"
#include "Server.h"
#include "Mutex.h"
#include "Threads.h"

Plugins::Plugins()
{
    this->unloadAllPlugins = false;
    this->awake = false;
    this->libraryExtensions << "*.dll" << "*.so" << "*.a" << "*.sl" << "*.dylib" << "*.bundle";
    // Connects all the signals and slots
    qRegisterMetaType<Future<bool> >("Future<bool>");
    QObject::connect(this, SIGNAL(loadSignal(QString,Future<bool>*)), this, SLOT(_load(QString,Future<bool>*)), Qt::QueuedConnection);
    QObject::connect(this, SIGNAL(unloadSignal(QString,Future<bool>*)), this, SLOT(_unload(QString,Future<bool>*)), Qt::QueuedConnection);
    QObject::connect(this, SIGNAL(installSignal(QString,Future<bool>*)), this, SLOT(_install(QString,Future<bool>*)), Qt::QueuedConnection);
    QObject::connect(this, SIGNAL(uninstallSignal(QString,Future<bool>*)), this, SLOT(_uninstall(QString,Future<bool>*)), Qt::QueuedConnection);
    this->moveToThread(this);
    // Starts the plugins thread
    Threads::instance()->newThread(this, false);
    // Waits that the thread is started
    this->mutex.lockForWrite();
    if (!this->awake)
        this->wait.wait(&mutex);
    this->mutex.unlock();
}

Plugins::~Plugins()
{
    this->shutdown();
    this->quit();
    QThread::wait();
    LOG_TRACE("Plugins destroyed!", "Plugins", "~Plugins");
}

void    Plugins::run()
{
    LOG_DEBUG("Plugins thread started", "Plugins", "run");
    // Tells to the thread that started the current thread that it is running
    this->mutex.lockForWrite();
    this->awake = true;
    this->wait.wakeAll();
    this->mutex.unlock();
    // Executes the event loop
    this->exec();
    LOG_DEBUG("Plugins thread finished", "Plugins", "run");
    this->moveToThread(QCoreApplication::instance()->thread());
}

Future<bool>    Plugins::load(const QString &id)
{
    Future<bool> *future = new Future<bool>(false);
    Future<bool> result(*future);

    emit this->loadSignal(id, future);
    return (result);
}

Future<bool>    Plugins::unload(const QString &id)
{
    Future<bool> *future = new Future<bool>(false);
    Future<bool> result(*future);

    emit this->unloadSignal(id, future);
    return (result);
}

Future<bool>    Plugins::install(const QString &id)
{
    Future<bool> *future = new Future<bool>(false);
    Future<bool> result(*future);

    emit this->installSignal(id, future);
    return (result);
}

Future<bool>    Plugins::uninstall(const QString &id)
{
    Future<bool> *future = new Future<bool>(false);
    Future<bool> result(*future);

    emit this->uninstallSignal(id, future);
    return (result);
}

void    Plugins::shutdown()
{
    Mutex   mutex(this->mutex, "Plugins", "shutdown");

    if (!mutex)
        return ;
    this->unloadAllPlugins = true;
    QStringListIterator it(this->orderedPlugins);
    mutex.unlock();
    // Tries to unload all the plugins
    while (it.hasNext())
        this->unload(it.next()).getResult();
    if (!mutex.lock())
        return ;
    // If some plugins are still loaded, this mean that they are still used
    // and we wait until they are released.
    if (this->plugins.size() > 0)
    {
        LOG_INFO("Some plugins are still used. The server is waiting that all the plugins are unloaded...", "Plugins", "shutdown");
        this->wait.wait(&this->mutex);
        LOG_INFO("All plugins has been unloaded", "Plugins", "shutdown");
    }
}

void    Plugins::_load(const QString &identifier, Future<bool> *f)
{
    Plugin  *plugin;
    QString id = Plugins::checkId(identifier);
    QSharedPointer<Future<bool> > future(f);

    LOG_DEBUG("Loading the plugin", Properties("id", id), "Plugins", "_load");
    Mutex mutex(this->mutex, "Plugins", "_load");
    if (!mutex)
        return ;
    if (this->unloadAllPlugins)
    {
        LOG_WARNING("No plugins can be loaded, because all plugins are unloading.", Properties("id", id), "Plugins", "_load");
        return ;
    }
    if (this->plugins.contains(id))
    {
        LOG_WARNING("The plugin is already loaded", Properties("id", id), "Plugins", "_load");
        return ;
    }
    if (this->_getState(id) != LightBird::IPlugins::UNLOADED)
    {
        if (this->_getState(id) == LightBird::IPlugins::UNKNOW)
            Log::error("The plugin has not been found", Properties("id", id), "Plugins", "_load");
        else
            Log::error("The plugin is not installed", Properties("id", id), "Plugins", "_load");
        return ;
    }
    plugin = new Plugin(id, this);
    if (!plugin->load())
    {
        LOG_ERROR("Unable to load the plugin", Properties("id", id), "Plugins", "_load");
        delete plugin;
        return ;
    }
    Extensions::instance()->add(plugin);
    this->plugins.insert(id, plugin);
    this->orderedPlugins.push_back(id);
    Events::instance()->send("plugin_loaded", id);
    LOG_INFO("Plugin loaded", Properties("id", id), "Plugins", "_load");
    emit this->loaded(id);
    future->setResult(true);
}

void    Plugins::_unload(const QString &id, Future<bool> *f)
{
    QSharedPointer<Future<bool> > future(f);

    LOG_DEBUG("Unloading the plugin", Properties("id", id), "Plugins", "_unload");
    Mutex mutex(this->mutex, "Plugins", "_unload");
    if (!mutex)
        return ;
    if (!this->plugins.contains(id))
    {
        LOG_WARNING("The plugin is already unloaded or does not exist", Properties("id", id), "Plugins", "_unload");
        return ;
    }
    if (this->plugins.value(id)->getState() != LightBird::IPlugins::LOADED)
    {
        LOG_WARNING("The plugin is already unloading", Properties("id", id), "Plugins", "_unload");
        return ;
    }
    Extensions::instance()->remove(this->plugins.value(id));
    if (!this->plugins.value(id)->unload())
    {
        LOG_ERROR("Unable to unload the plugin", Properties("id", id), "Plugins", "_unload");
        return ;
    }
    if (this->plugins.value(id)->getState() == LightBird::IPlugins::UNLOADED)
    {
        delete this->plugins.value(id);
        this->plugins.remove(id);
        this->orderedPlugins.removeAll(id);
        Events::instance()->send("plugin_unloaded", id);
        LOG_INFO("Plugin unloaded", Properties("id", id), "Plugins", "_unload");
    }
    future->setResult(true);
}

void    Plugins::_install(const QString &id, Future<bool> *f)
{
    LightBird::IPlugins::State    state;
    QSharedPointer<Future<bool> > future(f);

    LOG_DEBUG("Installing the plugin", Properties("id", id), "Plugins", "_install");
    Mutex mutex(this->mutex, "Plugins", "_install");
    if (!mutex)
        return ;
    if (this->unloadAllPlugins)
    {
        LOG_WARNING("No plugins can be installed, because all plugins are unloading.", Properties("id", id), "Plugins", "_install");
        return ;
    }
    if ((state = this->_getState(id)) != LightBird::IPlugins::UNINSTALLED)
    {
        LOG_WARNING(state != LightBird::IPlugins::UNKNOW ? "The plugin is already installed" : "The plugin is unknow",
                    Properties("id", id).add("state", QString::number(state)), "Plugins", "_install");
        return ;
    }
    Plugin plugin(id);
    if (!plugin.load(false))
    {
        LOG_WARNING("Unable to load the plugin in order to install it", Properties("id", id), "Plugins", "_install");
        return ;
    }
    if (!plugin.install())
    {
        LOG_WARNING("Unable to install the plugin", Properties("id", id), "Plugins", "_install");
        return ;
    }
    Events::instance()->send("plugin_installed", id);
    LOG_INFO("Plugin installed", Properties("id", id), "Plugins", "_install");
    future->setResult(true);
}

void    Plugins::_uninstall(const QString &id, Future<bool> *f)
{
    LightBird::IPlugins::State    state;
    QSharedPointer<Future<bool> > future(f);

    LOG_DEBUG("Uninstalling the plugin", Properties("id", id), "Plugins", "_uninstall");
    Mutex mutex(this->mutex, "Plugins", "_uninstall");
    if (!mutex)
        return ;
    if (this->unloadAllPlugins)
    {
        LOG_WARNING("No plugins can be uninstalled, because all plugins are unloading.", Properties("id", id), "Plugins", "_uninstall");
        return ;
    }
    if ((state = this->_getState(id)) == LightBird::IPlugins::UNINSTALLED)
    {
        LOG_WARNING("The plugin is already uninstalled", Properties("id", id), "Plugins", "_uninstall");
        return ;
    }
    if (this->plugins.contains(id))
    {
        LOG_WARNING("The plugin must be unloaded to be uninstalled", Properties("id", id), "Plugins", "_uninstall");
        return ;
    }
    Plugin plugin(id);
    if (!plugin.load(false))
    {
        LOG_WARNING("Unable to load the plugin in order to uninstall it", Properties("id", id), "Plugins", "_uninstall");
        return ;
    }
    if (!plugin.uninstall())
    {
        LOG_WARNING("Unable to uninstall the plugin", Properties("id", id), "Plugins", "_uninstall");
        return ;
    }
    Events::instance()->send("plugin_uninstalled", id);
    LOG_INFO("Plugin uninstalled", Properties("id", id), "Plugins", "_uninstall");
    future->setResult(true);
}

bool    Plugins::release(const QString &id)
{
    Mutex   mutex(this->mutex, "Plugins", "release");

    if (!mutex)
        return (false);
    if (!this->plugins.contains(id))
        return (false);
    this->plugins.value(id)->release();
    if (this->plugins.value(id)->getState() == LightBird::IPlugins::UNLOADED)
    {
        this->plugins.value(id)->deleteLater();
        this->plugins.remove(id);
        this->orderedPlugins.removeAll(id);
        Events::instance()->send("plugin_unloaded", id);
        LOG_INFO("Plugin unloaded", Properties("id", id), "Plugins", "release");
        if (this->unloadAllPlugins && this->plugins.size() == 0)
            this->wait.wakeAll();
    }
    return (true);
}

LightBird::IMetadata Plugins::getMetadata(const QString &id) const
{
    Mutex   mutex(this->mutex, Mutex::READ, "Plugins", "getMetadata");
    Plugin  *plugin;
    LightBird::IMetadata metadata;

    if (!mutex)
        return (metadata);
    if (this->plugins.contains(id))
        plugin = this->plugins.value(id);
    else
    {
        plugin = new Plugin(id);
        if (!plugin->load(false))
        {
            LOG_WARNING("Unable to load the plugin in order to get its metadata", Properties("id", id), "Plugins", "getMetadata");
            delete plugin;
            return (metadata);
        }
    }
    metadata = plugin->getMetadata();
    if (!this->plugins.contains(id))
        delete plugin;
    return (metadata);
}

QString Plugins::getResourcesPath(const QString &id)
{
    return (QString(PLUGINS_RESOURCES_PATH) + "/" + id);
}

QString Plugins::checkId(const QString &identifier)
{
    QString id;
    QString path;
    QString result;

    path = Configurations::c().pluginsPath;
    QStringListIterator dir(LightBird::cleanPath(identifier).split("/"));
    // Iterates over each directory of the id of the plugin
    while (dir.hasNext())
    {
        id = dir.peekNext();
        QStringListIterator it(QDir(path).entryList(QStringList(dir.peekNext()), QDir::Dirs));
        // For each directory, search the correct case
        while (it.hasNext())
        {
            if (it.peekNext() == dir.peekNext())
            {
                id = it.peekNext();
                break;
            }
            if (it.peekNext().toLower() == dir.peekNext().toLower())
                id = it.peekNext();
            it.next();
        }
        path += "/" + dir.peekNext();
        dir.next();
        result += id;
        if (dir.hasNext())
            result += "/";
    }
    return (result);
}

bool    Plugins::isInstalled(const QString &id)
{
    QDomElement element;

    element = Configurations::instance()->readDom().firstChildElement("configurations");
    for (element = element.firstChildElement("plugin"); !element.isNull() && element.attribute("id") != id; element = element.nextSiblingElement("plugin"))
        ;
    Configurations::instance()->release();
    return (!element.isNull());
}

QStringList Plugins::getLibraryExtensions()
{
    return (Plugins::instance()->libraryExtensions);
}

LightBird::IPlugins::State Plugins::getState(const QString &id) const
{
    Mutex   mutex(this->mutex, Mutex::READ, "Plugins", "getState");
    LightBird::IPlugins::State  state;

    if (!mutex)
        return (LightBird::IPlugins::UNKNOW);
    state = this->_getState(id);
    return (state);
}

QStringList Plugins::getPlugins() const
{
    QStringList plugins;

    this->_findPlugins(Configurations::c().pluginsPath, "", plugins);
    return (plugins);
}

QStringList Plugins::getLoadedPlugins() const
{
    Mutex   mutex(this->mutex, Mutex::READ, "Plugins", "getLoadedPlugins");
    QStringList list;

    if (!mutex)
        return (list);
    list = this->orderedPlugins;
    return (list);
}

QStringList Plugins::getUnloadedPlugins() const
{
    QStringList plugins;

    // Gets all the installed plugins
    plugins = this->getInstalledPlugins();
    // Removes the plugins loaded
    QStringListIterator it(this->getLoadedPlugins());
    while (it.hasNext())
        plugins.removeAll(it.next());
    return (plugins);
}

QStringList Plugins::getInstalledPlugins() const
{
    QStringList plugins;
    QDomElement element;

    element = Configurations::instance()->readDom().firstChildElement("configurations");
    for (element = element.firstChildElement("plugin"); !element.isNull(); element = element.nextSiblingElement("plugin"))
        if (element.hasAttribute("id"))
            plugins << element.attribute("id");
    Configurations::instance()->release();
    return (plugins);
}

QStringList Plugins::getUninstalledPlugins() const
{
    QStringList plugins;

    // Gets all the plugins
    this->_findPlugins(Configurations::c().pluginsPath, "", plugins);
    // Removes the plugins installed
    QStringListIterator it(this->getInstalledPlugins());
    while (it.hasNext())
        plugins.removeAll(it.next());
    return (plugins);
}

void    Plugins::_findPlugins(const QString &pluginsPath, const QString &path, QStringList &plugins) const
{
    QString id;

    // Runs through all the directories of the current location to find the plugins
    QStringListIterator it(QDir(pluginsPath + "/" + path).entryList(QDir::Dirs | QDir::NoDotAndDotDot));
    while (it.hasNext())
    {
        if (!path.isEmpty())
            id = path + "/" + it.peekNext();
        else
            id = it.peekNext();
        // If there is a library in the current directory, it is a plugin
        if (!QDir(pluginsPath + "/" + id).entryList(this->libraryExtensions, QDir::Files).isEmpty())
            plugins << id;
        this->_findPlugins(pluginsPath, id, plugins);
        it.next();
    }
}

LightBird::IPlugins::State Plugins::_getState(const QString &id) const
{
    // A loaded plugin can be LOADED, UNLOADING, or UNLOADED
    if (this->plugins.contains(id))
        return (this->plugins.value(id)->getState());
    // The plugin was not found
    else if (!QFileInfo(Configurations::c().pluginsPath + "/" + id).isDir())
        return (LightBird::IPlugins::UNKNOW);
    // The plugin is installed but not loaded
    else if (Plugins::isInstalled(id))
        return (LightBird::IPlugins::UNLOADED);
    // The plugin is uninstalled
    else
        return (LightBird::IPlugins::UNINSTALLED);
}

Plugins *Plugins::instance()
{
    return (Server::instance().getPlugins());
}
