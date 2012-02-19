#include <QCoreApplication>
#include <QHostAddress>

#include "IOnConnect.h"
#include "IDoRead.h"
#include "IDoWrite.h"
#include "IOnDestroy.h"
#include "IOnDisconnect.h"

#include "ApiSessions.h"
#include "Client.h"
#include "EngineClient.h"
#include "EngineServer.h"
#include "LightBird.h"
#include "Log.h"
#include "Plugins.hpp"
#include "SmartMutex.h"
#include "Threads.h"

Client::Client(QAbstractSocket *s, LightBird::INetwork::Transport t, const QStringList &pr,
               unsigned short p, int sd, const QHostAddress &pa, unsigned short pp,
               const QString &pn, LightBird::IClient::Mode m, IReadWrite *r) :
               transport(t), protocols(pr), port(p), socketDescriptor(sd), peerAddress(pa),
               peerPort(pp), peerName(pn), mode(m), readWriteInterface(r), socket(s)
{
    // Generates the uuid of the client
    this->id = LightBird::createUuid();
    this->readyRead = false;
    this->running = false;
    this->finish = false;
    this->disconnecting = false;
    this->disconnected = false;
    this->state = Client::NONE;
    // Set the connection date at the current date
    this->connectionDate = QDateTime::currentDateTime();
    // Creates the engine
    if (this->mode == LightBird::IClient::SERVER)
        this->engine = new EngineServer(*this);
    else
        this->engine = new EngineClient(*this);
    // Add a task to call the _onConnect method
    this->_newTask(Client::CONNECT);
}

Client::~Client()
{
    // Ensure that all the informations requests has been processed
    this->_getInformations();
    delete this->engine;
    Log::trace("Client destroyed!", Properties("id", this->id), "Client", "~Client");
}

void        Client::run()
{
    State   newTask = Client::NONE;
    bool    send = true;
    bool    receive = true;

    // Performs different actions depending on the client's state
    switch (this->state)
    {
        case Client::CONNECT :
            // Tries to connect to the client
            if (!this->readWriteInterface->connect(this))
                return this->_finish();
            Log::info("Client connected", Properties("id", this->id).add("port", this->port)
                      .add("socket", ((this->socketDescriptor >= 0) ? QString::number(this->socketDescriptor) : ""), false)
                      .add("peerAddress", this->peerAddress.toString()).add("peerName", this->peerName, false)
                      .add("mode", (this->getMode() == LightBird::IClient::CLIENT) ? "client" : "server")
                      .add("peerPort", this->peerPort), "Client", "run");
            // If the client is not allowed to connect, it is disconnected
            if (!this->_onConnect())
                newTask = Client::DISCONNECT;
            break;

        case Client::READ :
            // If data are available, we try to execute them
            if (this->_read())
                newTask = Client::RUN;
            break;

        case Client::SEND :
            // Some data have to be sent to the client
            if ((send = this->_send()))
                newTask = Client::RUN;
            break;

        case Client::RECEIVE :
            // Some data have to be received without sending a request
            if ((receive = this->_receive()))
                newTask = Client::RUN;
            break;

        case Client::RUN :
            // While run() returns true, the engine continues to run
            if (this->engine->run())
                newTask = Client::RUN;
            // Otherwise we try to get more data
            else
                this->readyRead = true;
            break;

        case Client::DISCONNECT :
            this->finish = false;
            if (this->_onDisconnect())
                // The client is destroyed now
                return this->_finish();
            // Otherwise it will be destroyed when all the data have been processed
            this->disconnecting = true;
            break;

        default:
            break;
    }

    // Runs a new task
    SmartMutex mutex(this->mutex, "Client", "run");
    if (!mutex)
        return ;
    // Fills the pending informations requests
    if (!this->informationsRequests.isEmpty())
        this->_getInformations();
    // The client must be disconnected
    if (this->finish)
        newTask = Client::DISCONNECT;
    // A new task has already been assigned
    else if (newTask != Client::NONE)
        newTask = newTask;
    // Some data have to be sent to the client
    else if (!this->sendRequests.isEmpty() && send)
        newTask = Client::SEND;
    // Some data have to be received without sending a request
    else if (!this->receiveResponses.isEmpty() && receive)
        newTask = Client::RECEIVE;
    // Some data are available to be read on the network
    else if (this->readyRead)
        newTask = Client::READ;
    // The client is disconnecting and there is nothing left to do
    else if (this->disconnecting)
    {
        mutex.unlock();
        return this->_finish();
    }
    // Starts the new task
    if (newTask != Client::NONE)
    {
        mutex.unlock();
        this->_newTask(newTask);
    }
    // All the tasks of the client has been completed
    else
        this->running = false;
}

void        Client::read(QByteArray *data)
{
    // If data is NULL they are read using readWriteInterface
    if (data == NULL)
    {
        data = new QByteArray();
        this->readWriteInterface->read(*data, this);
    }
    // Nothing has been read
    if (data->isEmpty())
    {
        delete data;
        return ;
    }
    SmartMutex mutex(this->mutex, "Client", "read");
    if (!mutex)
    {
        delete data;
        return ;
    }
    // Stores the new data
    this->data.append(data);
    this->readyRead = true;
    // If the client is not running, a new task can be created for read the data.
    // Otherwise the data will be read after the current task of the client is completed.
    if (!this->running)
        this->_newTask(Client::READ);
}

bool                    Client::_read()
{
    SmartMutex          mutex(this->mutex, "Client", "_read");
    QList<QByteArray *> data;

    if (!mutex)
        return (false);
    this->readyRead = false;
    // If there is no data, no processing are needed
    if (this->data.isEmpty())
        return (false);
    // Copy the data in order to clear the list inside the mutex
    data = this->data;
    this->data.clear();
    mutex.unlock();
    // Feed the engine outside the mutex
    this->engine->read(data);
    // Removes the data
    QListIterator<QByteArray *> it(data);
    while (it.hasNext())
        delete it.next();
    return (true);
}

void        Client::write(QByteArray *data)
{
    if (Log::instance()->isTrace())
        Log::trace("Writing data", Properties("id", this->id).add("data", LightBird::simplify(*data)).add("size", data->size()), "Client", "write");
    else if (Log::instance()->isDebug())
        Log::debug("Writing data", Properties("id", this->id).add("size", data->size()), "Client", "write");
    // Writes the data if the client is still connected
    if (!this->disconnecting)
        this->readWriteInterface->write(data, this);
    // Otherwise we have to destroy them
    else
        delete data;
}

bool            Client::send(const QString &protocol, const QVariantMap &informations, const QString &id)
{
    SmartMutex  mutex(this->mutex, "Client", "send");
    QVariantMap sendRequest;

    if (!mutex)
        return (false);
    sendRequest["protocol"] = protocol;
    sendRequest["informations"] = informations;
    if (this->mode == LightBird::IClient::SERVER)
    {
        this->sendRequests.push_back(sendRequest);
        if (!this->running && this->engine->isIdle())
            this->_newTask(Client::SEND);
        else
            return (false);
    }
    else if (this->mode == LightBird::IClient::CLIENT)
    {
        sendRequest["id"] = id;
        this->sendRequests.push_back(sendRequest);
        if (!this->running)
            this->_newTask(Client::SEND);
    }
    return (true);
}

bool                Client::_send()
{
    SmartMutex      mutex(this->mutex, "Client", "_send");
    EngineServer    *engineServer = qobject_cast<EngineServer *>(this->engine);
    EngineClient    *engineClient = qobject_cast<EngineClient *>(this->engine);
    bool            run = false;

    if (!mutex)
        return (false);
    if (engineServer && !this->sendRequests.isEmpty()
        && (run = engineServer->send(this->sendRequests.first().value("protocol").toString(),
                                     this->sendRequests.first().value("informations").toMap())))
        this->sendRequests.pop_front();
    else if (engineClient)
    {
        // Sets the send requests to the engine
        QListIterator<QVariantMap> it(this->sendRequests);
        while (it.hasNext())
        {
            // If Engine::send returned true at least once, the engine needs to run
            if (engineClient->send(it.peekNext().value("id").toString(),
                                   it.peekNext().value("protocol").toString(),
                                   it.peekNext().value("informations").toMap()))
                run = true;
            it.next();
        }
        this->sendRequests.clear();
    }
    return (run);
}

bool            Client::receive(const QString &protocol, const QVariantMap &informations)
{
    SmartMutex  mutex(this->mutex, "Client", "receive");
    QVariantMap receiveResponses;

    if (!mutex)
        return (false);
    receiveResponses["protocol"] = protocol;
    receiveResponses["informations"] = informations;
    if (this->mode == LightBird::IClient::CLIENT)
    {
        this->receiveResponses.push_back(receiveResponses);
        if (!this->running && this->engine->isIdle())
            this->_newTask(Client::RECEIVE);
        else
            return (false);
    }
    return (true);
}

bool                Client::_receive()
{
    SmartMutex      mutex(this->mutex, "Client", "_receive");
    EngineClient    *engine = qobject_cast<EngineClient *>(this->engine);
    bool            run = false;

    if (!mutex)
        return (false);
    if (engine && !this->receiveResponses.isEmpty()
        && (run = engine->receive(this->receiveResponses.first().value("protocol").toString(),
                                  this->receiveResponses.first().value("informations").toMap())))
        this->receiveResponses.pop_front();
    return (run);
}

void            Client::disconnect()
{
    SmartMutex  mutex(this->mutex, "Client", "disconnect");

    if (!mutex || this->disconnecting)
        return ;
    // If the client is not running, a new task can be created to disconnect it.
    // Otherwise it will be disconnected after the current task is completed.
    this->finish = true;
    if (!this->running)
        this->_newTask(Client::DISCONNECT);
}

bool        Client::doRead(QByteArray &data)
{
    QPair<QString, LightBird::IDoRead *> instance;

    data.clear();
    if ((instance = Plugins::instance()->getInstance<LightBird::IDoRead>(this->mode, this->transport, this->protocols, this->port)).second)
    {
        Log::trace("Calling IDoRead::doRead()", Properties("id", this->id).add("plugin", instance.first), "Client", "doRead");
        if (!instance.second->doRead(*this, data))
            Log::debug("IDoRead::doRead() returned false", Properties("id", this->id).add("plugin", instance.first).add("dataSize", data.size()), "Client", "doRead");
        Plugins::instance()->release(instance.first);
        return (true);
    }
    return (false);
}

bool        Client::doWrite(QByteArray &data)
{
    QPair<QString, LightBird::IDoWrite *> instance;

    if ((instance = Plugins::instance()->getInstance<LightBird::IDoWrite>(this->mode, this->transport, this->protocols, this->port)).second)
    {
        Log::trace("Calling IDoWrite::doWrite()", Properties("id", this->id).add("plugin", instance.first), "Client", "doWrite");
        if (!instance.second->doWrite(*this, data))
            Log::debug("IDoWrite::doWrite() returned false", Properties("id", this->id).add("plugin", instance.first).add("dataSize", data.size()), "Client", "doWrite");
        Plugins::instance()->release(instance.first);
        return (true);
    }
    return (false);
}

bool        Client::_onConnect()
{
    bool    accept = true;

    // Get the instances of all connected plugins that implements LightBird::IOnConnect and corresponds to the context
    QMapIterator<QString, LightBird::IOnConnect *> it(Plugins::instance()->getInstances<LightBird::IOnConnect>(this->mode, this->transport, this->protocols, this->port));
    while (it.hasNext())
    {
        Log::trace("Calling IOnConnect::onConnect()", Properties("id", this->id).add("plugin", it.peekNext().key()), "Client", "_onConnect");
        // Calls onConnect
        if (!it.peekNext().value()->onConnect(*this))
        {
            Log::trace("IOnConnect::onConnect() returned false. The client will be disconnected", Properties("id", this->id).add("plugin", it.peekNext().key()), "Client", "_onConnect");
            accept = false;
        }
        // Every plugins in the map must be released
        Plugins::instance()->release(it.next().key());
    }
    // If at least one onConnect call returned false, _onConnect return false, and the client will be disconnected
    return (accept);
}

bool        Client::_onDisconnect()
{
    bool    finish = true;

    QMapIterator<QString, LightBird::IOnDisconnect *> it(Plugins::instance()->getInstances<LightBird::IOnDisconnect>(this->mode, this->transport, this->protocols, this->port));
    while (it.hasNext())
    {
        Log::trace("Calling IOnDisconnect::onDisconnect()", Properties("id", this->id).add("plugin", it.peekNext().key()), "Client", "_onDisconnect");
        if (!it.peekNext().value()->onDisconnect(*this))
            finish = false;
        Plugins::instance()->release(it.next().key());
    }
    Log::info("Client disconnected", Properties("id", this->id).add("disconnecting", !finish), "Client", "run");
    return (finish);
}

void        Client::_onDestroy()
{
    QMapIterator<QString, LightBird::IOnDestroy *> it(Plugins::instance()->getInstances<LightBird::IOnDestroy>(this->mode, this->transport, this->protocols, this->port));
    while (it.hasNext())
    {
        Log::trace("Calling IOnDestroy::onDestroy()", Properties("id", this->id).add("plugin", it.peekNext().key()), "Client", "_onDestroy");
        it.peekNext().value()->onDestroy(*this);
        Plugins::instance()->release(it.next().key());
    }
}

bool        Client::isFinished() const
{
    return (this->disconnected);
}

const QString   &Client::getId() const
{
    return (this->id);
}

unsigned short  Client::getPort() const
{
    return (this->port);
}

const QStringList   &Client::getProtocols() const
{
    return (this->protocols);
}

LightBird::INetwork::Transport Client::getTransport() const
{
    return (this->transport);
}

int             Client::getSocketDescriptor() const
{
    return (this->socketDescriptor);
}

const QHostAddress  &Client::getPeerAddress() const
{
    return (this->peerAddress);
}

unsigned short  Client::getPeerPort() const
{
    return (this->peerPort);
}

const QString   &Client::getPeerName() const
{
    return (this->peerName);
}

const QDateTime &Client::getConnectionDate() const
{
    return (this->connectionDate);
}

LightBird::IClient::Mode Client::getMode() const
{
    return (this->mode);
}

QAbstractSocket &Client::getSocket()
{
    return (*this->socket);
}

QVariantMap     &Client::getInformations()
{
    return (this->informations);
}

LightBird::ITableAccounts &Client::getAccount()
{
    return (this->account);
}

LightBird::IRequest     &Client::getRequest()
{
    return (this->engine->getRequest());
}

LightBird::IResponse    &Client::getResponse()
{
    return (this->engine->getResponse());
}

QStringList Client::getSessions(const QString &id_account) const
{
    return (ApiSessions::instance()->getSessions(id_account, this->getId()));
}

LightBird::Session      Client::getSession(const QString &id_account) const
{
    QStringList         id;
    LightBird::Session  session;

    if (!(id = ApiSessions::instance()->getSessions(id_account, this->getId())).isEmpty())
        session = ApiSessions::instance()->getSession(id.first());
    return (session);
}

bool                    Client::isDisconnecting() const
{
    return (this->disconnecting);
}

void    Client::setPort(unsigned short port)
{
    this->port = port;
}

QString         Client::getProtocol(QString protocol)
{
    // If the protocol is defined we check that it is in the protocols handled by the client
    if (!protocol.isEmpty() && !this->protocols.contains("all") && !this->protocols.contains(protocol))
    {
        Log::debug("The protocol is not handled by the client", Properties("id", this->id).add("protocol", protocol), "Clients", "send");
        return (QString());
    }
    // Otherwise the protocol is the first in the protocols list
    if (protocol.isEmpty() && !this->protocols.isEmpty() && !this->protocols.contains("all"))
        protocol = this->protocols.first();
    // No protocol has been found
    if (protocol.isEmpty())
    {
        Log::debug("No protocol defined for the request", Properties("id", this->id), "Clients", "send");
        return (QString());
    }
    return (protocol);
}

void            Client::getInformations(LightBird::INetwork::Client &client, Future<bool> *future)
{
    SmartMutex  mutex(this->mutex, "Client", "getInformations");

    if (!mutex)
        return ;
    // If the client is not running or is running in the thread that requested the informations,
    // it is safe to get the informations directly
    if (!this->running || this->getThread() == QThread::currentThread())
        this->_getInformations(client, future);
    // Otherwise, the client will returns the informations as soon as possible
    else
        this->informationsRequests.insert(future, &client);
}

void    Client::_getInformations()
{
    QMapIterator<Future<bool> *, LightBird::INetwork::Client *> it(this->informationsRequests);

    while (it.hasNext())
    {
        it.next();
        this->_getInformations(*it.value(), it.key());
    }
    this->informationsRequests.clear();
}

void    Client::_getInformations(LightBird::INetwork::Client &client, Future<bool> *future)
{
    client.transport = this->transport;
    client.protocols = this->protocols;
    client.port = this->port;
    client.socketDescriptor = this->socketDescriptor;
    client.peerAddress = this->peerAddress;
    client.peerPort = this->peerPort;
    client.peerName = this->peerName;
    client.connectionDate = this->connectionDate;
    client.idAccount = this->account.getId();
    client.informations = this->getInformations();
    future->setResult(true);
    delete future;
}

void            Client::_finish()
{
    this->_onDestroy();
    this->disconnected = true;
    emit this->finished();
}
