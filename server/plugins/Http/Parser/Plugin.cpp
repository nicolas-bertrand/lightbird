#include <QtPlugin>
#include <QDomNode>

#include "ParserClient.h"
#include "ParserServer.h"
#include "Plugin.h"

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
    // Load the configuration
    this->configuration.protocols.push_back("HTTP/1.0");
    this->configuration.protocols.push_back("HTTP/1.1");
    if (!(this->configuration.maxHeaderSize = api->configuration(true).get("maxHeaderSize").toUInt()))
        this->configuration.maxHeaderSize = 1000000;
    if (!(this->configuration.maxPacketSize = api->configuration(true).get("maxPacketSize").toUInt()))
        this->configuration.maxPacketSize = 10000000;
    if (!(this->configuration.maxContentInMemory = api->configuration(true).get("maxContentInMemory").toUInt()))
        this->configuration.maxContentInMemory = 1000000;
    QDomNode node = api->configuration(true).readDom().firstChildElement("methodContent").firstChild();
    while (!node.isNull())
    {
        if (node.isElement())
            this->configuration.methodContent << node.nodeName();
        node = node.nextSibling();
    }
    api->configuration(true).release();
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
    metadata.name = "Parser HTTP";
    metadata.brief = "The HTTP parser.";
    metadata.description = "Parse HTTP requests. Deserialize the request/response and serialize the request/response, depending on the mode of the client.";
    metadata.autor = "LightBird team";
    metadata.site = "lightbird.cc";
    metadata.email = "team@lightbird.cc";
    metadata.version = "1.0";
    metadata.licence = "CC BY-NC-SA 3.0";
}

bool    Plugin::onConnect(LightBird::IClient &client)
{
    this->mutex.lockForWrite();
    if (client.getMode() == LightBird::IClient::CLIENT)
        this->parsers.insert(client.getId(), new ParserClient(client));
    else
        this->parsers.insert(client.getId(), new ParserServer(client));
    this->mutex.unlock();
    return (true);
}

bool    Plugin::onDisconnect(LightBird::IClient &client)
{
    this->mutex.lockForWrite();
    delete this->parsers.value(client.getId());
    this->parsers.remove(client.getId());
    this->mutex.unlock();
    return (true);
}

bool    Plugin::onProtocol(LightBird::IClient &client, const QByteArray &data, QString &protocol, bool &error)
{
    return (this->_getParser(client)->onProtocol(data, protocol, error));
}

bool    Plugin::doDeserializeHeader(LightBird::IClient &client, const QByteArray &data, quint64 &used)
{
    return (this->_getParser(client)->doDeserializeHeader(data, used));
}

bool    Plugin::doDeserializeContent(LightBird::IClient &client, const QByteArray &data, quint64 &used)
{
    return (this->_getParser(client)->doDeserializeContent(data, used));
}

void    Plugin::doSerializeHeader(LightBird::IClient &client, QByteArray &data)
{
    this->_getParser(client)->doSerializeHeader(data);
}

bool    Plugin::doSerializeContent(LightBird::IClient &client, QByteArray &data)
{
    return (this->_getParser(client)->doSerializeContent(data));
}

void    Plugin::onFinish(LightBird::IClient &client)
{
    if (client.getRequest().isError() || client.getResponse().isError())
        this->api->network().disconnect(client.getId());
}

Plugin::Configuration   &Plugin::getConfiguration()
{
    return (Plugin::configuration);
}

Parser      *Plugin::_getParser(const LightBird::IClient &client)
{
    Parser  *parser;

    this->mutex.lockForRead();
    parser = this->parsers[client.getId()];
    this->mutex.unlock();
    return (parser);
}

Q_EXPORT_PLUGIN2(Plugin, Plugin)
