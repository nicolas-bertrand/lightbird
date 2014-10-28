#include "Plugin.h"

Plugin::Plugin()
{
    this->identify = NULL;
    this->preview = NULL;
}

Plugin::~Plugin()
{
    delete this->identify;
    this->identify = NULL;
    delete this->preview;
    this->preview = NULL;
}

bool    Plugin::onLoad(LightBird::IApi *api)
{
    this->api = api;
    this->identify = new Identify(this->api);
    this->preview = new Preview(this->api);
    return (true);
}

void    Plugin::onUnload()
{
    delete this->identify;
    this->identify = NULL;
    delete this->preview;
    this->preview = NULL;
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
    metadata.name = "TagLib";
    metadata.brief = "Extracts metadata from audio files.";
    metadata.description = "This plugin uses TagLib to extract various meta-data (title, album, artist...) from various formats (mp3, mp4, ogg, mpeg...).";
    metadata.autor = "LightBird team";
    metadata.site = "lightbird.cc";
    metadata.email = "team@lightbird.cc";
    metadata.version = "1.0";
    metadata.licence = "CC BY-NC-SA 3.0";
    metadata.dependencies << "TagLib 1.9.1";
}

QStringList Plugin::getExtensionsNames()
{
    return (QStringList() << "IIdentify" << "IPreview");
}

void    *Plugin::getExtension(const QString &name)
{
    if (name == "IIdentify")
        return (qobject_cast<LightBird::IIdentify *>(this->identify));
    else if (name == "IPreview")
        return (qobject_cast<LightBird::IPreview *>(this->preview));
    return (NULL);
}

void    Plugin::releaseExtension(const QString &, void *)
{
}
