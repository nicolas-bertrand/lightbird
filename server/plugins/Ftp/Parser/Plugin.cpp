#include <stddef.h>
#include <cstddef>
#include <QtPlugin>
#include <QDomNode>

#include "Plugin.h"
#include "Parser.h"
#include "DataParser.h"
#include "ControlParser.h"

Plugin::Configuration   Plugin::configuration;

Plugin::Plugin()
{
}

Plugin::~Plugin()
{
}

bool    Plugin::onLoad(LightBird::IApi *api)
{
    this->api = api;
    return (true);
}

void    Plugin::onUnload()
{
}

bool    Plugin::onInstall(LightBird::IApi *api)
{
    this->api = api;
    return (true);
}

void    Plugin::onUninstall(LightBird::IApi *api)
{
    this->api = api;
}

void    Plugin::getMetadata(LightBird::IMetadata &metadata) const
{
    metadata.name = "Parser FTP";
    metadata.brief = "The FTP parser.";
    metadata.description = "Handles and parses FTP requests.";
    metadata.autor = "LightBird team";
    metadata.site = "lightbird.cc";
    metadata.email = "team@lightbird.cc";
    metadata.version = "1.0";
    metadata.licence = "CC BY-NC-SA 3.0";
}

bool     Plugin::onConnect(LightBird::IClient &client)
{
    bool result = true;

    this->mutex.lockForWrite();
    if (client.getProtocols().first() == "FTP")
        this->parsers.insert(client.getId(), new ControlParser(this->api, &client));
    else if (client.getProtocols().first() == "FTP-DATA")
        this->parsers.insert(client.getId(), new DataParser(this->api, &client));
    else
        result = false;
    this->mutex.unlock();
    return (result);
}

void    Plugin::onDestroy(LightBird::IClient &client)
{
    this->mutex.lockForWrite();
    if (this->parsers.contains(client.getId()))
    {    
        delete this->parsers.value(client.getId());
        this->parsers.remove(client.getId());
    }
    this->mutex.unlock();
}

bool     Plugin::doUnserializeContent(LightBird::IClient &client, const QByteArray &data, quint64 &used)
{
    bool result = false;
    
    this->mutex.lockForWrite();
    if (this->parsers.contains(client.getId()))
        result = this->parsers.value(client.getId())->doUnserializeContent(data, used);
    this->mutex.unlock();
    return (result);
}

bool     Plugin::doSerializeContent(LightBird::IClient &client, QByteArray &data)
{
    bool result = false;
    
    this->mutex.lockForWrite();
    if (this->parsers.contains(client.getId()))
        result = this->parsers.value(client.getId())->doSerializeContent(data);
    this->mutex.unlock();
    return (result);
}

bool     Plugin::onExecution(LightBird::IClient &client)
{
    bool result = false;
    
    this->mutex.lockForWrite();
    if (this->parsers.contains(client.getId()))
        result = this->parsers.value(client.getId())->onExecution();
    this->mutex.unlock();
    return (result);
}

bool     Plugin::onSerialize(LightBird::IClient &client, LightBird::IOnSerialize::Serialize type)
{
    bool result = false;
    
    this->mutex.lockForWrite();
    if (this->parsers.contains(client.getId()))
        result = this->parsers.value(client.getId())->onSerialize(type);
    this->mutex.unlock();
    return (result);
}

void    Plugin::onFinish(LightBird::IClient &client)
{
    this->mutex.lockForWrite();
    if (this->parsers.contains(client.getId()))
        this->parsers.value(client.getId())->onFinish();
    this->mutex.unlock();
}

bool     Plugin::onDisconnect(LightBird::IClient &client)
{
    bool result = false;
    
    this->mutex.lockForWrite();
    if (this->parsers.contains(client.getId()))
        result = this->parsers.value(client.getId())->onDisconnect();
    this->mutex.unlock();
    return (result);
}

Plugin::Configuration   &Plugin::getConfiguration()
{
    return (Plugin::configuration);
}

Q_EXPORT_PLUGIN2(Plugin, Plugin)
