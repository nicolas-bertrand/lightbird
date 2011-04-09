#include <QDir>
#include <QFile>
#include "Configurations.h"
#include "Log.h"
#include "Plugin.hpp"
#include "Plugins.hpp"

Plugin::Plugin(const QString &id, QObject *parent) : QObject(parent)
{
    this->id = id;
    this->_initialize();
    Log::debug("Plugin created", Properties("id", this->id), "Plugin", "Plugin");
}

Plugin::~Plugin()
{
    Log::trace("Plugin destroyed!", Properties("id", this->id), "Plugin", "~Plugin");
    // If the plugin is still loaded, we call onUnload before destroying it
    if (this->state == LightBird::IPlugins::LOADED)
        this->instance->onUnload();
    this->_clean();
    if (this->loader != NULL)
        delete this->loader;
    this->loader = NULL;
}

bool    Plugin::load(bool callOnLoad)
{
    if (!this->lockPlugin.tryLockForWrite(MAXTRYLOCK))
    {
        Log::error("Deadlock", "Plugin", "load");
        return (false);
    }
    Log::debug("Loading the plugin", Properties("id", this->id), "Plugin", "load");
    if (!this->_load())
    {
        Log::error("Unable to load the plugin", Properties("id", this->id), "Plugin", "load");
        this->_clean();
        this->lockPlugin.unlock();
        return (false);
    }
    if (callOnLoad && !this->instance->onLoad(this->api))
    {
        Log::error("The plugin returned false from IPlugin::onLoad, so it will not be loaded", Properties("id", this->id), "Plugin", "load");
        this->_clean();
        this->lockPlugin.unlock();
        return (false);
    }
    this->state = LightBird::IPlugins::LOADED;
    Log::info("Plugin loaded", Properties("id", this->id), "Plugin", "load");
    this->lockPlugin.unlock();
    return (true);
}

bool    Plugin::unload(bool callOnUnload)
{
    if (!this->lockPlugin.tryLockForWrite(MAXTRYLOCK))
    {
        Log::error("Deadlock", "Plugin", "unload");
        return (false);
    }
    Log::debug("Unloading the plugin", Properties("id", this->id), "Plugin", "unload");
    if (this->state == LightBird::IPlugins::UNLOADED)
    {
        this->lockPlugin.unlock();
        return (false);
    }
    if (this->state == LightBird::IPlugins::LOADED)
    {
        this->state = LightBird::IPlugins::UNLOADING;
        if (callOnUnload)
            this->instance->onUnload();
    }
    if (this->used == 0)
    {
        this->_unload();
        Log::info("Plugin unloaded", Properties("id", this->id), "Plugin", "unload");
    }
    else
        Log::debug("The plugin will be unloaded later because it is still used", Properties("id", this->id).add("used", QString::number(this->used)), "Plugins", "unload");
    this->lockPlugin.unlock();
    return (true);
}

bool    Plugin::install()
{
    if (!this->lockPlugin.tryLockForWrite(MAXTRYLOCK))
    {
        Log::error("Deadlock", "Plugin", "install");
        return (false);
    }
    Log::debug("Installing the plugin", Properties("id", this->id), "Plugin", "install");
    if (this->state != LightBird::IPlugins::LOADED)
    {
        Log::error("The plugin must be loaded to be installed", Properties("id", this->id), "Plugin", "install");
        this->lockPlugin.unlock();
        return (false);
    }
    if (this->configuration->get("installed") == "true")
    {
        Log::error("The plugin is already installed", Properties("id", this->id), "Plugin", "install");
        this->lockPlugin.unlock();
        return (false);
    }
    if (this->instance->onInstall(this->api) == false)
    {
        Log::error("The plugin returned false from IPlugin::onInstall(), so it will not be installed", Properties("id", this->id), "Plugin", "install");
        this->lockPlugin.unlock();
        return (false);
    }
    this->configuration->set("installed", "true");
    if (!this->configuration->save())
    {
        Log::error("Unable to save the configuration of the plugin in order to install it.", Properties("id", this->id), "Plugin", "install");
        this->lockPlugin.unlock();
        return (false);
    }
    Log::info("Plugin installed", Properties("id", this->id), "Plugin", "install");
    this->lockPlugin.unlock();
    return (true);
}

bool    Plugin::uninstall()
{
    if (!this->lockPlugin.tryLockForWrite(MAXTRYLOCK))
    {
        Log::error("Deadlock", "Plugin", "uninstall");
        return (false);
    }
    Log::debug("Uninstalling the plugin", Properties("id", this->id), "Plugin", "uninstall");
    if (this->state != LightBird::IPlugins::LOADED)
    {
        Log::error("The plugin must be loaded to be uninstalled", Properties("id", this->id), "Plugin", "uninstall");
        this->lockPlugin.unlock();
        return (false);
    }
    if (this->configuration->get("installed") != "true")
    {
        Log::error("The plugin is already uninstalled", Properties("id", this->id), "Plugin", "uninstall");
        this->lockPlugin.unlock();
        return (false);
    }
    this->instance->onUninstall(this->api);
    this->configuration->set("installed", "false");
    if (!this->configuration->save())
    {
        Log::error("Unable to save the configuration of the plugin in order to uninstall it.", Properties("id", this->id), "Plugin", "uninstall");
        this->lockPlugin.unlock();
        return (false);
    }
    Log::info("Plugin uninstalled", Properties("id", this->id), "Plugin", "uninstall");
    this->lockPlugin.unlock();
    return (true);
}

bool    Plugin::release()
{
    if (!this->lockPlugin.tryLockForWrite(MAXTRYLOCK))
    {
        Log::error("Deadlock", "Plugin", "release");
        return (false);
    }
    if (this->state == LightBird::IPlugins::UNLOADED)
    {
        Log::warning("The plugin is unloaded", Properties("id", this->id), "Plugin", "release");
        this->lockPlugin.unlock();
        return (false);
    }
    this->used--;
    if (this->used < 0)
    {
        Log::warning("Used plugin is lesser than 0", Properties("id", this->id), "Plugin", "release");
        this->used = 0;
    }
    if (this->used == 0 && this->state == LightBird::IPlugins::UNLOADING)
        this->_unload();
    this->lockPlugin.unlock();
    return (true);
}

LightBird::IMetadata     Plugin::getMetadata() const
{
    LightBird::IMetadata metadata;

    if (!this->lockPlugin.tryLockForRead(MAXTRYLOCK))
    {
        Log::error("Deadlock", "Plugin", "getMetadata");
        return (metadata);
    }
    this->instance->getMetadata(metadata);
    this->lockPlugin.unlock();
    return (metadata);
}

bool    Plugin::checkContext(const QString &transport, const QStringList &protocols,
                             unsigned short port, const QString &method, const QString &type, bool all)
{
    QListIterator<Context>  it(this->contexts);

    // Iterate through all the contexts of the plugin
    while (it.hasNext())
    {
        // If at least one context is valid, true is returned
        if ((!all && it.next().isValid(transport, protocols, port)) ||
            (all && it.next().isValid(transport, protocols, port, method, type)))
        {
            /*if (Log::instance()->isTrace())
                Log::trace("Context valid", Properties(it.peekPrevious().toMap()).add("id", this->id)
                           .add("requiredTransport", transport).add("requiredProtocols", protocols.join(" "))
                           .add("requiredPort", port).add("requiredMethod", method, false)
                           .add("requiredType", type, false).add("all", all), "Plugin", "checkContext");*/
            return (true);
        }
        /*else if (Log::instance()->isTrace())
            Log::trace("Context invalid", Properties(it.peekPrevious().toMap()).add("id", this->id)
                       .add("requiredTransport", transport).add("requiredProtocols", protocols.join(" "))
                       .add("requiredPort", port).add("requiredMethod", method, false)
                       .add("requiredType", type, false).add("all", all), "Plugin", "checkContext");*/
    }
    return (false);
}

LightBird::IPlugins::State      Plugin::getState()
{
    LightBird::IPlugins::State  state;

    if (!this->lockPlugin.tryLockForRead(MAXTRYLOCK))
    {
        Log::error("Deadlock", "Plugin", "getState");
        return (LightBird::IPlugins::UNKNOW);
    }
    state = this->state;
    this->lockPlugin.unlock();
    return (state);
}

void        Plugin::_initialize()
{
    this->state = LightBird::IPlugins::UNLOADED;
    this->used = 0;
    this->configuration = NULL;
    this->instance = NULL;
    this->instanceObject = NULL;
    this->loader = NULL;
    this->api = NULL;
    this->_clean();
    this->path = Configurations::instance()->get("pluginsPath") + "/" + this->id + "/";
    if (!this->_loadLibrary())
        return ;
    QFileInfo file(this->path + "Configuration.xml");
    if (!file.isFile() || !file.size())
        this->_createConfigurations();
}

bool                    Plugin::_loadLibrary()
{
    QStringList         nameFilters;
    LightBird::IPlugin  *instance;

    // List the possible extensions
    nameFilters << "*.dll" << "*.so" << "*.a" << "*.sl" << "*.dylib" << "*.bundle" << "*.sip";
    QStringListIterator dir(QDir(this->path).entryList(nameFilters, QDir::Files));
    // Iterate over the files of the plugin directory
    while (dir.hasNext() && !this->loader)
    {
        // If the file name has the good extension
        if (QLibrary::isLibrary(dir.peekNext()) || dir.peekNext().contains(".sip"))
        {
            this->loader = new QPluginLoader(this->path + dir.peekNext());
            this->libraryName = dir.peekNext();
            // If the plugin implements IPlugin
            if ((instance = qobject_cast<LightBird::IPlugin *>(loader->instance())) != NULL &&
                loader->isLoaded() == true)
                delete instance;
            else
            {
                delete this->loader;
                this->loader = NULL;
            }
        }
        dir.next();
    }
    if (!this->loader)
    {
        Log::error("Unable to find a valid plugin library", Properties("id", this->id).add("file", this->path + this->libraryName), "Plugin", "_load");
        return (false);
    }
    Log::trace("Plugin library found", Properties("id", this->id).add("file", this->path + this->libraryName), "Plugin", "_load");
    return (true);
}

void        Plugin::_createConfigurations()
{
    QString resourcesPath = Plugins::getResourcesPath(this->id);

    // Destroy the file if it is empty
    if (QFileInfo(this->path + "Configuration.xml").isFile() && !QFile::remove(this->path + "Configuration.xml"))
    {
        Log::warning("Failed to remove the configuration file of the plugin", Properties("id", this->id), "Plugin", "_createConfigurations");
        return ;
    }
    Log::trace("Creating the configuration file of the plugin from its resources", Properties("id", this->id), "Plugin", "_createConfigurations");
    // Creates the configuration from the resource of the plugin
    if (Configurations::instance(this->path + "Configuration.xml", resourcesPath + "/configuration"))
        Log::debug("Plugin configuration created", Properties("id", this->id).add("file", resourcesPath + "/configuration"), "Plugin", "_createConfigurations");
    else
        Log::warning("Unable to create the configuration of the plugin", Properties("id", this->id).add("file", resourcesPath + "/configuration"), "Plugin", "_createConfigurations");
}

bool    Plugin::_load()
{
    if (!this->loader)
    {
        Log::error("Failed to load the plugin", Properties("id", this->id), "Plugin", "_load");
        return (false);
    }
    if (!QFileInfo(this->path + "Configuration.xml").isFile())
    {
        Log::error("The configuration does not exists", Properties("id", this->id).add("file", this->path + "Configuration.xml"), "Plugin", "_load");
        return (false);
    }
    if (!(this->configuration = Configurations::instance(this->path + "Configuration.xml")))
    {
        Log::error("Failed to load the configuration file", Properties("id", this->id).add("file", this->path + "Configuration.xml"), "Plugin", "_load");
        return (false);
    }
    this->instanceObject = this->loader->instance();
    if (!(this->instance = qobject_cast<LightBird::IPlugin *>(this->instanceObject)))
    {
        Log::error("Failed to load the plugin", Properties("id", this->id), "Plugin", "_load");
        return (false);
    }
    bool timers = false;
    // If the plugin implements ITimer, they are loaded
    if (qobject_cast<LightBird::ITimer *>(this->instanceObject))
        timers = true;
    this->api = new Api(this->id, this->configuration, timers);
    this->_loadInformations();
    this->_loadResources();
    return (true);
}

void                        Plugin::_loadInformations()
{
    QDomNode                read;
    QDomNode                dom;
    QDomNode                contextNode;
    QString                 nodeName;
    QString                 nodeValue;

    // Load the contexts of the plugin
    read = this->configuration->readDom().firstChild();
    dom = read.parentNode().firstChildElement("contexts").firstChild();
    while (!dom.isNull())
    {
        // Iterates through all the contexts
        if (dom.isElement() && dom.nodeName().toLower().trimmed() == "context")
        {
            Context context;
            contextNode = dom.firstChild();
            // For each context, we store its informations
            while (!contextNode.isNull())
            {
                if (contextNode.isElement() && contextNode.toElement().text().trimmed().size() > 0)
                {
                    nodeName = contextNode.nodeName().toLower().trimmed();
                    nodeValue = contextNode.toElement().text().toLower().trimmed();
                    if (nodeName == "transport")
                    {
                        nodeValue = nodeValue.toUpper();
                        if (nodeValue != "TCP" && nodeValue != "UDP")
                            nodeValue.clear();
                        context.setTransport(nodeValue);
                    }
                    else if (nodeName == "protocol")
                        context.setProtocol(contextNode.toElement().text().trimmed());
                    else if (nodeName == "port")
                    {
                        if (nodeValue == "all")
                            context.setPort(0);
                        else if (nodeValue.toInt() != 0)
                            context.setPort(nodeValue.toInt());
                    }
                    else if (nodeName == "method")
                        context.setMethod(nodeValue);
                    else if (nodeName == "type")
                        context.setType(nodeValue);
                }
                contextNode = contextNode.nextSibling();
            }
            // Saves the context if it doesn't already exists
            if (!this->contexts.contains(context))
            {
                if (Log::instance()->isDebug())
                    Log::debug("Context added", Properties(context.toMap()).add("id", this->id), "Plugin", "_loadInformations");
                this->contexts.push_back(context);
            }
        }
        dom = dom.nextSibling();
    }
    this->configuration->release();
}

void    Plugin::_loadResources()
{
    QDomNode                read;
    QDomNode                dom;
    QString                 nodeName;
    QString                 nodeValue;
    QString                 path;
    QString                 resourcesPath;

    // Creates the resources of the plugin that doesn't exists
    resourcesPath = Plugins::getResourcesPath(this->id);
    read = this->configuration->readDom().firstChild();
    dom = read.parentNode().firstChildElement("resources").firstChild();
    while (!dom.isNull())
    {
        if (dom.isElement() && dom.toElement().text().trimmed().size() > 0)
        {
            nodeName = dom.nodeName().toLower().trimmed();
            nodeValue = dom.toElement().text().trimmed();
            if (nodeName == "resource" && !QFileInfo(this->path + nodeValue).isFile())
            {
                nodeValue = QDir::cleanPath(this->path + nodeValue);
                // Creates the directory of the resource if it doesn't exists
                path = nodeValue.left(nodeValue.lastIndexOf('/'));
                if (!QFileInfo(path).isDir())
                    QDir().mkpath(path);
                // Copy the resource
                nodeName = resourcesPath + "/" + dom.toElement().attribute("alias");
                Log::trace("Copying the resource of the plugin to the file system", Properties("id", this->id)
                           .add("file", nodeValue).add("resource", nodeName), "Plugin", "_loadInformations");
                if (!Configurations::copy(nodeName, nodeValue))
                    Log::warning("Unable to copy the plugin resource to the file system", Properties("id", this->id)
                                 .add("file", nodeValue).add("resource", nodeName), "Plugin", "_loadInformations");
            }
        }
        dom = dom.nextSibling();
    }
    this->configuration->release();
}

void    Plugin::_unload()
{
    this->_clean();
    this->state = LightBird::IPlugins::UNLOADED;
}

void    Plugin::_clean()
{
    this->configuration = NULL;
    if (this->instance != NULL)
        delete this->instance;
    this->instance = NULL;
    this->instanceObject = NULL;
    if (this->api != NULL)
        delete this->api;
    this->api = NULL;
}
