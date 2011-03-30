#include <QCoreApplication>
#include "Plugins.hpp"
#include "Api.h"
#include "Log.h"
#include "Extensions.h"
#include "ApiPlugins.h"

Api::Api(const QString &id, Configuration *configuration, bool timers, QObject *parent) : QObject(parent)
{
    this->id = id;
    this->logsApi = new ApiLogs(this->id, this);
    this->databaseApi = new ApiDatabase(this->id, this);
    this->configurationApi = configuration;
    this->networkApi = new ApiNetwork(this->id, this);
    // If the timers has to be loaded
    if (timers)
        this->timersApi = new ApiTimers(this->id, this);
    // Set the plugin path
    this->pluginPath = Configurations::instance()->get("pluginsPath") + "/" + this->id + "/";
}

Api::~Api()
{
    Log::trace("Api destroyed!", Properties("id", this->id), "Api", "~Api");
}

Streamit::ILogs             &Api::log()
{
    return (*this->logsApi);
}

Streamit::IDatabase         &Api::database()
{
    return (*this->databaseApi);
}

Streamit::IConfiguration    &Api::configuration(bool plugin)
{
    Configuration           *configuration;

    if (plugin)
        configuration = this->configurationApi;
    else
        configuration = Configurations::instance();
    return (*configuration);
}

Streamit::IConfiguration    *Api::configuration(const QString &path, const QString &alternative)
{
    return (Configurations::instance(path, alternative));
}

Streamit::IPlugins          &Api::plugins()
{
    return (*ApiPlugins::instance());
}

Streamit::INetwork          &Api::network()
{
    return (*this->networkApi);
}

Streamit::IGuis             &Api::guis()
{
    return (*ApiGuis::instance());
}

Streamit::IExtensions       &Api::extensions()
{
    return (*Extensions::instance());
}

Streamit::ITimers           &Api::timers()
{
    return (*this->timersApi);
}

void                        Api::stop()
{
    QCoreApplication::instance()->quit();
}

void                        Api::restart()
{
    QCoreApplication::instance()->quit();
}

const QString               &Api::getId()
{
    return (this->id);
}

QString                     Api::getServerVersion()
{
    return (VERSION);
}

const QString               &Api::getPluginPath()
{
    return (this->pluginPath);
}
