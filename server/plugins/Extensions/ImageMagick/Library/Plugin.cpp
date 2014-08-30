#include "Plugin.h"

Plugin::Plugin()
{
    this->identify = NULL;
    this->image = NULL;
}

Plugin::~Plugin()
{
    delete this->identify;
    this->identify = NULL;
    delete this->image;
    this->image = NULL;
}

bool    Plugin::onLoad(LightBird::IApi *api)
{
    this->api = api;
    this->identify = new Identify(this->api);
    this->image = new Image(this->api);
    return (*this->identify && *this->image);
}

void    Plugin::onUnload()
{
    delete this->identify;
    this->identify = NULL;
    delete this->image;
    this->image = NULL;
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
    metadata.name = "ImageMagick/Library";
    metadata.brief = "Manage images using the ImageMagick libraries.";
    metadata.description = "Allows plugins to manipulate images via the ImageMagick library.";
    metadata.autor = "LightBird team";
    metadata.site = "lightbird.cc";
    metadata.email = "team@lightbird.cc";
    metadata.version = "1.0";
    metadata.licence = "CC BY-NC-SA 3.0";
    metadata.dependencies << "ImageMagick 6.8.9";
}

QStringList Plugin::getExtensionsNames()
{
    return (QStringList() << "IIdentify" << "IImage" << "IPreview");
}

void    *Plugin::getExtension(const QString &name)
{
    if (name == "IIdentify")
        return (dynamic_cast<LightBird::IIdentify *>(this->identify));
    else if (name == "IImage")
        return (dynamic_cast<LightBird::IImage *>(this->image));
    else if (name == "IPreview")
        return (dynamic_cast<LightBird::IPreview *>(this->image));
    return (NULL);
}

void    Plugin::releaseExtension(const QString &, void *)
{
}
