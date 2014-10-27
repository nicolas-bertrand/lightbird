#include <QStringList>

#include "Configuration.h"
#include "Database.h"
#include "Ftp.h"
#include "Library.h"
#include "LightBird.h"
#include "Network.h"
#include "Plugin.h"

Plugin::Plugin()
    : _shutdown(false)
{
}

Plugin::~Plugin()
{
}

bool    Plugin::onLoad(LightBird::IApi *api)
{
    LightBird::IConfiguration &configuration = api->configuration(true);

    _api = api;
    _api->events().subscribe(QStringList() << "server_started");
    // Loads the tests
    if (configuration.count("configuration"))
        _tests << QPair<QString, ITest *>("Configuration", new Configuration(*_api));
    if (configuration.count("database"))
        _tests << QPair<QString, ITest *>("Database", new Database(*_api));
    if (configuration.count("ftp"))
        _tests << QPair<QString, ITest *>("Ftp", new Ftp(*_api));
    if (configuration.count("library"))
        _tests << QPair<QString, ITest *>("Library", new Library(*_api));
    if (configuration.count("network"))
        _tests << QPair<QString, ITest *>("Network", new Network(*_api));
    if (configuration.get("shutdown") == "true")
        _shutdown = true;
    return (true);
}

void    Plugin::onUnload()
{
    QListIterator<QPair<QString, ITest *> > test(_tests);

    while (test.hasNext())
        delete test.next().second;
}

bool    Plugin::onInstall(LightBird::IApi *api)
{
    _api = api;
    return (true);
}

void    Plugin::onUninstall(LightBird::IApi *api)
{
    _api = api;
}

void    Plugin::getMetadata(LightBird::IMetadata &metadata) const
{
    metadata.name = "Test";
    metadata.brief = "Runs various tests on the server.";
    metadata.description = "Allows to test the server API, the network, the database, etc.";
    metadata.autor = "LightBird team";
    metadata.site = "lightbird.cc";
    metadata.email = "team@lightbird.cc";
    metadata.version = "1.0";
    metadata.licence = "CC BY-NC-SA 3.0";
}

void    Plugin::event(const QString &, const QVariant &)
{
    this->start();
}

void    Plugin::run()
{
    unsigned int    line = 0;
    QListIterator<QPair<QString, ITest *> > test(_tests);

    _api->log().info("Running the tests of the server...", "Plugin", "event");
    // Runs the tests
    while (test.hasNext() && !line)
        line = test.next().second->run();
    if (!line)
        _api->log().info("All the tests were successful!", "Plugin", "event");
    else
        _api->log().error("At least one test failed!", Properties("class", test.peekPrevious().first).add("line", line).toMap(), "Plugin", "event");
    if (_shutdown)
    {
        ::exit(42);
        _api->log().info("Shutting down the server", "Plugin", "event");
        _api->stop();
    }
}

void    Plugin::execute()
{
    dynamic_cast<Plugin *>(QThread::currentThread())->exec();
}
