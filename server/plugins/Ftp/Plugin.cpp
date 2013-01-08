#include <stddef.h>
#include <cstddef>
#include <QSharedPointer>

#include "Ftp.h"
#include "LightBird.h"
#include "ParserControl.h"
#include "ParserData.h"
#include "Plugin.h"

Q_DECLARE_METATYPE(QSharedPointer<Parser>)

Plugin  *Plugin::instance = NULL;

Plugin::Plugin()
{
    Plugin::instance = this;
}

Plugin::~Plugin()
{
    Plugin::instance = NULL;
}

bool    Plugin::onLoad(LightBird::IApi *api)
{
    this->api = api;
    this->commands = new Commands(api);
    this->api->contexts().declareInstance(CONTROL_CONNECTION, (this->control = new Control(api)));
    this->api->contexts().declareInstance(DATA_CONNECTION, (this->data = new Data(api)));
    this->api->contexts().loadContextsFromConfiguration();
    this->timerManager = new Timer(api);
    // Fills the configuration
    if (!(this->configuration.maxPacketSize = api->configuration(true).get("maxPacketSize").toUInt()))
        this->configuration.maxPacketSize = 10000000;
    if (!(this->configuration.waitConnectionTime = api->configuration(true).get("waitConnectionTime").toUInt()) || this->configuration.waitConnectionTime > 30000)
        this->configuration.waitConnectionTime = 5000;
    if (!(this->configuration.timeout = api->configuration(true).get("timeout").toUInt()))
        this->configuration.timeout = 120;
    // Opens the passive data ports
    this->_openPassiveDataPorts();
    return (true);
}

void    Plugin::onUnload()
{
    delete this->commands;
    this->commands = NULL;
    delete this->control;
    this->control = NULL;
    delete this->data;
    this->data = NULL;
    delete this->timerManager;
    this->timerManager = NULL;
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
    metadata.name = "FTP";
    metadata.brief = "The FTP server.";
    metadata.description = "A minimalist FTP server.";
    metadata.autor = "LightBird team";
    metadata.site = "lightbird.cc";
    metadata.email = "team@lightbird.cc";
    metadata.version = "1.0";
    metadata.licence = "CC BY-NC-SA 3.0";
}

bool    Plugin::onConnect(LightBird::IClient &client)
{
    bool    result = true;
    Parser  *parser = NULL;

    // Control connection
    if (client.getMode() == LightBird::IClient::SERVER && !this->configuration.passivePorts.contains(client.getPort()))
    {
        client.getContexts() << CONTROL_CONNECTION;
        parser = new ParserControl(*this->api, client);
        result = this->control->onConnect(client);
    }
    // Data connection
    else
    {
        client.getContexts() << DATA_CONNECTION;
        parser = new ParserData(*this->api, client);
        result = this->data->onConnect(client);
    }
    // Creates the parser of the client
    client.getInformations()[PARSER] = QVariant().fromValue(QSharedPointer<Parser>(parser));
    if (result)
        this->timerManager->startTimeout(client.getId());
    return (result);
}

void    Plugin::onDestroy(LightBird::IClient &client)
{
    Mutex   mutex(Plugin::instance->mutex, "Plugin", "onDestroy");

    this->_cleanPassiveConnections(client.getId(), client.getId());
    this->timerManager->stopTimeout(client.getId());
}

bool    Plugin::timer(const QString &name)
{
    return (this->timerManager->timer(name));
}

void    Plugin::sendControlMessage(const QString &controlId, const Commands::Result &message)
{
    QVariantMap informations;

    if (!Plugin::instance || controlId.isEmpty() || message.second.isEmpty())
        return ;
    // The presence of the information indicates that we need to send a message and not process a request
    informations.insert(CONTROL_SEND_MESSAGE, true);
    // The code of the message
    informations.insert(CONTROL_CODE, message.first);
    // And it's description
    informations.insert(CONTROL_MESSAGE, message.second);
    Plugin::instance->api->network().send(controlId, FTP_PROTOCOL_NAME, informations);
}

Parser  &Plugin::getParser(LightBird::IClient &client)
{
    return (*client.getInformations()[PARSER].value<QSharedPointer<Parser> >().data());
}

Commands    &Plugin::getCommands()
{
    return (*Plugin::instance->commands);
}

Timer   &Plugin::getTimer()
{
    return (*Plugin::instance->timerManager);
}

Plugin::Configuration   &Plugin::getConfiguration()
{
    return (Plugin::instance->configuration);
}

ushort  Plugin::getPassivePort(LightBird::IClient &client)
{
    Mutex         mutex(Plugin::instance->mutex, "Plugin", "getPassivePort");
    QList<ushort> portsUsed;

    // Removes the port previously retained by this client
    Plugin::instance->_cleanPassiveConnections(client.getId());
    // Gets the list of the ports already used by this address
    QMapIterator<ushort, QPair<QString, QHostAddress> > used(Plugin::instance->passivePorts);
    while (used.hasNext())
        if (used.next().value().second == client.getPeerAddress())
            portsUsed << used.key();
    // Searches an available port
    QListIterator<ushort> it(Plugin::instance->configuration.passivePorts);
    while (it.hasNext())
        if (!portsUsed.contains(it.next()))
        {
            // Retains it
            Plugin::instance->passivePorts.insert(it.peekPrevious(), QPair<QString, QHostAddress>(client.getId(), client.getPeerAddress()));
            return (it.peekPrevious());
        }
    return (0);
}

QSharedPointer<Mutex> Plugin::getDataConnection(LightBird::Session &session, LightBird::IClient &client, QString &dataId)
{
    QSharedPointer<Mutex> mutex(new Mutex(Plugin::instance->mutex, "Plugin", "getDataConnection"));
    QHostAddress ip(session->getInformation(SESSION_TRANSFER_IP).toString());
    ushort       port = session->getInformation(SESSION_TRANSFER_PORT).toUInt();

    QMapIterator<ushort, QPair<QString, QHostAddress> > it(Plugin::instance->dataWaiting);
    while (it.hasNext())
    {
        it.next();
        // The data connection have been found
        if (it.key() == port && it.value().second == ip)
        {
            dataId = it.value().first;
            Plugin::instance->_cleanPassiveConnections(client.getId(), dataId, false);
            return (mutex);
        }
    }
    // The control client is waiting the data connection
    Plugin::instance->controlWaiting << client.getId();
    return (mutex);
}

QSharedPointer<Mutex> Plugin::getControlConnection(LightBird::IClient &client, QString &controlId, bool &isValid)
{
    QSharedPointer<Mutex> mutex(new Mutex(Plugin::instance->mutex, "Plugin", "getControlConnection"));
    QHostAddress ip(client.getPeerAddress());
    ushort       port = client.getPort();

    isValid = false;
    // Ensures that no over data client is waiting for the same control connection
    QMapIterator<ushort, QPair<QString, QHostAddress> > it1(Plugin::instance->dataWaiting);
    while (it1.hasNext())
    {
        it1.next();
        if (it1.key() == port && it1.value().second == ip)
            return (mutex);
    }
    // Searches the control connection
    QMapIterator<ushort, QPair<QString, QHostAddress> > it2(Plugin::instance->passivePorts);
    while (it2.hasNext() && !isValid)
    {
        it2.next();
        // The control connection has been found
        if (it2.key() == port && it2.value().second == ip)
        {
            isValid = true;
            // And is waiting the client
            if (Plugin::instance->controlWaiting.contains(it2.value().first))
            {
                controlId = it2.value().first;
                Plugin::instance->_cleanPassiveConnections(controlId, client.getId(), false);
                return (mutex);
            }
        }
    }
    // The data client is waiting the control connection
    if (isValid)
        Plugin::instance->dataWaiting.insert(port, QPair<QString, QHostAddress>(client.getId(), ip));
    return (mutex);
}

void    Plugin::_openPassiveDataPorts()
{
    QStringList  protocols;
    unsigned int maxClients;
    QStringList  fail;

    // Gets the ports list from the data contexts
    QMapIterator<QString, LightBird::IContext *> it1(this->api->contexts().get("data"));
    while (it1.hasNext())
    {
        QStringListIterator it2(it1.next().value()->getPorts());
        while (it2.hasNext())
            if (!this->configuration.passivePorts.contains(it2.next().toUInt()))
                this->configuration.passivePorts << it2.peekPrevious().toUInt();
    }
    if (!this->configuration.passivePorts.isEmpty())
        this->api->log().info("Opening the ports of the passive data connection", "Plugin", "_openPassiveDataPorts");
    else
        LOG_DEBUG("Passive transfert mode disable", "Plugin", "_openPassiveDataPorts");
    // Tries to open the FTP passive ports
    QMutableListIterator<ushort> it3(this->configuration.passivePorts);
    while (it3.hasNext())
    {
        if (!this->api->network().getPort(it3.next(), protocols, maxClients))
        {
            if (!this->api->network().openPort(it3.peekPrevious(), QStringList(FTP_PROTOCOL_NAME)))
                fail << QString::number(it3.peekPrevious());
        }
        // The port is already opened for another protocol
        else if (!protocols.contains(FTP_PROTOCOL_NAME, Qt::CaseInsensitive))
        {
            fail << QString::number(it3.peekPrevious());
            it3.remove();
        }
    }
    if (!fail.isEmpty())
        LOG_WARNING("Unable to open some ports for the passive data connection", Properties("ports", fail.join(" ")).toMap(), "Plugin", "_openPassiveDataPorts");
}

void    Plugin::_cleanPassiveConnections(const QString &controlId, const QString &dataId, bool passivePort)
{
    this->controlWaiting.removeAll(controlId);
    QMutableMapIterator<ushort, QPair<QString, QHostAddress> > it1(this->dataWaiting);
    while (it1.hasNext())
        if (it1.next().value().first == dataId)
            it1.remove();
    if (passivePort)
    {
        QMutableMapIterator<ushort, QPair<QString, QHostAddress> > it2(this->passivePorts);
        while (it2.hasNext())
            if (it2.next().value().first == controlId)
                it2.remove();
    }
}
