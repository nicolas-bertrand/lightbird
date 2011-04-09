#include <QApplication>
#include <QTranslator>
#include <QString>
#include <QPixmap>
#include <QBitmap>
#include <QFileInfo>
#include <QDir>

#include "Database.h"
#include "Threads.h"
#include "Server.h"
#include "Network.h"
#include "Configurations.h"
#include "Plugins.hpp"
#include "Log.h"
#include "IGui.h"
#include "Timer.h"
#include "ITimer.h"
#include "ApiGuis.h"

Server::Server(int argc, char *argv[], QObject *parent) : QObject(parent)
{
    // Parse the arguments of the program
    for (int i = 0; i < argc; ++i)
        this->arguments << argv[i];
    Log::info("Server starting", Properties("command line", arguments.join(", ").prepend('"').append('"')), "Server", "Server");
    QStringListIterator it(this->arguments);
    while (it.hasNext())
    {
        it.next();
        // If the configuration is found
        if (it.peekPrevious() == "-c" && it.hasNext())
            this->configurationPath = it.peekNext();
    }
    this->_initialize();
}

Server::~Server()
{
    // Delete the splash screen
    if (this->splashScreen)
        this->splashScreen->deleteLater();
    // From now, the logs are printed directly on the standard output, and ILog will not be called anymore
    Log::instance()->setMode(Log::PRINT);
    // Unload all the plugins (block until all plugins are unloaded)
    if (Plugins::isLoaded())
        Plugins::instance()->unloadAll();
    // Finish all the threads (only once all the plugins are unloaded)
    if (Threads::isLoaded())
        Threads::instance()->deleteAll();
    Log::info("Server destroyed", "Server", "~Server");
}

bool    Server::isInitialized()
{
    return (this->initialized);
}

void    Server::_initialize()
{
    this->initialized = false;
    Log::info("Initialazing the server", "Server", "_initialize");
    this->splashScreen = NULL;
    // Seed the random number generator
    ::qsrand(QDateTime::currentDateTime().toTime_t());
    // Then the configuration is loaded
    Log::info("Loading the server configuration", "Server", "_initialize");
    if (!Configurations::server(this->configurationPath, this))
    {
        Log::fatal("Failed to load the server configuration", "Server", "_initialize");
        return this->_splashScreen();
    }
    // This allows to display the splash screen
    this->_splashScreen(Configurations::instance()->get("splashScreen"));
#ifdef Q_WS_WIN
    // Tells Qt where are its plugins
    QCoreApplication::addLibraryPath(Configurations::instance()->get("QtPluginsPath"));
#endif
    // The threads manager must be initialized just after the configuration
    Log::info("Loading the thread manager", "Server", "_initialize");
    Threads::instance(this);
    // Load the translator
    Log::info("Loading the translation", "Server", "_initialize");
    if (!this->_loadTranslation(":languages/", Configurations::instance()->get("languagesPath") + "/"))
        Log::error("Unable to load the translation of the server", "Server", "_initialize");
    // Creates the file path if it doesn't exists
    QString filesPath = Configurations::instance()->get("filesPath");
    if (!QFileInfo(filesPath).isDir() && !QDir().mkpath(filesPath))
    {
        Log::fatal("Failed to create the files path", Properties("filesPath", filesPath), "Server", "_initialize");
        return this->_splashScreen();
    }
    // Creates the temporary directory
    if (!this->_temporaryDirectory())
        return this->_splashScreen();
    // Load the database
    Log::info("Loading the database", "Server", "_initialize");
    if (!Database::instance(this))
    {
        Log::fatal("Failed to load the database", "Server", "_initialize");
        return this->_splashScreen();
    }
    // Load the network
    Log::info("Loading the network", "Server", "_initialize");
    Network::instance(this);
    this->_loadNetwork();
    // The plugins are loaded
    Log::info("Loading the plugins", "Server", "_initialize");
    Plugins::instance(this);
    this->_loadPlugins();
    // The log is then initialized which mean that previous logs are really write
    Log::info("Loading the logs", "Server", "_initialize");
    Log::instance()->setMode(Log::WRITE);
    // Hide the splash screen, since the initialization is finished
    this->_splashScreen();
    Log::info("Server initialized", "Server", "_initialize");
    this->initialized = true;
}

void    Server::_splashScreen(const QString &path)
{
    // If we are in noGui mode or there is no splash screen, the splash screen is not displayed
    if (!qobject_cast<QApplication *>(QCoreApplication::instance()) || (path.isEmpty() && !this->splashScreen))
        return ;
    // Creates the splash screen
    if (!this->splashScreen)
    {
        QPixmap pixmap(path);
        if (pixmap.isNull())
        {
            if (path.isEmpty() == false)
                Log::warning("Invalid splash screen", Properties("file", path), "Server", "_splashScreen");
            return ;
        }
        this->splashScreen = new QSplashScreen(pixmap, Qt::WindowStaysOnTopHint);
        this->splashScreen->setMask(QRegion(pixmap.mask()));
        this->splashScreen->setWindowOpacity(0.9);
    }
    // Show or hide the splash screen
    this->splashScreen->setVisible(!this->splashScreen->isVisible());
    return ;
}

bool            Server::_loadTranslation(const QString &resource, const QString &file)
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
    bool    clean = false;

    path = directory.cleanPath(Configurations::instance()->get("temporaryPath"));
    // If the temporary directory is not the working directory
    if (!path.isEmpty() && path != ".")
    {
        // Removes all the files in the temporary directory if needed
        if (directory.cleanPath(Configurations::instance()->get("cleanTemporaryPath")) == "true")
            clean = true;
        if (clean && QFileInfo(path).isDir())
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
                Log::fatal("Unable to create the temporary directory", Properties("path", path), "Server", "_temporaryDirectory");
                return (false);
            }
            else
                Log::debug("Temporary directory created", Properties("path", path), "Server", "_temporaryDirectory");
        }
    }
    return (true);
}

void                Server::_loadNetwork()
{
    QDomNodeList                    nodes;
    QMap<QString, QString>          port;
    QList<QMap<QString, QString> >  ports;
    LightBird::INetwork::Transports transport;
    bool                            allCreated;

    // Get the information on the ports to load from the configuration
    nodes = Configurations::instance()->readDom().firstChildElement("ports").elementsByTagName("port");
    for (int i = 0; i < nodes.size(); ++i)
    {
        port["port"] = nodes.item(i).toElement().text();
        port["protocols"] = nodes.item(i).toElement().attribute("protocol");
        port["transport"] = nodes.item(i).toElement().attribute("transport");
        port["maxClients"] = nodes.item(i).toElement().attribute("maxClients");
        ports.push_back(port);
    }
    Configurations::instance()->release();
    // Creates the ports in the list
    allCreated = true;
    QListIterator<QMap<QString, QString> >  it(ports);
    while (it.hasNext())
    {
        transport = LightBird::INetwork::TCP;
        if (it.peekNext().value("transport").toUpper() == "UDP")
            transport = LightBird::INetwork::UDP;
        if (!(Network::instance()->addPort(it.peekNext().value("port").toShort(),
                                           it.peekNext().value("protocols").simplified().split(' '),
                                           transport, it.peekNext().value("maxClients").toUInt()).getResult()))
            allCreated = false;
        it.next();
    }
    if (nodes.size() == 0)
        Log::warning("No plugin to load", "Server", "_loadNetwork");
    else
        Log::trace("Network loaded", "Server", "_loadNetwork");
}

void                Server::_loadPlugins()
{
    QDomElement     dom;
    QDomNodeList    plugins;
    QStringList     list;
    int             i;

    // Allows the server to know when a plugin is loaded (only in GUI mode)
    if (qobject_cast<QApplication *>(QCoreApplication::instance()))
        QObject::connect(Plugins::instance(), SIGNAL(loaded(QString)), this, SLOT(_pluginLoaded(QString)), Qt::QueuedConnection);
    ApiGuis::instance(this);
    // If the word "all" is in the node "plugins", all the installed plugins are loaded
    if (Configurations::instance()->get("plugins").trimmed().toLower() == "all")
    {
        Log::debug("All the installed plugins are going to be loaded", "Server", "_loadPlugins");
        list = Plugins::instance()->getInstalledPlugins();
    }
    // Get the list of the plugins to load
    else
    {
        dom = Configurations::instance()->readDom();
        plugins = dom.elementsByTagName("plugin");
        for (i = 0; i < plugins.size(); ++i)
            list.push_back(plugins.item(i).toElement().text());
        Configurations::instance()->release();
    }
    // Load the plugins in the list
    QStringListIterator it(list);
    while (it.hasNext())
        // The getResult allows to wait until the plugin is actually loaded (or not)
        Plugins::instance()->load(it.next()).getResult();;
    if (list.size() == 0)
        Log::warning("No plugin to load", "Server", "_loadPlugins");
    else
        Log::trace("Plugins loaded", "Server", "_loadPlugins");
}

void                Server::_pluginLoaded(QString id)
{
    LightBird::IGui *instance;
    QString         path;

    // Load the translation of the plugin if possible
    path = Configurations::instance()->get("pluginsPath") + "/" + id + "/";
    if (Configurations::instance(path + "Configuration.xml")->count("translation") &&
        Configurations::instance(path + "Configuration.xml")->get("translation") == "true")
    {
        Log::debug("Loading the translation of the plugin", Properties("id", id), "_pluginLoaded", "Server");
        this->_loadTranslation(Plugins::instance()->getResourcesPath(id) + "/languages/", path);
    }
    // Calls the Gui method of the plugin if it implements it
    if ((instance = Plugins::instance()->getInstance<LightBird::IGui>(id)))
    {
        instance->gui();
        Plugins::instance()->release(id);
    }
}

#include "TableAccounts.h"
#include "TableCollections.h"
#include "TableDirectories.h"
#include "TableEvents.h"
#include "TableFiles.h"
#include "TableGroups.h"
#include "TableLimits.h"
#include "TablePermissions.h"
#include "TableTags.h"

bool        Server::unitTests()
{
    bool    result = true;

    Log::info("Runing the unit tests of the whole server", "Server", "unitTests");
    if (!Configurations::instance()->unitTests())
        result = false;
    else if (!TableAccounts::unitTests())
        result = false;
    else if (!TableCollections::unitTests())
        result = false;
    else if (!TableDirectories::unitTests())
        result = false;
    else if (!TableEvents::unitTests())
        result = false;
    else if (!TableFiles::unitTests())
        result = false;
    else if (!TableGroups::unitTests())
        result = false;
    else if (!TableLimits::unitTests())
        result = false;
    else if (!TablePermissions::unitTests())
        result = false;
    else if (!TableTags::unitTests())
        result = false;
    if (result)
        Log::info("All the unit tests were successful!", "Server", "unitTests");
    else
        Log::info("At least one unit test failed!", "Server", "unitTests");
    return (result);
}
