#include <QtPlugin>
// INT64_C and UINT64_C are not defined in C++
#define INT64_C(c) (c ## LL)
#define UINT64_C(c) (c ## ULL)
// FFmpeg is a C library
extern "C"
{
    #include <libavformat/avformat.h>
}

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
    av_register_all();
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
    metadata.name = "FFmpeg";
    metadata.brief = "Implements the IVideo and IAudio extensions using FFmpeg.";
    metadata.description = "Implements the IVideo and IAudio extensions using FFmpeg.";
    metadata.autor = "LightBird team";
    metadata.site = "lightbird.cc";
    metadata.email = "team@lightbird.cc";
    metadata.version = "1.0";
    metadata.licence = "CC BY-NC-SA 3.0";
}

QStringList Plugin::getExtensionsNames()
{
    return (QStringList());
}

void        *Plugin::getExtension(const QString &name)
{
    return (NULL);
}

void        Plugin::releaseExtension(const QString &, void *)
{
}

Q_EXPORT_PLUGIN2(plugin, Plugin)
