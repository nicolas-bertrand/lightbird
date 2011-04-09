#include <QDir>

#include "Log.h"
#include "Plugins.hpp"
#include "ApiPlugins.h"
#include "Threads.h"
#include "IGui.h"
#include "Extensions.h"

Plugins *Plugins::_instance = NULL;

Plugins *Plugins::instance(QObject *parent)
{
    if (Plugins::_instance == NULL)
        Plugins::_instance = new Plugins(parent);
    return (Plugins::_instance);
}

bool    Plugins::isLoaded()
{
    if (Plugins::_instance)
        return (true);
    return (false);
}

Plugins::Plugins(QObject *parent)
{
    this->parent = parent;
    this->unloadAllPlugins = false;
    this->awake = false;
    // Connect all the signals and slots
    qRegisterMetaType<Future<bool> >("Future<bool>");
    QObject::connect(this, SIGNAL(loadSignal(QString,Future<bool>*)), this, SLOT(_load(QString,Future<bool>*)), Qt::QueuedConnection);
    QObject::connect(this, SIGNAL(unloadSignal(QString,Future<bool>*)), this, SLOT(_unload(QString,Future<bool>*)), Qt::QueuedConnection);
    QObject::connect(this, SIGNAL(installSignal(QString,Future<bool>*)), this, SLOT(_install(QString,Future<bool>*)), Qt::QueuedConnection);
    QObject::connect(this, SIGNAL(uninstallSignal(QString,Future<bool>*)), this, SLOT(_uninstall(QString,Future<bool>*)), Qt::QueuedConnection);
    // Starting the plugins thread
    this->moveToThread(this);
    Threads::instance()->newThread(this);
    // Wait that the thread is started
    this->mutex.lockForWrite();
    if (!this->awake)
        this->wait.wait(&mutex);
    this->mutex.unlock();
}

Plugins::~Plugins()
{
    Log::trace("Plugins destroyed!", "Plugins", "~Plugins");
}

void        Plugins::run()
{
    // Initialize the plugins API
    ApiPlugins::instance(this);
    // Initialize the extension manager
    Extensions::instance(this);
    Log::debug("Plugins thread started", "Plugins", "run");
    // Tells to the thread that started the current thread that it is running
    this->mutex.lockForWrite();
    this->awake = true;
    this->wait.wakeAll();
    this->mutex.unlock();
    // Execute the event loop
    this->exec();
    Log::debug("Plugins thread finished", "Plugins", "run");
    // The thread where lives the Plugins is changed to the thread of its parent
    if (this->parent != NULL)
        this->moveToThread(this->parent->thread());
}

Future<bool>        Plugins::load(const QString &id)
{
    Future<bool>    *future = new Future<bool>(false);
    Future<bool>    result(*future);

    emit this->loadSignal(id, future);
    return (result);
}

Future<bool>        Plugins::unload(const QString &id)
{
    Future<bool>    *future = new Future<bool>(false);
    Future<bool>    result(*future);

    emit this->unloadSignal(id, future);
    return (result);
}

Future<bool>        Plugins::install(const QString &id)
{
    Future<bool>    *future = new Future<bool>(false);
    Future<bool>    result(*future);

    emit this->installSignal(id, future);
    return (result);
}

Future<bool>        Plugins::uninstall(const QString &id)
{
    Future<bool>    *future = new Future<bool>(false);
    Future<bool>    result(*future);

    emit this->uninstallSignal(id, future);
    return (result);
}

void    Plugins::unloadAll()
{
    Log::error("Unloading all the plugins", "Plugins", "unloadAll");
    if (!this->mutex.tryLockForRead(MAXTRYLOCK))
    {
        Log::error("Deadlock", "Plugins", "unloadAll");
        return ;
    }
    this->unloadAllPlugins = true;
    QStringListIterator it(this->orderedPlugins);
    this->mutex.unlock();
    // Try to unload all the plugins
    while (it.hasNext())
        this->unload(it.next()).getResult();
    if (!this->mutex.tryLockForRead(MAXTRYLOCK))
    {
        Log::error("Deadlock", "Plugins", "unloadAll");
        return ;
    }
    // If some plugins are still loaded, this mean that they are still used
    // and we wait until they are released.
    if (this->plugins.size() > 0)
    {
        Log::info("Some plugins are still used. The server is waiting that all the plugins are unloaded...", "Plugins", "unloadAll");
        this->wait.wait(&this->mutex);
        Log::info("All plugins has been unloaded", "Plugins", "unloadAll");
    }
    this->mutex.unlock();
}

void                                Plugins::_load(const QString &identifier, Future<bool> *f)
{
    Plugin                          *plugin;
    QString                         id = this->_checkId(identifier);
    QSharedPointer<Future<bool> >   future(f);

    if (!this->mutex.tryLockForWrite(MAXTRYLOCK))
    {
        Log::error("Deadlock", "Plugins", "_load");
        return ;
    }
    if (this->unloadAllPlugins)
    {
        Log::warning("No plugins can be loaded, because all plugins are unloading.", Properties("id", id), "Plugins", "_load");
        this->mutex.unlock();
        return ;
    }
    if (this->plugins.contains(id))
    {
        Log::warning("The plugin is already loaded", Properties("id", id), "Plugins", "_load");
        this->mutex.unlock();
        return ;
    }
    plugin = new Plugin(id, this);
    if (this->_getState(id) != Streamit::IPlugins::UNLOADED)
    {
        Log::error("The plugin is not installed", Properties("id", id), "Plugins", "_load");
        delete plugin;
        this->mutex.unlock();
        return ;
    }
    this->lockResourcesPath.lock();
    this->resourcesPath.insert(id, plugin->getResourcePath());
    this->lockResourcesPath.unlock();
    if (!plugin->load())
    {
        Log::error("Unable to load the plugin", Properties("id", id), "Plugins", "_load");
        delete plugin;
        this->mutex.unlock();
        return ;
    }
    Extensions::instance()->add(plugin);
    this->plugins.insert(id, plugin);
    this->orderedPlugins.push_back(id);
    Log::debug("Plugin loaded", Properties("id", id), "Plugins", "_load");
    emit this->loaded(id);
    future->setResult(true);
    this->mutex.unlock();
}

void                                Plugins::_unload(const QString &id, Future<bool> *f)
{
    QSharedPointer<Future<bool> >   future(f);

    if (!this->mutex.tryLockForWrite(MAXTRYLOCK))
    {
        Log::error("Deadlock", "Plugins", "_unload");
        return ;
    }
    if (!this->plugins.contains(id))
    {
        Log::warning("The plugin is already unloaded", Properties("id", id), "Plugins", "_unload");
        this->mutex.unlock();
        return ;
    }
    if (this->plugins.value(id)->getState() != Streamit::IPlugins::LOADED)
    {
        Log::warning("The plugin is already unloading", Properties("id", id), "Plugins", "_unload");
        this->mutex.unlock();
        return ;
    }
    Extensions::instance()->remove(this->plugins.value(id));
    if (!this->plugins.value(id)->unload())
    {
        Log::error("Unable to unload the plugin", Properties("id", id), "Plugins", "_unload");
        this->mutex.unlock();
        return ;
    }
    if (this->plugins.value(id)->getState() == Streamit::IPlugins::UNLOADED)
    {
        delete this->plugins.value(id);
        this->plugins.remove(id);
        this->orderedPlugins.removeAll(id);
        Log::debug("Plugin unloaded", Properties("id", id), "Plugins", "_unload");
    }
    future->setResult(true);
    this->mutex.unlock();
    return ;
}

void                                Plugins::_install(const QString &id, Future<bool> *f)
{
    Streamit::IPlugins::State       state;
    QSharedPointer<Future<bool> >   future(f);

    if (!this->mutex.tryLockForWrite(MAXTRYLOCK))
    {
        Log::error("Deadlock", "Plugins", "_install");
        return ;
    }
    if (this->unloadAllPlugins)
    {
        Log::warning("No plugins can be installed, because all plugins are unloading.", Properties("id", id), "Plugins", "_install");
        this->mutex.unlock();
        return ;
    }
    if ((state = this->_getState(id)) != Streamit::IPlugins::UNINSTALLED &&
        state != Streamit::IPlugins::UNKNOW)
    {
        Log::warning("The plugin is already installed", Properties("id", id).add("state", QString::number(state)), "Plugins", "_install");
        this->mutex.unlock();
        return ;
    }
    Plugin plugin(id);
    this->lockResourcesPath.lock();
    this->resourcesPath.insert(id, plugin.getResourcePath());
    this->lockResourcesPath.unlock();
    if (!plugin.load(false))
    {
        Log::warning("Unable to load the plugin in order to install it", Properties("id", id), "Plugins", "_install");
        this->mutex.unlock();
        return ;
    }
    if (!plugin.install())
    {
        Log::warning("Unable to install the plugin", Properties("id", id), "Plugins", "_install");
        this->mutex.unlock();
        return ;
    }
    Log::debug("Plugin installed", Properties("id", id), "Plugins", "_install");
    future->setResult(true);
    this->mutex.unlock();
    return ;
}

void                                Plugins::_uninstall(const QString &id, Future<bool> *f)
{
    Streamit::IPlugins::State       state;
    QSharedPointer<Future<bool> >   future(f);

    if (!this->mutex.tryLockForWrite(MAXTRYLOCK))
    {
        Log::error("Deadlock", "Plugins", "_uninstall");
        return ;
    }
    if (this->unloadAllPlugins)
    {
        Log::warning("No plugins can be uninstalled, because all plugins are unloading.", Properties("id", id), "Plugins", "_uninstall");
        this->mutex.unlock();
        return ;
    }
    if ((state = this->_getState(id)) == Streamit::IPlugins::UNINSTALLED)
    {
        Log::warning("The plugin is already uninstalled", Properties("id", id), "Plugins", "_uninstall");
        this->mutex.unlock();
        return ;
    }
    if (this->plugins.contains(id))
    {
        Log::warning("The plugin must be unloaded to be uninstalled", Properties("id", id), "Plugins", "_uninstall");
        this->mutex.unlock();
        return ;
    }
    Plugin plugin(id);
    if (!plugin.load(false))
    {
        Log::warning("Unable to load the plugin in order to uninstall it", Properties("id", id), "Plugins", "_uninstall");
        this->mutex.unlock();
        return ;
    }
    if (!plugin.uninstall())
    {
        Log::warning("Unable to uninstall the plugin", Properties("id", id), "Plugins", "_uninstall");
        this->mutex.unlock();
        return ;
    }
    Log::debug("Plugin uninstalled", Properties("id", id), "Plugins", "_uninstall");
    future->setResult(true);
    this->mutex.unlock();
}

bool    Plugins::release(const QString &id)
{
    if (!this->mutex.tryLockForWrite(MAXTRYLOCK))
    {
        Log::error("Deadlock", "Plugins", "release");
        return (false);
    }
    if (!this->plugins.contains(id))
    {
        this->mutex.unlock();
        return (false);
    }
    this->plugins.value(id)->release();
    if (this->plugins.value(id)->getState() == Streamit::IPlugins::UNLOADED)
    {
        this->plugins.value(id)->deleteLater();
        this->plugins.remove(id);
        this->orderedPlugins.removeAll(id);
        Log::debug("Plugin unloaded", Properties("id", id), "Plugins", "release");
        if (this->unloadAllPlugins && this->plugins.size() == 0)
            this->wait.wakeAll();
    }
    this->mutex.unlock();
    return (true);
}

Streamit::IMetadata     Plugins::getMetadata(const QString &id) const
{
    Streamit::IMetadata metadata;
    Plugin              *plugin;

    if (!this->mutex.tryLockForRead(MAXTRYLOCK))
    {
        Log::error("Deadlock", "Plugins", "getMetadata");
        return (metadata);
    }
    if (this->plugins.contains(id))
        plugin = this->plugins.value(id);
    else
    {
        plugin = new Plugin(id);
        if (!plugin->load(false))
        {
            Log::warning("Unable to load the plugin in order to get its metadata", Properties("id", id), "Plugins", "getMetadata");
            delete plugin;
            this->mutex.unlock();
            return (metadata);
        }
    }
    metadata = plugin->getMetadata();
    if (!this->plugins.contains(id))
        delete plugin;
    this->mutex.unlock();
    return (metadata);
}

Streamit::IPlugins::State Plugins::getState(const QString &id)
{
    Streamit::IPlugins::State   state;

    if (!this->mutex.tryLockForRead(MAXTRYLOCK))
    {
        Log::error("Deadlock", "Plugins", "getState");
        return (Streamit::IPlugins::UNKNOW);
    }
    state = this->_getState(id);
    this->mutex.unlock();
    return (state);
}

QString     Plugins::getResourcesPath(const QString &id)
{
    QString resourcesPath;

    this->lockResourcesPath.lock();
    resourcesPath = this->resourcesPath.value(id);
    this->lockResourcesPath.unlock();
    return (resourcesPath);
}

QStringList Plugins::getPlugins()
{
    return (this->getInstalledPlugins() + this->getUninstalledPlugins());
}

QStringList     Plugins::getLoadedPlugins()
{
    QStringList list;

    if (!this->mutex.tryLockForRead(MAXTRYLOCK))
    {
        Log::error("Deadlock", "Plugins", "getLoadedPlugins");
        return (list);
    }
    list = this->orderedPlugins;
    this->mutex.unlock();
    return (list);
}

QStringList     Plugins::getInstalledPlugins()
{
    QStringList plugins;

    this->_findPlugins(Configurations::instance()->get("pluginsPath"), "", plugins, true);
    return (plugins);
}

QStringList Plugins::getUninstalledPlugins()
{
    QStringList plugins;

    this->_findPlugins(Configurations::instance()->get("pluginsPath"), "", plugins, false);
    return (plugins);
}

void                    Plugins::_findPlugins(const QString &pluginsPath, const QString &path, QStringList &plugins, bool installed)
{
    Configuration       *c;
    QString             d;
    QStringListIterator it(QDir(pluginsPath + "/" + path).entryList(QDir::Dirs | QDir::NoDotAndDotDot));

    while (it.hasNext())
    {
        if (!path.isEmpty())
            d = path + "/" + it.peekNext();
        else
            d = it.peekNext();
        // If the plugin is installed
        if (installed && QFileInfo(pluginsPath + "/" + d + "/Configuration.xml").isFile() && (c = Configurations::instance(pluginsPath + "/" + d + "/Configuration.xml")) && c->get("installed") == "true")
            plugins << d;
        // If it is uninstalled
        else if (!installed)
        {
            if (!QFileInfo(pluginsPath + "/" + d + "/Configuration.xml").isFile())
            {
                if (QDir(pluginsPath + "/" + d).entryList(QDir::Files | QDir::NoDotAndDotDot).size() > 0)
                    plugins << d;
            }
            else if (!(c = Configurations::instance(pluginsPath + "/" + d + "/Configuration.xml")) || c->get("installed") != "true")
                plugins << d;
        }
        this->_findPlugins(pluginsPath, d, plugins, installed);
        it.next();
    }
}

Streamit::IPlugins::State   Plugins::_getState(const QString &id)
{
    Configuration           *configuration = NULL;
    QString                 path;

    path = Configurations::instance()->get("pluginsPath") + "/" + id + "/";
    if (QFileInfo(path + "Configuration.xml").isFile())
        configuration = Configurations::instance(path);
    // A loaded plugin can be LOADED, UNLOADING, or UNLOADED
    if (this->plugins.contains(id))
        return (this->plugins.value(id)->getState());
    // The plugin was not found
    else if (!QFileInfo(path).isDir())
        return (Streamit::IPlugins::UNKNOW);
    // The plugin is installed but not loaded
    else if (configuration && configuration->get("installed") == "true")
        return (Streamit::IPlugins::UNLOADED);
    // The plugin is uninstalled
    else
        return (Streamit::IPlugins::UNINSTALLED);
}

QString Plugins::_checkId(const QString &identifier)
{
    QString         id;
    QString         path;
    QString         result;

    path = Configurations::instance()->get("pluginsPath");
    QStringListIterator dir(QDir::cleanPath(identifier).split("/"));
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
                break ;
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
