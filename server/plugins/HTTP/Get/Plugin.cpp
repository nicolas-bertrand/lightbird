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

bool    Plugin::onLoad(LightBird::IApi *api)
{
    this->api = api;
    api->plugins().getMetadata(api->getId());
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
    metadata.name = "HTTP Get";
    metadata.brief = "Implements the GET HTTP method.";
    metadata.description = "Allows clients to download files from the server, and get data of its files.";
    metadata.autor = "LightBird team";
    metadata.site = "lightbird.cc";
    metadata.email = "team@lightbird.cc";
    metadata.version = "1.0";
    metadata.licence = "LGPL";
}

bool    Plugin::doExecution(LightBird::IClient &client)
{
    LightBird::IMetadata metadata;

    metadata = this->api->plugins().getMetadata(this->api->getId());
    QString s = metadata.name;
    client.getResponse().getContent().setContent(s.toAscii());
    return (true);
}

Q_EXPORT_PLUGIN2(plugin, Plugin)
