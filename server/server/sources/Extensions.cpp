#include "Extensions.h"
#include "Log.h"
#include "Plugin.hpp"
#include "Plugins.hpp"

Extensions  *Extensions::_instance = NULL;

Extensions  *Extensions::instance(QObject *parent)
{
    if (Extensions::_instance == NULL)
        Extensions::_instance = new Extensions(parent);
    return (Extensions::_instance);
}

Extensions::Extensions(QObject *parent) : QObject(parent)
{
    Log::trace("Extensions created", "Extensions", "Extensions");
    QObject::connect(this, SIGNAL(releaseSignal(QString)), this, SLOT(_release(QString)), Qt::QueuedConnection);
}

Extensions::~Extensions()
{
    Log::trace("Extensions destroyed!", "Extensions", "~Extensions");
}

void    Extensions::add(Plugin *plugin)
{
    LightBird::IExtension   *extension;
    Properties              properties;

    if (!this->mutex.tryLock(MAXTRYLOCK))
    {
        Log::error("Deadlock", "Extensions", "add");
        return ;
    }
    // Check that the plugin implements the IExtensions interface
    if ((extension = qobject_cast<LightBird::IExtension *>(plugin->instanceObject)) == NULL)
        return (this->mutex.unlock());
    // If the plugin is already managed, an error occured
    if (this->plugins.contains(plugin->id))
    {
        Log::error("The plugin is already managed", "Extensions", "add");
        return (this->mutex.unlock());
    }
    // Get the names of the extensions that the plugin implements
    QStringListIterator it(extension->getExtensionsNames());
    // If the plugin doesn't implements any extensions, it is not added
    if (!it.hasNext())
        return (this->mutex.unlock());
    while (it.hasNext())
    {
        properties.add("extensionName", it.peekNext());
        this->extensionsPlugins.insertMulti(it.next(), plugin->id);
    }
    // Add the plugin
    this->plugins[plugin->id].instance = plugin;
    this->plugins[plugin->id].extensions = extension;
    this->plugins[plugin->id].loaded = true;
    Log::debug("Extensions plugin added", properties.add("pluginId", plugin->id), "Extensions", "add");
    this->mutex.unlock();
}

void        Extensions::remove(Plugin *plugin)
{
    if (!this->mutex.tryLock(MAXTRYLOCK))
    {
        Log::error("Deadlock", "Extensions", "remove");
        return ;
    }
    // Check that the plugin exists
    if (!this->plugins.contains(plugin->id))
        return (this->mutex.unlock());
    this->plugins[plugin->id].loaded = false;
    this->_remove(plugin->id);
    Log::debug("Extensions plugin removed", Properties("pluginId", plugin->id), "Extensions", "remove");
    this->mutex.unlock();
}

QList<void *>       Extensions::get(const QString &name)
{
    QList<void *>   result;
    Extension       extension;

    if (!this->mutex.tryLock(MAXTRYLOCK))
    {
        Log::error("Deadlock", "Extensions", "remove");
        return (result);
    }
    // Search all the plugins that implements the required extension
    QStringListIterator it(this->extensionsPlugins.values(name));
    while (it.hasNext())
    {
        // If the plugin exists and is still loaded
        if (this->plugins.contains(it.peekNext()) && this->plugins[it.peekNext()].loaded == true)
        {
            // Ask the plugin to instanciate the extension
            extension.instance = this->plugins[it.peekNext()].extensions->getExtension(name);
            // If the extension is valid, it is saved, and put un the result list
            if (extension.instance != NULL)
            {
                extension.plugin = it.peekNext();
                this->extensions.insertMulti(name, extension);
                result.push_back(extension.instance);
                Log::trace("Extension instance created", Properties("pluginId", it.peekNext()).add("extensionName", name), "Extensions", "get");
                // Increments the used variable of plugins, so that it will not be unloaded until the extension is released
                this->plugins[it.peekNext()].instance->used++;
            }
        }
        it.next();
    }
    this->mutex.unlock();
    return (result);
}

void        Extensions::release(QList<void *> extensions)
{
    QString plugin;

    if (!this->mutex.tryLock(MAXTRYLOCK))
    {
        Log::error("Deadlock", "Extensions", "remove");
        return ;
    }
    // For each extensions to release
    QListIterator<void *> it1(extensions);
    while (it1.hasNext())
    {
        // Search the current extension
        QMutableMapIterator<QString, Extension> it2(this->extensions);
        while (it2.hasNext())
        {
            it2.next();
            // Release it
            if (it2.value().instance == it1.peekNext())
            {
                plugin = it2.value().plugin;
                this->plugins[plugin].extensions->releaseExtension(it2.key(), it2.value().instance);
                emit this->releaseSignal(plugin);
                Log::trace("Extension instance released", Properties("pluginId", plugin).add("extensionName", it2.key()), "Extensions", "release");
                it2.remove();
                if (this->plugins[plugin].loaded == false)
                    this->_remove(plugin);
                break;
            }
        }
        it1.next();
    }
    this->mutex.unlock();
}

void        Extensions::_remove(const QString &plugin)
{
    bool    used = false;

    // Check if some extensions of the plugin are still used
    QMapIterator<QString, Extension> it1(this->extensions);
    while (it1.hasNext() && !used)
    {
        it1.next();
        // One of the extensions of the plugin is still used
        if (it1.value().plugin == plugin)
            used = true;
    }
    if (!used)
    {
        this->plugins.remove(plugin);
        QMutableMapIterator<QString, QString> it2(this->extensionsPlugins);
        while (it2.hasNext())
        {
            it2.next();
            if (it2.value() == plugin)
                it2.remove();
        }
        Log::trace("Plugin removed", Properties("pluginId", plugin), "Extensions", "_remove");
    }
    else
        Log::trace("Some extensions of this plugin are still used", Properties("pluginId", plugin), "Extensions", "_remove");
}

void    Extensions::_release(QString id)
{
    Plugins::instance()->release(id);
}
