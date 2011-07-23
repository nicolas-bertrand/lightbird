#include <QApplication>
#include <QDir>
#include <QFileInfo>
#include <QProcess>
#include <QTranslator>

#include "IGui.h"

#include "Log.h"
#include "Server.h"
#include "Tools.h"

Server  *Server::_instance = NULL;

Server  &Server::instance(Arguments &args, QObject *parent)
{
    if (Server::_instance == NULL)
        Server::_instance = new Server(args, parent);
    return (*Server::_instance);
}

Server  &Server::instance()
{
    if (Server::_instance == NULL)
        Server::_instance = new Server();
    return (*Server::_instance);
}

void    Server::shutdown()
{
    delete Server::_instance;
    Server::_instance = NULL;
}

Server::Server(Arguments args, QObject *parent) : QObject(parent),
                                                  arguments(args),
                                                  restart(false),
                                                  apiGuis(NULL),
                                                  apiPlugins(NULL),
                                                  configurations(NULL),
                                                  database(NULL),
                                                  events(NULL),
                                                  extensions(NULL),
                                                  network(NULL),
                                                  plugins(NULL),
                                                  threadPool(NULL),
                                                  threads(NULL)
{
    Server::_instance = this;
    Log::info("Server starting", Properties("command line", this->arguments.toString()), "Server", "Server");
    this->_initialize();
}

void    Server::_initialize()
{
    Log::info("Initialazing the server", "Server", "_initialize");
    // Set the current path of the application to the path of the executable
    QDir::setCurrent(QCoreApplication::applicationDirPath());
    // Seed the random number generator
    ::qsrand((unsigned)(QDateTime::currentDateTime().toMSecsSinceEpoch() / 1000));
    // Then the configuration is loaded
    Log::info("Loading the server configuration", "Server", "_initialize");
    if (!*(this->configurations = new Configurations(this->arguments.getConfiguration(), this)))
        return Log::fatal("Failed to load the server configuration", "Server", "_initialize");
    // Tells Qt where are its plugins
    QCoreApplication::addLibraryPath(Configurations::instance()->get("QtPluginsPath"));
    // The threads manager must be initialized just after the configuration
    Log::info("Loading the thread manager", "Server", "_initialize");
    this->threads = new Threads(this);
    // Load the thread pool
    Log::info("Loading the thread pool", "Server", "_initialize");
    this->threadPool = new ThreadPool(this);
    // Load the translator
    Log::info("Loading the translation", "Server", "_initialize");
    if (!this->_loadTranslation(Configurations::instance()->get("languagesPath") + "/", ":languages/"))
        Log::error("Unable to load the translation of the server", "Server", "_initialize");
    // Creates the file path if it doesn't exists
    QString filesPath = Configurations::instance()->get("filesPath");
    if (!QFileInfo(filesPath).isDir() && !QDir().mkpath(filesPath))
        return Log::fatal("Failed to create the files path", Properties("filesPath", filesPath), "Server", "_initialize");
    // Creates the temporary directory
    if (!this->_temporaryDirectory())
        return Log::fatal("Failed to manage the temporary directory", "Server", "_initialize");
    // Load the database
    Log::info("Loading the database", "Server", "_initialize");
    if (!*(this->database = new Database(this)))
        return Log::fatal("Failed to load the database", "Server", "_initialize");
    // Load the network
    Log::info("Loading the network", "Server", "_initialize");
    this->network = new Network(this);
    this->_loadNetwork();
    // Load the events system
    this->events = new Events(this);
    // Load the plugins
    Log::info("Loading the plugins", "Server", "_initialize");
    this->plugins = new Plugins();
    this->_loadPlugins();
    // The log is then initialized which mean that previous logs are really write
    Log::info("Loading the logs", "Server", "_initialize");
    Log::instance()->setMode(Log::WRITE);
    Log::info("Server initialized", "Server", "_initialize");
    Events::instance()->send("server_started");
    this->isInitialized();
}

Server::~Server()
{
    Log::info("Server shutdown", "Server", "~Server");
    // From now on, the logs are printed directly on the standard output, and ILog will not be called anymore
    Log::instance()->setMode(Log::PRINT);
    // Shutdown the features of the server
    Log::info("Unloads all the plugins", "Server", "~Server");
    if (this->plugins)
        this->plugins->shutdown();
    Log::info("Closes the thread pool", "Server", "~Server");
    if (this->threadPool)
        this->threadPool->shutdown();
    Log::info("Finishes the other threads", "Server", "~Server");
    if (this->threads)
        this->threads->shutdown();
    Log::info("Shutdown the network", "Server", "~Server");
    if (this->network)
        this->network->shutdown();
    Log::info("Cleans the server features", "Server", "~Server");
    // The reason why we dont delete directly these objects and call shutdown
    // is that they still need each other before this point. Now we are guaranteed
    // that there is no remaining activity on the server and we can destroy its
    // features safely.
    delete this->plugins;
    delete this->extensions;
    delete this->apiPlugins;
    delete this->apiGuis;
    delete this->events;
    delete this->network;
    delete this->database;
    delete this->threadPool;
    delete this->threads;
    delete this->configurations;
    Log::info("Server stopped", "Server", "~Server");
    // Restart the server if necessary
    if (this->restart)
        QProcess::startDetached(QCoreApplication::instance()->applicationFilePath(), this->arguments.toStringList(), QDir::currentPath());
}

bool            Server::_loadTranslation(const QString &file, const QString &resource)
{
    QTranslator translator;
    QString     language;

    language = Configurations::instance()->get("language");
    if (language.isEmpty())
        language = QLocale::system().name().section('_', 0, 0);
    if (language.isEmpty())
        language = "en";
    if (QFileInfo(file + language + ".qm").isFile())
        language = file + language + ".qm";
    else
        language = resource + language;
    if (!translator.load(language))
    {
        Log::debug("Failed to load the translation", Properties("file", language), "Server", "_loadTranslation");
        return (false);
    }
    Log::debug("Translation loaded", Properties("file", language), "Server", "_loadTranslation");
    QCoreApplication::instance()->installTranslator(&translator);
    return (true);
}

bool    Server::_temporaryDirectory()
{
    QString path;
    QDir    directory;

    path = Tools::cleanPath(Configurations::instance()->get("temporaryPath"));
    // If the temporary directory is not the working directory
    if (!path.isEmpty() && path != ".")
    {
        // Removes all the files in the temporary directory if needed
        if (Tools::cleanPath(Configurations::instance()->get("cleanTemporaryPath")) == "true" && QFileInfo(path).isDir())
        {
            Log::debug("Cleaning the temporary directory", Properties("path", path), "Server", "_temporaryDirectory");
            directory.setPath(path);
            // Loop over all the files of the directory
            QStringListIterator it(directory.entryList(QDir::Files));
            while (it.hasNext())
                if (!directory.remove(it.next()))
                    Log::warning("Unable to remove the temporary file", Properties("file", it.peekPrevious()).add("path", path), "Server", "_temporaryDirectory");
                else if (Log::instance()->isTrace())
                    Log::trace("Temporary file removed", Properties("file", it.peekPrevious()).add("path", path), "Server", "_temporaryDirectory");
        }
        // Creates the temporary directory if it doesn't exists
        if (!QFileInfo(path).isDir())
        {
            if (!directory.mkpath(path))
            {
                Log::error("Unable to create the temporary directory", Properties("path", path), "Server", "_temporaryDirectory");
                return (false);
            }
            else
                Log::debug("Temporary directory created", Properties("path", path), "Server", "_temporaryDirectory");
        }
    }
    return (true);
}

void    Server::_loadNetwork()
{
    QDomElement                     node;
    QMap<QString, QString>          port;
    QList<QMap<QString, QString> >  ports;
    LightBird::INetwork::Transport  transport;
    bool                            loaded = false;

    // Get the information on the ports to load from the configuration
    node = Configurations::instance()->readDom().firstChildElement("ports");
    for (node = node.firstChildElement("port"); !node.isNull(); node = node.nextSiblingElement("port"))
    {
        port["port"] = node.text();
        port["protocols"] = node.attribute("protocol");
        port["transport"] = node.attribute("transport");
        port["maxClients"] = node.attribute("maxClients");
        ports.push_back(port);
    }
    Configurations::instance()->release();
    // Opens the ports in the list
    QListIterator<QMap<QString, QString> >  it(ports);
    while (it.hasNext())
    {
        transport = LightBird::INetwork::TCP;
        if (it.peekNext().value("transport").toUpper() == "UDP")
            transport = LightBird::INetwork::UDP;
        if (Network::instance()->openPort(it.peekNext().value("port").toShort(),
                                          it.peekNext().value("protocols").simplified().split(' '),
                                          transport, it.peekNext().value("maxClients").toUInt()))
            loaded = true;
        it.next();
    }
    if (loaded)
        Log::debug("Network loaded", "Server", "_loadNetwork");
    else
        Log::warning("No network port opened", "Server", "_loadNetwork");
}

void            Server::_loadPlugins()
{
    QDomElement plugin;
    QStringList list;
    bool        loaded = false;

    // Load some API and the extension manager
    this->apiGuis = new ApiGuis(this);
    this->apiPlugins = new ApiPlugins(this);
    this->extensions = new Extensions(this);
    // Allows the server to know when a plugin is loaded (only in GUI mode)
    if (qobject_cast<QApplication *>(QCoreApplication::instance()))
        QObject::connect(Plugins::instance(), SIGNAL(loaded(QString)), this, SLOT(_pluginLoaded(QString)), Qt::QueuedConnection);
    // If the word "all" is in the node "plugins", all the installed plugins are loaded
    if (Configurations::instance()->get("plugins").trimmed().toLower() == "all")
    {
        Log::debug("All the installed plugins are going to be loaded", "Server", "_loadPlugins");
        list = Plugins::instance()->getInstalledPlugins();
    }
    // Get the list of the plugins to load
    else
    {
        plugin = Configurations::instance()->readDom().firstChildElement("plugins");
        for (plugin = plugin.firstChildElement("plugin"); !plugin.isNull(); plugin = plugin.nextSiblingElement("plugin"))
            list.push_back(plugin.text());
        Configurations::instance()->release();
    }
    // Load the plugins in the list
    QStringListIterator it(list);
    while (it.hasNext())
        // The getResult allows to wait until the plugin is actually loaded (or not)
        if (Plugins::instance()->load(it.next()).getResult())
            loaded = true;
    if (loaded)
        Log::debug("Plugins loaded", "Server", "_loadPlugins");
    else
        Log::warning("No plugin loaded", "Server", "_loadPlugins");
}

void                Server::_pluginLoaded(QString id)
{
    LightBird::IGui *instance;
    QString         path;

    // Load the translation of the plugin if possible
    path = Configurations::instance()->get("pluginsPath") + "/" + id + "/";
    if (Configurations::instance(id)->count("translation") &&
        Configurations::instance(id)->get("translation") == "true")
    {
        Log::debug("Loading the translation of the plugin", Properties("id", id), "_pluginLoaded", "Server");
        this->_loadTranslation(path, Plugins::instance()->getResourcesPath(id) + "/languages/");
    }
    // Calls the Gui method of the plugin if it implements it
    if ((instance = Plugins::instance()->getInstance<LightBird::IGui>(id)))
    {
        instance->gui();
        Plugins::instance()->release(id);
    }
}

void            Server::stop(bool restart)
{
    this->restart = restart;
    QCoreApplication::quit();
}

ApiGuis         *Server::getApiGuis()
{
    return (Server::instance().getApiGuis());
}

ApiPlugins      *Server::getApiPlugins()
{
    return (Server::instance().getApiPlugins());
}

Configuration   *Server::getConfiguration(const QString &configuration, const QString &alternative)
{
    return (Server::instance().configurations->getConfiguration(configuration, alternative));
}

Database        *Server::getDatabase()
{
    return (Server::instance().database);
}

Events          *Server::getEvents()
{
    return (Server::instance().events);
}

Extensions      *Server::getExtensions()
{
    return (Server::instance().extensions);
}

Network         *Server::getNetwork()
{
    return (Server::instance().network);
}

Plugins         *Server::getPlugins()
{
    return (Server::instance().plugins);
}

ThreadPool      *Server::getThreadPool()
{
    return (Server::instance().threadPool);
}

Threads         *Server::getThreads()
{
    return (Server::instance().threads);
}
