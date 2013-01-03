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
    /// @param contexts : The contexts api.
    /// @param event : If the plugin implements IEvent.
    /// @param timers : If the timers has to be loaded.
    Api(const QString &id, LightBird::IConfiguration &configuration, LightBird::IContexts &contexts, bool event, bool timers, QObject *parent = NULL);
    ~Api();

    LightBird::IConfiguration &configuration(bool plugin = false);
    LightBird::IConfiguration *configuration(const QString &path, const QString &alternative = "");
    LightBird::IContexts      &contexts();
    LightBird::IDatabase      &database();
    LightBird::IEvents        &events();
    LightBird::IExtensions    &extensions();
    LightBird::IGuis          &guis();
    LightBird::ILogs          &log();
    LightBird::INetwork       &network();
    LightBird::IPlugins       &plugins();
    LightBird::ISessions      &sessions();
    LightBird::ITimers        &timers();
    void                      stop();
    void                      restart();
    const QString             &getId() const;
    const QString             &getPluginPath() const;
    const QString             &getResourcesPath() const;
    QString                   getServerVersion() const;
    QString                   getLanguage() const;

private:
    Api();
    Api(const Api &);
    Api &operator=(const Api &);

    QString                   id;                ///< The id of the plugin for which the object has been created.
    QString                   pluginPath;        ///< The path to the plugin that own this object.
    QString                   resourcesPath;     ///< The plugin resources path.
    LightBird::IConfiguration &configurationApi; ///< The instance of the configuration api.
    LightBird::IContexts      &contextsApi;      ///< The instance of the contexts api.
    ApiDatabase               databaseApi;       ///< The instance of the database api.
    ApiEvents                 *eventsApi;        ///< The instance of the events api.
    ApiLogs                   logsApi;           ///< The instance of the logs api.
    ApiNetwork                networkApi;        ///< The instance of the network api
    ApiTimers                 *timersApi;        ///< The instance of the timer api.
};

#endif // API_H
