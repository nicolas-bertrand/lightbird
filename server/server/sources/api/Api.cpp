#include <QCoreApplication>

#include "Api.h"
#include "ApiGuis.h"
#include "ApiPlugins.h"
#include "ApiSessions.h"
#include "Configurations.h"
#include "Extensions.h"
#include "Log.h"
#include "Plugins.hpp"
#include "Server.h"

Api::Api(const QString &id, LightBird::IConfiguration &configuration, bool event, bool timers, QObject *parent) :
         QObject(parent),
         configurationApi(configuration),
         databaseApi(id),
         logsApi(id),
         networkApi(id)
{
    this->id = id;
    this->eventsApi = new ApiEvents(this->id, event);
    // If the timers has to be loaded
    if (timers)
        this->timersApi = new ApiTimers(this->id, this);
    // Set the plugin path
    this->pluginPath = Configurations::instance()->get("pluginsPath") + "/" + this->id + "/";
    this->resourcesPath = QString(PLUGINS_RESOURCES_PATH) + "/" + this->id;
}

Api::~Api()
{
    // If the thread has not been started we can delete it safely
    if (!this->eventsApi->isRunning() && !this->eventsApi->isFinished())
        delete this->eventsApi;
    // Otherwise we rely on the Threads manager to delete it
    else
        this->eventsApi->quit();
}

LightBird::IConfiguration   &Api::configuration(bool plugin)
{
    LightBird::IConfiguration *configuration;

    if (plugin)
        configuration = &this->configurationApi;
    else
        configuration = Configurations::instance();
    return (*configuration);
}

LightBird::IConfiguration   *Api::configuration(const QString &path, const QString &alternative)
{
    return (Configurations::instance(path, alternative));
}

LightBird::IDatabase        &Api::database()
{
    return (this->databaseApi);
}

LightBird::IEvents          &Api::events()
{
    return (*this->eventsApi);
}

LightBird::IExtensions      &Api::extensions()
{
    return (*Extensions::instance());
}

LightBird::IGuis            &Api::guis()
{
    return (*ApiGuis::instance());
}

LightBird::ILogs            &Api::log()
{
    return (this->logsApi);
}

LightBird::INetwork         &Api::network()
{
    return (this->networkApi);
}

LightBird::IPlugins         &Api::plugins()
{
    return (*ApiPlugins::instance());
}

LightBird::ISessions        &Api::sessions()
{
    return (*ApiSessions::instance());
}

LightBird::ITimers          &Api::timers()
{
    return (*this->timersApi);
}

void                        Api::stop()
{
    Server::instance().stop(false);
}

void                        Api::restart()
{
    Server::instance().stop(true);
}

const QString               &Api::getId() const
{
    return (this->id);
}

const QString               &Api::getPluginPath() const
{
    return (this->pluginPath);
}

const QString               &Api::getResourcesPath() const
{
    return (this->resourcesPath);
}

QString                     Api::getServerVersion() const
{
    return (VERSION);
}

QString                     Api::getLanguage() const
{
    QString language = Configurations::instance()->get("language");

    if (language.isEmpty())
        language = QLocale::system().name().section('_', 0, 0);
    if (language.isEmpty())
        language = "en";
    return (language);
}
