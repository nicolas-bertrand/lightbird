#include <QtPlugin>

#include "Plugin.h"

Plugin::Plugin()
{
    this->identifier = NULL;
}

Plugin::~Plugin()
{
    delete this->identifier;
    this->identifier = NULL;
}

bool    Plugin::onLoad(LightBird::IApi *api)
{
    this->api = api;
    this->identifier = new Identifier(*this->api);
    return (true);
}

void    Plugin::onUnload()
{
    delete this->identifier;
    this->identifier = NULL;
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
    metadata.name = "Identifier";
    metadata.brief = "This extension allows plugins to identify a file by getting all its information.";
    metadata.description = "This plugin implements the IIdentifier and IMime interfaces in order to return information on a file.";
    metadata.autor = "LightBird team";
    metadata.site = "lightbird.cc";
    metadata.email = "team@lightbird.cc";
    metadata.version = "1.0";
    metadata.licence = "CC BY-NC-SA 3.0";
}

QStringList Plugin::getExtensionsNames()
{
    return (QStringList() << "IIdentifier" << "IMime");
}

void        *Plugin::getExtension(const QString &name)
{
    if (name == "IIdentifier")
        return (dynamic_cast<LightBird::IIdentifier *>(this->identifier));
    else if (name == "IMime")
        return (dynamic_cast<LightBird::IMime *>(this->identifier));
    return (NULL);
}

void        Plugin::releaseExtension(const QString &, void *)
{
}

Q_EXPORT_PLUGIN2(plugin, Plugin)
