#include <QtPlugin>

#include "UnitTests.h"
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
    this->api->events().subscribe(QStringList() << "server_started");
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
    metadata.name = "Test";
    metadata.brief = "Run various tests on the server API.";
    metadata.description = "Allows to test the server API, the network, and the database.";
    metadata.autor = "LightBird team";
    metadata.site = "lightbird.cc";
    metadata.email = "team@lightbird.cc";
    metadata.version = "1.0";
    metadata.licence = "CC BY-NC-SA 3.0";
}

void    Plugin::event(const QString &event, const QVariant &)
{
    // Launch the unit tests
    if (event == "server_started")
        UnitTests(*this->api);
}

Q_EXPORT_PLUGIN2(Plugin, Plugin)
