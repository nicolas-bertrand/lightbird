#ifndef SERVER_H
# define SERVER_H

# include <QObject>
# include <QString>

# include "ApiGuis.h"
# include "ApiPlugins.h"
# include "Arguments.h"
# include "Configurations.h"
# include "Database.h"
# include "Events.h"
# include "Extensions.h"
# include "Initialize.h"
# include "Network.h"
# include "Plugins.hpp"
# include "ThreadPool.h"
# include "Threads.h"

/// @brief The main class of the LightBird server.
class Server : public QObject,
               public Initialize
{
    Q_OBJECT

public:
    /// @brief Initializes the server with the arguments in parameter.
    static Server   &instance(Arguments &args, QObject *parent);
    /// @brief Provides access to the Server instance.
    static Server   &instance();
    /// @brief Shutdown the Server and delete its instance.
    static void     shutdown();

    /// @brief Stops the server, ie this entire process.
    /// @param restart : It true, the server will be restarted in another process.
    void            stop(bool restart = false);
    // These methods allows to access to the features of the server.
    ApiGuis         *getApiGuis();
    ApiPlugins      *getApiPlugins();
    Configuration   *getConfiguration(const QString &configuration, const QString &alternative);
    Database        *getDatabase();
    Events          *getEvents();
    Extensions      *getExtensions();
    Network         *getNetwork();
    Plugins         *getPlugins();
    ThreadPool      *getThreadPool();
    Threads         *getThreads();

private:
    /// @brief Initializes all the components of the server.
    Server(Arguments arguments = Arguments(), QObject *parent = 0);
    /// @brief Stops the server. This may take some time, since all the working
    /// threads have to be finished.
    ~Server();
    Server(const Server &server);
    Server  &operator=(const Server &server);

    /// @brief Called by the constructor to initialize the server.
    void    _initialize();
    /// @brief Load the translatations of the texts of the server and the plugins
    /// in the local language.
    /// @param file : The path of the language file on the file system.
    /// @param resource : The path of the language file in the resources. Used only
    /// if the file has not been found using the first parameter.
    /// @return True if the translation has been loaded.
    bool    _loadTranslation(const QString &file, const QString &resource);
    /// @brief Manage the temporary directory. Creates it if it doesn't exists, or
    /// removes its files.
    /// @return True if no error occured.
    bool    _temporaryDirectory();
    /// @brief Opens each ports mentionned in the configuration file of the server,
    /// under the nodes <port>, childrens of the node <ports>.
    void    _loadNetwork();
    /// @brief Attemps to load each plugin mentionned in the configuration file of
    /// the server, under the nodes <plugin>, childrens of the node <plugins>.
    void    _loadPlugins();

private slots:
    /// @brief Calls the IGui::show() method of a plugin that has just been loaded,
    /// and load its translations if possible.
    void    _pluginLoaded(QString id);

private:
    Arguments       arguments;  ///< Stores the arguments used to launch the server.
    bool            restart;    ///< If true, the server is going to be restarted.
    static Server   *_instance; ///< The instance of the Server singleton.
    // The following members manages all the features of the server.
    ApiGuis         *apiGuis;
    ApiPlugins      *apiPlugins;
    Configurations  *configurations;
    Database        *database;
    Events          *events;
    Extensions      *extensions;
    Network         *network;
    Plugins         *plugins;
    ThreadPool      *threadPool;
    Threads         *threads;
};

#endif // SERVER_H
