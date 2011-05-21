#ifndef API_H
# define API_H

# include <QObject>
# include <QString>

# include "IApi.h"

# include "ApiDatabase.h"
# include "ApiEvents.h"
# include "ApiLogs.h"
# include "ApiNetwork.h"
# include "ApiTimers.h"

/// @brief The server implementation of the IApi interface.
class Api : public QObject,
            public LightBird::IApi
{
public:
    /// @param id : The id of the plugin.
    /// @param configuration : Its configuration.
    /// @param event : If the plugin implements IEvent.
    /// @param timers : If the timers has to be loaded.
    Api(const QString &id, LightBird::IConfiguration &configuration, bool event, bool timers, QObject *parent = 0);
    ~Api();

    LightBird::IConfiguration   &configuration(bool plugin = false);
    LightBird::IConfiguration   *configuration(const QString &path, const QString &alternative = "");
    LightBird::IDatabase        &database();
    LightBird::IEvents          &events();
    LightBird::IExtensions      &extensions();
    LightBird::IGuis            &guis();
    LightBird::ILogs            &log();
    LightBird::INetwork         &network();
    LightBird::IPlugins         &plugins();
    LightBird::ITimers          &timers();
    void                        stop();
    void                        restart();
    const QString               &getId();
    QString                     getServerVersion();
    const QString               &getPluginPath();

private:
    Api();
    Api(const Api &);
    Api &operator=(const Api &);

    QString                     id;                 ///< The id of the plugin for which the object has been created.
    QString                     pluginPath;         ///< The path to the plugin that own this object.
    LightBird::IConfiguration   &configurationApi;  ///< The instance of the configuration api.
    ApiDatabase                 databaseApi;        ///< The instance of the database api.
    ApiEvents                   *eventsApi;         ///< The instance of the events api.
    ApiLogs                     logsApi;            ///< The instance of the logs api.
    ApiNetwork                  networkApi;         ///< The instance of the network api.
    ApiTimers                   *timersApi;         ///< The instance of the timer api.
};

#endif // API_H
