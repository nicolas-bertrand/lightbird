#include <QStringList>

#include "Configuration.h"
#include "Database.h"
#include "Ftp.h"
#include "Library.h"
#include "LightBird.h"
#include "Network.h"
#include "Plugin.h"

Plugin::Plugin()
    : shutdown(false)
{
}

Plugin::~Plugin()
{
}

bool    Plugin::onLoad(LightBird::IApi *api)
{
    LightBird::IConfiguration &configuration = api->configuration(true);

    this->api = api;
    this->api->events().subscribe(QStringList() << "server_started");
    // Loads the tests
    if (configuration.count("configuration"))
        this->tests << QPair<QString, ITest *>("Configuration", new Configuration(*this->api));
    if (configuration.count("database"))
        this->tests << QPair<QString, ITest *>("Database", new Database(*this->api));
    if (configuration.count("ftp"))
        this->tests << QPair<QString, ITest *>("Ftp", new Ftp(*this->api));
    if (configuration.count("library"))
        this->tests << QPair<QString, ITest *>("Library", new Library(*this->api));
    if (configuration.count("network"))
        this->tests << QPair<QString, ITest *>("Network", new Network(*this->api));
    if (configuration.get("shutdown") == "true")
        this->shutdown = true;
    return (true);
}

void    Plugin::onUnload()
{
    QListIterator<QPair<QString, ITest *> > test(this->tests);

    while (test.hasNext())
        delete test.next().second;
}

bool    Plugin::onInstall(LightBird::IApi *api)
{
    this->api = api;
    return (true);
}

void    Plugin::onUninstall(LightBird::IApi *api)
{
    this->api = api;
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
    unsigned int    line = 0;
    QListIterator<QPair<QString, ITest *> > test(this->tests);

    LOG_INFO("Running the tests of the server...", "Plugin", "event");
    // Runs the tests
    while (test.hasNext() && !line)
        line = test.next().second->run();
    if (!line)
        this->api->log().info("All the tests were successful!", "Plugin", "event");
    else
        LOG_ERROR("At least one test failed!", Properties("class", test.peekPrevious().first).add("line", line).toMap(), "Plugin", "event");
    if (this->shutdown)
    {
        this->api->log().info("Shutting down the server", "Plugin", "event");
        this->api->stop();
    }
}
