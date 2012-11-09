#include <QtPlugin>

#include "Plugin.h"

Plugin::Plugin()
{
    this->identify = NULL;
}

Plugin::~Plugin()
{
    delete this->identify;
    this->identify = NULL;
}

bool    Plugin::onLoad(LightBird::IApi *api)
{
    this->api = api;
    this->identify = new Identify(this->api);
    return (*this->identify);
}

void    Plugin::onUnload()
{
    delete this->identify;
    this->identify = NULL;
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
    metadata.name = "ImageMagick";
    metadata.brief = "This extension allows plugins to manage images using ImageMagick.";
    metadata.description = "Allows plugins to manipulate images via the ImageMagick library.";
    metadata.autor = "LightBird team";
    metadata.site = "lightbird.cc";
    metadata.email = "team@lightbird.cc";
    metadata.version = "1.0";
    metadata.licence = "CC BY-NC-SA 3.0";
}

QStringList Plugin::getExtensionsNames()
{
    return (QStringList() << "IIdentify");
}

void    *Plugin::getExtension(const QString &name)
{
    if (name == "IIdentify")
        return (dynamic_cast<LightBird::IIdentify *>(this->identify));
    return (NULL);
}

void    Plugin::releaseExtension(const QString &, void *)
{
}

Q_EXPORT_PLUGIN2(plugin, Plugin)
