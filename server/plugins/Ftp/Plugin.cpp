#include <stddef.h>
#include <cstddef>
#include <QtPlugin>
#include <QDomNode>

#include "Plugin.h"
#include "ParserData.h"
#include "ParserControl.h"

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
    this->handler = new ClientHandler(api);
    // Fills the configuration
    if (!(this->configuration.maxPacketSize = api->configuration(true).get("maxPacketSize").toUInt()))
        this->configuration.maxPacketSize = 10000000;
    // Gets the data connection protocol name
    QDomElement element = this->api->configuration(true).readDom();
    element = element.firstChildElement("contexts").firstChildElement("context").firstChildElement("protocol");
    while (!element.isNull())
    {
        if (element.text() != "FTP")
            break;
        element = element.nextSiblingElement("protocol");
    }
    if ((this->configuration.dataProtocolName = element.text()).isEmpty())
        this->configuration.dataProtocolName = "FTP-DATA";
    this->api->configuration(true).release();
    // Opens the data connection port if necessary
    QStringList protocols;
    unsigned int maxClients;
    this->configuration.passivePort = this->api->configuration(true).get("passive").toUShort();
    if (this->configuration.passivePort && !this->api->network().getPort(this->configuration.passivePort, protocols, maxClients))
        this->api->network().openPort(this->configuration.passivePort, QStringList(this->configuration.dataProtocolName));
    return (true);
}

void    Plugin::onUnload()
{
    delete this->handler;
    this->handler = NULL;
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
    metadata.name = "FTP";
    metadata.brief = "The FTP server.";
    metadata.description = "A minimalist FTP server.";
    metadata.autor = "LightBird team";
    metadata.site = "lightbird.cc";
    metadata.email = "team@lightbird.cc";
    metadata.version = "1.0";
    metadata.licence = "CC BY-NC-SA 3.0";
}

bool        Plugin::onConnect(LightBird::IClient &client)
{
    bool    result = true;
    Parser  *parser = NULL;

    if (client.getProtocols().first() == "FTP")
    {
        parser = new ParserControl(this->api, client);
        result = this->handler->onConnect(client);
    }
    else if (client.getProtocols().first() == this->configuration.dataProtocolName)
    {
        parser = new ParserData(this->api, client);
        result = this->handler->onDataConnect(client);
    }
    else
        result = false;
    this->mutex.lockForWrite();
    if (parser)
        this->parsers.insert(client.getId(), parser);
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
    return (this->_getParser(client)->doUnserializeContent(data, used));
}

bool     Plugin::doSerializeContent(LightBird::IClient &client, QByteArray &data)
{
    return (this->_getParser(client)->doSerializeContent(data));
}

bool     Plugin::doExecution(LightBird::IClient &client)
{
    bool result = false;

    if (client.getRequest().getProtocol() == "FTP")
        result = this->handler->doControlExecute(client);
    else if (client.getRequest().getProtocol() == this->configuration.dataProtocolName && client.getMode() == LightBird::IClient::SERVER)
        result = this->handler->doDataExecute(client);
    return (result);
}

bool     Plugin::onExecution(LightBird::IClient &client)
{
    return (this->_getParser(client)->onExecution());
}

bool     Plugin::doSend(LightBird::IClient &client)
{
    bool result = false;

    if (client.getRequest().getProtocol() == this->configuration.dataProtocolName)
        result = this->handler->doDataExecute(client);
    return (result);
}

bool     Plugin::onSerialize(LightBird::IClient &client, LightBird::IOnSerialize::Serialize type)
{
    return (this->_getParser(client)->onSerialize(type));
}

void    Plugin::onFinish(LightBird::IClient &client)
{
    return (this->_getParser(client)->onFinish());
}

bool     Plugin::onDisconnect(LightBird::IClient &client)
{
    if (client.getProtocols().first() == this->configuration.dataProtocolName)
        this->handler->onDataDisconnect(client);
    return (this->_getParser(client)->onDisconnect());
}

Parser      *Plugin::_getParser(const LightBird::IClient &client)
{
    Parser  *parser;

    this->mutex.lockForRead();
    parser = this->parsers[client.getId()];
    this->mutex.unlock();
    return (parser);
}

Plugin::Configuration   &Plugin::getConfiguration()
{
    return (Plugin::configuration);
}

Q_EXPORT_PLUGIN2(Plugin, Plugin)
