#include <QtPlugin>
#include <QFileInfo>
#include <QDir>
#include <QDomNode>

#include "ITableFiles.h"
#include "IMime.h"
#include "Plugin.h"

Plugin::Plugin()
{
}

Plugin::~Plugin()
{
}

bool    Plugin::onLoad(Streamit::IApi *api)
{
    this->api = api;
    api->plugins().getMetadata(api->getId());
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

void    Plugin::getMetadata(Streamit::IMetadata &metadata) const
{
    metadata.name = "HTTP Get";
    metadata.brief = "Implements the GET HTTP method.";
    metadata.description = "Allows clients to download files from the server, and get data of its files.";
    metadata.autor = "LightBird team";
    metadata.site = "lightbird.cc";
    metadata.email = "team@lightbird.cc";
    metadata.version = "1.0";
    metadata.licence = "LGPL";
}

QString Plugin::getResourcesPath()
{
    return (":plugins/HTTP/Get");
}

bool    Plugin::doExecution(Streamit::IClient &client)
{
    Streamit::IMetadata metadata;

    metadata = this->api->plugins().getMetadata(this->api->getId());
    QString s = metadata.name;
    client.getResponse().getContent().setContent(s.toAscii());
    return (true);
}

Q_EXPORT_PLUGIN2(plugin, Plugin)
