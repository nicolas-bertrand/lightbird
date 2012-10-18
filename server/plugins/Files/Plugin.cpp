#include <QtPlugin>

#include "Plugin.h"

Plugin::Plugin()
{
    this->files = NULL;
}

Plugin::~Plugin()
{
    delete this->files;
}

bool    Plugin::onLoad(LightBird::IApi *api)
{
    this->api = api;
    this->api->timers().setTimer("thread");
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
    metadata.name = "Files path manager";
    metadata.brief = "Manages the directory that stores the files uploaded to the server.";
    metadata.description = "Automatically adds the file added in the filesPath to the database. Removes later the files that couldn't be deleted immediatly.";
    metadata.autor = "LightBird team";
    metadata.site = "lightbird.cc";
    metadata.email = "team@lightbird.cc";
    metadata.version = "1.0";
    metadata.licence = "CC BY-NC-SA 3.0";
}

bool    Plugin::timer(const QString &name)
{
    if (!this->files)
        this->files = new Files(this->api, name);
    this->files->timer();
    return (true);
}

Q_EXPORT_PLUGIN2(Plugin, Plugin)
