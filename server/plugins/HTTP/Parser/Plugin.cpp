#include <QtPlugin>
#include <QDomNode>

#include "Plugin.h"

Plugin::Configuration   Plugin::configuration;

Plugin::Plugin()
{
}

Plugin::~Plugin()
{
}

bool    Plugin::onLoad(Streamit::IApi *api)
{
    this->api = api;
    // Load the configuration
    this->configuration.protocols.push_back("SiTP/1.0");
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
    return (true);
}

void    Plugin::onUnload()
{
}

bool    Plugin::onInstall(Streamit::IApi *api)
{
    this->api = api;
    return (true);
}

void    Plugin::onUninstall(Streamit::IApi *api)
{
    this->api = api;
}

QString Plugin::getResourcesPath()
{
    return (":plugins/HTTP/Parser");
}

bool    Plugin::onConnect(Streamit::IClient &client)
{
    this->mutex.lockForWrite();
    this->parsers.insert(client.getId(), Parser(&client));
    this->mutex.unlock();
    return (true);
}

void    Plugin::onDisconnect(Streamit::IClient &client)
{
    this->mutex.lockForWrite();
    this->parsers.remove(client.getId());
    this->mutex.unlock();
}

bool    Plugin::onProtocol(Streamit::IClient &client, const QByteArray &data, QString &protocol, bool &error)
{
    return (this->_getParser(client)->onProtocol(data, protocol, error));
}

bool    Plugin::doUnserializeHeader(Streamit::IClient &client, const QByteArray &data, quint64 &used)
{
    return (this->_getParser(client)->doUnserializeHeader(data, used));
}

bool    Plugin::doUnserializeContent(Streamit::IClient &client, const QByteArray &data, quint64 &used)
{
    return (this->_getParser(client)->doUnserializeContent(data, used));
}

void    Plugin::doSerializeHeader(Streamit::IClient &client, QByteArray &data)
{
    this->_getParser(client)->doSerializeHeader(data);
}

bool    Plugin::doSerializeContent(Streamit::IClient &client, QByteArray &data)
{
    return (this->_getParser(client)->doSerializeContent(data));
}

void    Plugin::onWrote(Streamit::IClient &client)
{
    if (client.getRequest().isError())
        this->api->network().disconnect(client.getId());
}

Plugin::Configuration   &Plugin::getConfiguration()
{
    return (Plugin::configuration);
}

Parser      *Plugin::_getParser(const Streamit::IClient &client)
{
    Parser  *parser;
    this->mutex.lockForRead();
    parser = &this->parsers[client.getId()];
    this->mutex.unlock();
    return (parser);
}

Q_EXPORT_PLUGIN2(plugin, Plugin)
