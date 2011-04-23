#include <QtPlugin>

#include "Get.h"

Get::Get()
{
}

Get::~Get()
{
}

bool    Get::onLoad(LightBird::IApi *api)
{
    this->api = api;
    return (true);
}

void    Get::onUnload()
{
}

bool    Get::onInstall(LightBird::IApi *api)
{
    this->api = api;
    return (true);
}

void    Get::onUninstall(LightBird::IApi *api)
{
    this->api = api;
}

void    Get::getMetadata(LightBird::IMetadata &metadata) const
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

bool    Get::doExecution(LightBird::IClient &client)
{
    LightBird::IMetadata metadata;

    metadata = this->api->plugins().getMetadata(this->api->getId());
    QString s = metadata.name;
    client.getResponse().getContent().setContent(s.toAscii());
    return (true);
}

Q_EXPORT_PLUGIN2(Get, Get)
