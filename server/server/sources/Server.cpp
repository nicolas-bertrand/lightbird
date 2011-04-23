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

Server::Server(Arguments &args, QObject *parent) : QObject(parent),
                                                   arguments(args),
                                                   initialized(false)
{
    Log::info("Server starting", Properties("command line", this->arguments.toString()), "Server", "Server");
    this->_initialize();
}

void    Server::_initialize()
{
    Log::info("Initialazing the server", "Server", "_initialize");
    // Seed the random number generator
    ::qsrand((unsigned)(QDateTime::currentDateTime().toMSecsSinceEpoch() / 1000));
    // Then the configuration is loaded
    Log::info("Loading the server configuration", "Server", "_initialize");
    if (!Configurations::server(this->arguments.getConfiguration(), this))
        return Log::fatal("Failed to load the server configuration", "Server", "_initialize");
    // Tells Qt where are its plugins
    QCoreApplication::addLibraryPath(Configurations::instance()->get("QtPluginsPath"));
    // The threads manager must be initialized just after the configuration
    Log::info("Loading the thread manager", "Server", "_initialize");
    Threads::instance(this);
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
    if (!Database::instance(this))
        return Log::fatal("Failed to load the database", "Server", "_initialize");
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
    Log::info("Server initialized", "Server", "_initialize");
    this->initialized = true;
}

Server::~Server()
{
    Log::info("Server shutdown", "Server", "~Server");
    // From now on, the logs are printed directly on the standard output, and ILog will not be called anymore
    Log::instance()->setMode(Log::PRINT);
    // Unload all the plugins (block until all plugins are unloaded)
    if (Plugins::isLoaded())
        Plugins::instance()->unloadAll();
    // Finish all the threads (only once all the plugins are unloaded)
    if (Threads::isLoaded())
        Threads::instance()->deleteAll();
    Log::info("Server stopped", "Server", "~Server");
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

    path = directory.cleanPath(Configurations::instance()->get("temporaryPath"));
    // If the temporary directory is not the working directory
    if (!path.isEmpty() && path != ".")
    {
        // Removes all the files in the temporary directory if needed
        if (directory.cleanPath(Configurations::instance()->get("cleanTemporaryPath")) == "true" && QFileInfo(path).isDir())
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
    LightBird::INetwork::Transports transport;
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
        if (Network::instance()->addPort(it.peekNext().value("port").toShort(),
                                         it.peekNext().value("protocols").simplified().split(' '),
                                         transport, it.peekNext().value("maxClients").toUInt()).getResult())
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

Server::operator bool()
{
    return (this->initialized);
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
