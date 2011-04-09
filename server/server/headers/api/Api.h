#ifndef API_H
# define API_H

# include <QObject>

# include "IApi.h"

# include "ApiLogs.h"
# include "ApiDatabase.h"
# include "Configurations.h"
# include "ApiNetwork.h"
# include "ApiGuis.h"
# include "ApiTimers.h"

class Api : public QObject,
            public LightBird::IApi
{
public:
    /// @param id : The id of the plugin.
    /// @param configuration : Its configuration.
    /// @param timers : If the timers has to be loaded.
    Api(const QString &id, Configuration *configuration, bool timers = false, QObject *parent = 0);
    ~Api();

    /// @see LightBird::IApi::log
    LightBird::ILogs            &log();
    /// @see LightBird::IApi::database
    LightBird::IDatabase        &database();
    /// @see LightBird::IApi::configuration
    LightBird::IConfiguration   &configuration(bool plugin = false);
    /// @see LightBird::IApi::configuration
    LightBird::IConfiguration   *configuration(const QString &path, const QString &alternative = "");
    /// @see LightBird::IApi::plugins
    LightBird::IPlugins         &plugins();
    /// @see LightBird::IApi::network
    LightBird::INetwork         &network();
    /// @see LightBird::IApi::guis
    LightBird::IGuis            &guis();
    /// @see LightBird::IApi::extensions
    LightBird::IExtensions      &extensions();
    /// @see LightBird::IApi::timers
    LightBird::ITimers          &timers();
    /// @see LightBird::IApi::stop
    void                        stop();
    /// @see LightBird::IApi::restart
    void                        restart();
    /// @see LightBird::IApi::getId
    const QString               &getId();
    /// @see LightBird::IApi::getServerVersion
    QString                     getServerVersion();
    /// @see LightBird::IApi::getPluginPath
    const QString               &getPluginPath();

private:
    Api();
    Api(const Api &);
    Api &operator=(const Api &);

    QString         id;
    ApiLogs         *logsApi;
    ApiDatabase     *databaseApi;
    Configuration   *configurationApi;
    ApiNetwork      *networkApi;
    ApiTimers       *timersApi;
    QString         pluginPath;
};

#endif // API_H
