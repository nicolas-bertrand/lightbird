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
            public Streamit::IApi
{
public:
    /**
     * @param id : The id of the plugin.
     * @param configuration : Its configuration.
     * @param timers : If the timers has to be loaded.
     */
    Api(const QString &id, Configuration *configuration, bool timers = false, QObject *parent = 0);
    ~Api();

    /// @see Streamit::IApi::log
    Streamit::ILogs             &log();
    /// @see Streamit::IApi::database
    Streamit::IDatabase         &database();
    /// @see Streamit::IApi::configuration
    Streamit::IConfiguration    &configuration(bool plugin = false);
    /// @see Streamit::IApi::configuration
    Streamit::IConfiguration    *configuration(const QString &path, const QString &alternative = "");
    /// @see Streamit::IApi::plugins
    Streamit::IPlugins          &plugins();
    /// @see Streamit::IApi::network
    Streamit::INetwork          &network();
    /// @see Streamit::IApi::guis
    Streamit::IGuis             &guis();
    /// @see Streamit::IApi::extensions
    Streamit::IExtensions       &extensions();
    /// @see Streamit::IApi::timers
    Streamit::ITimers           &timers();
    /// @see Streamit::IApi::stop
    void                        stop();
    /// @see Streamit::IApi::restart
    void                        restart();
    /// @see Streamit::IApi::getId
    const QString               &getId();
    /// @see Streamit::IApi::getServerVersion
    QString                     getServerVersion();
    /// @see Streamit::IApi::getPluginPath
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
