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
    return (":plugins/HTTP/Get");
}

bool    Plugin::doExecution(Streamit::IClient &client)
{
    client.getResponse().getContent().setContent("Hello world!");
    return (true);
}

Q_EXPORT_PLUGIN2(plugin, Plugin)
