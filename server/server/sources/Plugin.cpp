#include <QDir>
#include <QDomElement>
#include <QFile>
#include <QFileInfo>

#include "IEvent.h"
#include "ITimer.h"

#include "Configurations.h"
#include "LightBird.h"
#include "Log.h"
#include "Plugin.hpp"
#include "Plugins.hpp"
#include "SmartMutex.h"

Plugin::Plugin(const QString &identifier, QObject *parent) : QObject(parent),
                                                             id(identifier)
{
    LOG_TRACE("Plugin created", Properties("id", this->id), "Plugin", "Plugin");
    this->_initialize();
}

Plugin::~Plugin()
{
    this->_clean();
    delete this->loader;
    this->loader = NULL;
    LOG_TRACE("Plugin destroyed!", Properties("id", this->id), "Plugin", "~Plugin");
}

bool            Plugin::load(bool full)
{
    SmartMutex  mutex(this->mutex, "Plugin", "load");

    if (!mutex)
        return (false);
    if (!this->_load())
    {
        LOG_ERROR("Unable to load the plugin", Properties("id", this->id), "Plugin", "load");
        this->_clean();
        return (false);
    }
    if (full && !this->configuration)
    {
        LOG_ERROR("The plugin must be installed to be loaded", Properties("id", this->id), "Plugin", "load");
        this->_clean();
        return (false);
    }
    if (full && !this->instance->onLoad(this->api))
    {
        LOG_ERROR("The plugin returned false from IPlugin::onLoad, so it will not be loaded", Properties("id", this->id), "Plugin", "load");
        this->_clean();
        return (false);
    }
    this->state = LightBird::IPlugins::LOADED;
    return (true);
}

bool            Plugin::unload(bool full)
{
    SmartMutex  mutex(this->mutex, "Plugin", "unload");

    if (!mutex)
        return (false);
    if (this->state == LightBird::IPlugins::UNLOADED)
        return (false);
    if (this->state == LightBird::IPlugins::LOADED)
    {
        this->state = LightBird::IPlugins::UNLOADING;
        if (full)
            this->instance->onUnload();
    }
    if (this->used == 0)
        this->_unload();
    else
        LOG_DEBUG("The plugin will be unloaded later because it is still used", Properties("id", this->id).add("used", QString::number(this->used)), "Plugins", "unload");
    return (true);
}

bool            Plugin::install()
{
    SmartMutex  mutex(this->mutex, "Plugin", "install");

    if (!mutex)
        return (false);
    if (this->state != LightBird::IPlugins::LOADED)
    {
        LOG_ERROR("The plugin must be loaded to be installed", Properties("id", this->id), "Plugin", "install");
        return (false);
    }
    if (this->configuration)
    {
        LOG_ERROR("The plugin is already installed", Properties("id", this->id), "Plugin", "install");
        return (false);
    }
    if (!this->_createConfiguration())
    {
        LOG_ERROR("Unable to create the configuration of the plugin", Properties("id", this->id), "Plugin", "install");
        return (false);
    }
    this->configuration = Configurations::instance(this->id);
    this->_loadApi();
    if (this->instance->onInstall(this->api) == false)
    {
        LOG_ERROR("The plugin returned false from IPlugin::onInstall(), so it will not be installed", Properties("id", this->id), "Plugin", "install");
        this->_removeConfiguration();
        return (false);
    }
    return (true);
}

bool            Plugin::uninstall()
{
    SmartMutex  mutex(this->mutex, "Plugin", "uninstall");

    if (!mutex)
        return (false);
    if (this->state != LightBird::IPlugins::LOADED)
    {
        LOG_ERROR("The plugin must be loaded to be uninstalled", Properties("id", this->id), "Plugin", "uninstall");
        return (false);
    }
    if (!this->configuration)
    {
        LOG_ERROR("The plugin is already uninstalled", Properties("id", this->id), "Plugin", "uninstall");
        return (false);
    }
    this->instance->onUninstall(this->api);
    this->_removeConfiguration();
    return (true);
}

bool            Plugin::release()
{
    SmartMutex  mutex(this->mutex, "Plugin", "release");

    if (!mutex)
        return (false);
    if (this->state == LightBird::IPlugins::UNLOADED)
    {
        LOG_WARNING("The plugin is already unloaded", Properties("id", this->id), "Plugin", "release");
        return (false);
    }
    this->used--;
    if (this->used < 0)
    {
        LOG_WARNING("Used plugin is lesser than 0", Properties("id", this->id), "Plugin", "release");
        this->used = 0;
    }
    if (this->used == 0 && this->state == LightBird::IPlugins::UNLOADING)
        this->_unload();
    return (true);
}

LightBird::IMetadata     Plugin::getMetadata() const
{
    LightBird::IMetadata metadata;
    SmartMutex           mutex(this->mutex, SmartMutex::READ, "Plugin", "getMetadata");

    if (!mutex)
        return (metadata);
    this->instance->getMetadata(metadata);
    return (metadata);
}

bool    Plugin::checkContext(const QString &mode, const QString &transport, const QStringList &protocols,
                             unsigned short port, const QString &method, const QString &type, bool all) const
{
    QListIterator<Context>  it(this->contexts);

    // Iterate through all the contexts of the plugin
    while (it.hasNext())
    {
        // If at least one context is valid, true is returned
        if ((!all && it.next().isValid(mode, transport, protocols, port)) ||
            (all && it.next().isValid(mode, transport, protocols, port, method, type)))
        {
            /*LOG_TRACE("Context valid", Properties(it.peekPrevious().toMap()).add("id", this->id)
                      .add("requiredmode", mode).add("requiredTransport", transport)
                      .add("requiredProtocols", protocols.join(" "))
                      .add("requiredPort", port).add("requiredMethod", method, false)
                      .add("requiredType", type, false).add("all", all), "Plugin", "checkContext");*/
            return (true);
        }
        /*else
            LOG_TRACE("Context invalid", Properties(it.peekPrevious().toMap()).add("id", this->id)
                      .add("requiredmode", mode).add("requiredTransport", transport)
                      .add("requiredProtocols", protocols.join(" "))
                      .add("requiredPort", port).add("requiredMethod", method, false)
                      .add("requiredType", type, false).add("all", all), "Plugin", "checkContext");*/
    }
    return (false);
}

LightBird::IPlugins::State      Plugin::getState() const
{
    LightBird::IPlugins::State  state;
    SmartMutex                  mutex(this->mutex, SmartMutex::READ, "Plugin", "getState");

    if (!mutex)
        return (LightBird::IPlugins::UNKNOW);
    state = this->state;
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
    this->_loadLibrary();
    this->_loadDefaultConfiguration();
    Configurations::instance(this->id);
}

bool                    Plugin::_loadLibrary()
{
    LightBird::IPlugin  *instance;

    // Iterate over the files of the plugin directory
    QStringListIterator dir(QDir(this->path).entryList(Plugins::getLibraryExtensions(), QDir::Files));
    while (dir.hasNext() && !this->loader)
    {
        // If the file name has the good extension
        if (QLibrary::isLibrary(dir.peekNext()))
        {
            this->loader = new QPluginLoader(this->path + dir.peekNext());
            this->libraryName = dir.peekNext();
            // If the plugin implements IPlugin
            if ((instance = qobject_cast<LightBird::IPlugin *>(loader->instance())) != NULL &&
                loader->isLoaded() == true)
                delete instance;
            // The library is not a valid plugin
            else
            {
                if (!loader->isLoaded())
                    Log::debug("The plugin could not be loaded", Properties("id", this->id).add("file", this->path + this->libraryName).add("message", loader->errorString()), "Plugin", "_load");
                else
                    Log::debug("The plugin does not implement LightBird::IPlugin", Properties("id", this->id).add("file", this->path + this->libraryName), "Plugin", "_load");
                delete this->loader;
                this->loader = NULL;
            }
        }
        dir.next();
    }
    if (!this->loader)
    {
        LOG_WARNING("Unable to find a valid plugin library", Properties("id", this->id).add("file", this->path + this->libraryName), "Plugin", "_load");
        return (false);
    }
    LOG_TRACE("Plugin library found", Properties("id", this->id).add("file", this->path + this->libraryName), "Plugin", "_load");
    return (true);
}

void            Plugin::_loadDefaultConfiguration()
{
    QDomElement element;
    bool        create = false;

    element = Configurations::instance()->writeDom().firstChildElement("configurations");
    for (element = element.firstChildElement("plugin"); !element.isNull(); element = element.nextSiblingElement("plugin"))
        if (element.attribute("id") == this->id)
        {
            if (!element.hasChildNodes())
            {
                create = true;
                element.parentNode().removeChild(element);
            }
            break;
        }
    Configurations::instance()->release();
    if (create && !this->_createConfiguration())
    {
        element = Configurations::instance()->writeDom().firstChildElement("configurations");
        element.appendChild(element.ownerDocument().createElement("plugin")).toElement().setAttribute("id", this->id);
        Configurations::instance()->release();
    }
}

bool    Plugin::_load()
{
    if (!this->loader)
    {
        LOG_DEBUG("Failed to load the plugin", Properties("id", this->id), "Plugin", "_load");
        return (false);
    }
    this->instanceObject = this->loader->instance();
    if (!(this->instance = qobject_cast<LightBird::IPlugin *>(this->instanceObject)))
    {
        LOG_DEBUG("The plugin does not implement IPlugin", Properties("id", this->id), "Plugin", "_load");
        return (false);
    }
    if (Plugins::isInstalled(this->id))
        this->configuration = Configurations::instance(this->id);
    this->_loadApi();
    return (true);
}

void    Plugin::_loadApi()
{
    // The api is loaded only if the configuration is available (i.e if the plugin is installed)
    if (this->configuration)
    {
        bool event = qobject_cast<LightBird::IEvent *>(this->instanceObject) != NULL;
        bool timers = qobject_cast<LightBird::ITimer *>(this->instanceObject) != NULL;
        this->api = new Api(this->id, *this->configuration, event, timers);
        this->_loadContexts();
        this->_loadResources();
    }
}

void                        Plugin::_loadContexts()
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
                    if (nodeName == "mode")
                    {
                        if (nodeValue != "client" && nodeValue != "server")
                            nodeValue.clear();
                        context.setMode(nodeValue);
                    }
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
                LOG_DEBUG("Context added", Properties(context.toMap()).add("id", this->id), "Plugin", "_loadInformations");
                this->contexts.push_back(context);
            }
        }
        dom = dom.nextSibling();
    }
    this->configuration->release();
}

void            Plugin::_loadResources()
{
    QDomNode    read;
    QDomNode    dom;
    QString     nodeName;
    QString     nodeValue;
    QString     path;
    QString     alias;
    QString     resourcesPath;

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
            nodeValue = LightBird::cleanPath(this->path + nodeValue);
            alias = dom.toElement().attribute("alias");
            // Copy the file
            if (nodeName == "resource" && !alias.isEmpty() && !QFileInfo(nodeValue).isFile())
            {
                // Creates the directory of the resource if it doesn't exists
                path = nodeValue.left(nodeValue.lastIndexOf('/'));
                if (!QFileInfo(path).isDir())
                    QDir().mkpath(path);
                // Copy the resource
                nodeName = resourcesPath + "/" + dom.toElement().attribute("alias");
                LOG_TRACE("Copying the resource of the plugin to the file system", Properties("id", this->id)
                          .add("file", nodeValue).add("resource", nodeName), "Plugin", "_loadInformations");
                if (!LightBird::copy(nodeName, nodeValue))
                    LOG_WARNING("Unable to copy the plugin resource to the file system", Properties("id", this->id)
                                .add("file", nodeValue).add("resource", nodeName), "Plugin", "_loadInformations");
            }
            // Copy all the files in the resources of the plugin
            else if (nodeName == "resource" && alias.isEmpty())
                this->_copyAllResources(resourcesPath, nodeValue);
        }
        dom = dom.nextSibling();
    }
    this->configuration->release();
}

void            Plugin::_copyAllResources(const QString &resourcesPath, const QString &destDir, QString currentDir)
{
    QStringList files;
    QString     source;
    QString     destination;

    // Copy all the files in the current directory
    files = QDir(resourcesPath + "/" + currentDir).entryList(QDir::Files | QDir::NoDotAndDotDot);
    if (currentDir.isEmpty())
    {
        files.removeAll("configuration");
        files.removeAll("queries");
    }
    QStringListIterator f(files);
    while (f.hasNext())
    {
        source = resourcesPath + currentDir + "/" + f.peekNext();
        destination = destDir + currentDir + "/" + f.peekNext();
        if (!QFileInfo(destination).isFile())
        {
            // Create the directory of the resource if it doesn't exists
            if (!QFileInfo(destDir + "/" + currentDir).isDir())
                QDir().mkpath(destDir + "/" + currentDir);
            // Copy the resource
            LOG_TRACE("Copying the resource of the plugin to the file system", Properties("id", this->id)
                      .add("file", destination).add("resource", source), "Plugin", "_copyAllResources");
            if (!LightBird::copy(source, destination))
                LOG_WARNING("Unable to copy the plugin resource to the file system", Properties("id", this->id)
                            .add("file", destination).add("resource", source), "Plugin", "_copyAllResources");
        }
        f.next();
    }
    // Run recursively through all the directories of the current folder
    QStringListIterator d(QDir(resourcesPath + "/" + currentDir).entryList(QDir::Dirs | QDir::NoDotAndDotDot));
    while (d.hasNext())
        this->_copyAllResources(resourcesPath, destDir, currentDir + "/" + d.next());
}

void    Plugin::_unload()
{
    this->_clean();
    this->state = LightBird::IPlugins::UNLOADED;
}

bool                Plugin::_createConfiguration()
{
    QDomDocument    doc;
    QDomElement     element;
    QString         errorMsg;
    int             errorLine;
    int             errorColumn;

    // Create the plugin configuration from its resource if it doesn't exists
    if (!Plugins::isInstalled(this->id))
    {
        element = Configurations::instance()->writeDom().firstChildElement("configurations");
        // Copy the default configuration in the resource of the plugin into the configuration of the server
        QFile file(Plugins::getResourcesPath(this->id) + "/configuration");
        if (file.exists())
        {
            // Try to parse the default XML configuration of the plugin, from its resources
            if (!doc.setContent(&file, false, &errorMsg, &errorLine, &errorColumn))
            {
                LOG_ERROR("An error occured while parsing the configuration file of a plugin", Properties("message", errorMsg).add("file", file.fileName())
                          .add("line", QString::number(errorLine)).add("column", errorColumn).add("id", this->id), "Plugin", "_createConfigurations");
                Configurations::instance()->release();
                return (false);
            }
            // Add its configuration into the configuration of the server
            QDomElement plugin = element.ownerDocument().importNode(doc.documentElement(), true).toElement();
            plugin.setTagName("plugin");
            plugin.setAttribute("id", this->id);
            element.appendChild(plugin);
        }
        // If the default configuration doesn't exists, just create the node of the plugin
        else
        {
            LOG_DEBUG("The configuration of the plugin doesn't exists in its resources", Properties("id", this->id), "Plugin", "_createConfigurations");
            QDomElement plugin = element.ownerDocument().createElement("plugin");
            plugin.setAttribute("id", this->id);
            element.appendChild(plugin);
        }
        Configurations::instance()->release();
        // Save the changes
        Configurations::instance()->save();
    }
    return (true);
}

void            Plugin::_removeConfiguration()
{
    QDomElement element;

    // Search the configuration node of the plugin
    element = Configurations::instance()->writeDom().firstChildElement("configurations");
    for (element = element.firstChildElement("plugin"); !element.isNull() && element.attribute("id") != this->id; element = element.nextSiblingElement("plugin"))
        ;
    // Removes it
    if (!element.isNull())
    {
        element.parentNode().removeChild(element);
        Configurations::instance()->release();
        Configurations::instance()->save();
    }
    else
        Configurations::instance()->release();
    // The plugin is unloaded to avoid problems (the api needs a valid configuration)
    this->_clean();
}

void    Plugin::_clean()
{
    this->configuration = NULL;
    delete this->instance;
    this->instance = NULL;
    this->instanceObject = NULL;
    delete this->api;
    this->api = NULL;
}
