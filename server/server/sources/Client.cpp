#include <QCoreApplication>
#include <QHostAddress>
#include <QTimer>

#include "IOnConnect.h"
#include "IDoRead.h"
#include "IDoWrite.h"
#include "IOnPause.h"
#include "IOnResume.h"
#include "IOnDisconnect.h"
#include "IOnDestroy.h"

#include "ApiSessions.h"
#include "Client.h"
#include "EngineClient.h"
#include "EngineServer.h"
#include "LightBird.h"
#include "Log.h"
#include "Plugins.hpp"
#include "Mutex.h"
#include "Threads.h"

Client::Client(QAbstractSocket *s, const QStringList &pr, LightBird::INetwork::Transport t,
               unsigned short p, int sd, const QHostAddress &pa, unsigned short pp,
               const QString &pn, LightBird::IClient::Mode m, IReadWrite *r, const QStringList &c)
    : transport(t)
    , protocols(pr)
    , port(p)
    , socketDescriptor(sd)
    , peerAddress(pa)
    , peerPort(pp)
    , peerName(pn)
    , mode(m)
    , readWriteInterface(r)
    , contexts(c)
    , socket(s)
    , validator(contexts, mode, transport, protocols, port)
{
    // Generates the uuid of the client
    this->id = LightBird::createUuid();
    this->reading = false;
    this->running = false;
    this->writing = NULL;
    this->state = Client::NONE;
    this->pauseState = Pause::NONE;
    this->resumeAfterPausing = false;
    this->pauseTimer = NULL;
    this->disconnectState = Disconnect::NONE;
    this->disconnectFatal = false;
    // Sets the connection date at the current date
    this->connectionDate = QDateTime::currentDateTime();
    // Creates the engine
    if (this->mode == LightBird::IClient::SERVER)
        this->engine = new EngineServer(*this);
    else
        this->engine = new EngineClient(*this);
    // Adds a task to call the _onConnect method
    this->_newTask(Client::CONNECT);
}

Client::~Client()
{
    // Ensures that all the informations requests has been processed
    this->_getInformations();
    delete this->engine;
    LOG_TRACE("Client destroyed!", Properties("id", this->id), "Client", "~Client");
}

void    Client::run()
{
    RunState s;
    State    &newTask = s.newTask;
    bool     &send = s.send;
    bool     &receive = s.receive;

    // Performs different actions depending on the client's state
    switch (this->state)
    {
        case Client::CONNECT :
            // Tries to connect to the client
            if (!this->readWriteInterface->connect(this))
                return this->_finish();
            LOG_INFO("Client connected", Properties("id", this->id).add("port", this->port)
                     .add("socket", ((this->socketDescriptor >= 0) ? QString::number(this->socketDescriptor) : ""), false)
                     .add("peerAddress", this->peerAddress.toString()).add("peerName", this->peerName, false)
                     .add("mode", (this->getMode() == LightBird::IClient::CLIENT) ? "client" : "server")
                     .add("peerPort", this->peerPort), "Client", "run");
            // If the client is not allowed to connect, it is disconnected
            if (!this->_onConnect())
                return this->_finish();
            break;

        case Client::READING :
            // The execution is paused while data are being read
            this->reading = false;
            this->readWriteInterface->read(this);
            return ;

        case Client::READ :
            // If data are available, we try to execute them
            if (this->_read())
                newTask = Client::RUN;
            break;

        case Client::WRITTEN :
            // Resumes the workflow where it was before the writing
            newTask = this->written;
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
            // The processing is paused while data are being written on the network
            if (this->writing)
                return (this->_write(newTask));
            break;

        case Client::PAUSE :
            this->_onPause();
            if (this->_pause())
                return ;
            break ;

        case Client::RESUME :
            this->_onResume();
            s = this->runStatePause;
            break;

        case Client::DISCONNECT :
            if (this->_disconnect())
                return ;
            s = this->runStateDisconnect;
            break;

        default:
            break;
    }

    // Runs a new task
    Mutex mutex(this->mutex, "Client", "run");
    if (!mutex)
        return ;
    // Fills the pending informations requests
    if (!this->informationsRequests.isEmpty())
        this->_getInformations();
    // The network workflow is being paused
    if (this->pauseState == Pause::PAUSING)
    {
        this->runStatePause = s;
        newTask = Client::PAUSE;
    }
    // The network workflow is being resumed
    else if (this->pauseState == Pause::RESUMING)
        newTask = Client::RESUME;
    // The client must be disconnected
    else if (this->disconnectState == Disconnect::DISCONNECT)
    {
        this->runStateDisconnect = s;
        newTask = Client::DISCONNECT;
    }
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
    else if (this->reading)
        newTask = Client::READING;
    // The client is disconnecting and there is nothing left to do
    else if (this->disconnectState == Disconnect::DISCONNECTING)
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

void    Client::readyRead()
{
    Mutex   mutex(this->mutex, "Client", "readyRead");

    if (!mutex)
        return ;
    this->reading = true;
    if (!this->running)
        this->_newTask(Client::READING);
}

void    Client::bytesRead()
{
    Mutex   mutex(this->mutex, "Client", "bytesRead");

    if (!mutex || !this->running || this->state != Client::READING)
        return ;
    // All the data have not been read
    if (this->socket->size() > 0)
        this->reading = true;
    this->_newTask(Client::READ);
}

bool    Client::_read()
{
    Mutex   mutex(this->mutex, "Client", "_read");

    if (!mutex)
        return (false);
    // If there is no data, no processing is needed
    if (this->data.isEmpty())
        return (false);
    mutex.unlock();
    // Calls the IOnRead interface outside the mutex
    this->engine->onRead();
    return (true);
}

void    Client::write(QByteArray *data)
{
    if (data->size() == 0)
    {
        delete data;
        return ;
    }
    if (Log::instance()->isTrace())
        Log::trace("Writing data", Properties("id", this->id).add("data", LightBird::simplify(*data)).add("size", data->size()), "Client", "write");
    else if (Log::instance()->isDebug())
        Log::debug("Writing data", Properties("id", this->id).add("size", data->size()), "Client", "write");
    // The data will be written in Client::_write
    this->writing = data;
}

void    Client::_write(Client::State newTask)
{
    this->written = newTask;
    this->readWriteInterface->write(this->writing, this);
}

void    Client::bytesWritten()
{
    Mutex   mutex(this->mutex, "Client", "bytesWritten");

    if (mutex && this->writing)
    {
        this->writing = NULL;
        this->_newTask(Client::WRITTEN);
    }
}

bool    Client::send(const QString &protocol, const QVariantMap &informations, const QString &id)
{
    Mutex       mutex(this->mutex, "Client", "send");
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

bool    Client::_send()
{
    Mutex        mutex(this->mutex, "Client", "_send");
    EngineServer *engineServer = qobject_cast<EngineServer *>(this->engine);
    EngineClient *engineClient = qobject_cast<EngineClient *>(this->engine);
    bool         run = false;

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

bool    Client::receive(const QString &protocol, const QVariantMap &informations)
{
    Mutex       mutex(this->mutex, "Client", "receive");
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

bool    Client::_receive()
{
    Mutex        mutex(this->mutex, "Client", "_receive");
    EngineClient *engine = qobject_cast<EngineClient *>(this->engine);
    bool         run = false;

    if (!mutex)
        return (false);
    if (engine && !this->receiveResponses.isEmpty()
        && (run = engine->receive(this->receiveResponses.first().value("protocol").toString(),
                                  this->receiveResponses.first().value("informations").toMap())))
        this->receiveResponses.pop_front();
    return (run);
}

bool    Client::pause(int msec)
{
    Mutex   mutex(this->mutex, "Client", "pause");

    if (!mutex || this->pauseState != Pause::NONE)
        return (false);
    this->pauseState = Pause::PAUSING;
    this->runStatePause = RunState();
    // Creates the pause timer in the Client thread
    if (msec > 0)
    {
        this->pauseTimer = new QTimer();
        this->pauseTimer->setSingleShot(true);
        QObject::connect(this->pauseTimer, SIGNAL(timeout()), this, SLOT(resume()), Qt::QueuedConnection);
        this->pauseTimer->start(msec);
        this->pauseTimer->moveToThread(QObject::thread());
    }
    if (!this->running)
        this->_newTask(Client::PAUSE);
    return (true);
}

bool    Client::_pause()
{
    Mutex   mutex(this->mutex, "Client", "_pause");

    if (!mutex)
        return (false);
    // The network workflow has already been resumed
    if (this->resumeAfterPausing)
        this->pauseState = Pause::RESUMING;
    this->resumeAfterPausing = false;
    if (this->pauseState == Pause::RESUMING)
        return (false);
    // The workflow is paused now
    this->pauseState = Pause::PAUSED;
    // The client has been disconnected while we were pausing
    if (this->disconnectState == Disconnect::DISCONNECT)
        return (false);
    return (true);
}

void    Client::_onPause()
{
    QPair<QString, LightBird::IOnPause *> instance;

    if ((instance = Plugins::instance()->getInstance<LightBird::IOnPause>(this->getValidator(true, false))).second)
    {
        LOG_TRACE("Calling IOnPause::onPause()", Properties("id", this->id).add("plugin", instance.first), "Client", "_onPause");
        instance.second->onPause(*this);
        Plugins::instance()->release(instance.first);
    }
    this->pauseState = Pause::NONE;
}

bool    Client::resume()
{
    Mutex        mutex(this->mutex, "Client", "resume");
    Pause::State oldPauseState = this->pauseState;

    if (!mutex || this->pauseState == Pause::NONE || this->pauseState == Pause::RESUMING)
        return (false);
    if (this->pauseState != Pause::PAUSING)
        this->pauseState = Pause::RESUMING;
    // The workflow will be resumed just after PAUSING is finished
    else
        this->resumeAfterPausing = true;
    // Stops the timer if it has not timeout
    if (this->pauseTimer && this->pauseTimer->isActive())
    {
        this->pauseTimer->deleteLater();
        this->pauseTimer = NULL;
    }
    // Runs the RESUME task is we are not PAUSING and the client is not being disconnected
    if (oldPauseState == Pause::PAUSED && this->disconnectState != Disconnect::DISCONNECT && this->disconnectState != Disconnect::DISCONNECTED)
        this->_newTask(Client::RESUME);
    return (true);
}

void    Client::_onResume()
{
    QPair<QString, LightBird::IOnResume *> instance;
    bool    timeout = (this->pauseTimer != NULL && !this->pauseTimer->isActive());

    if ((instance = Plugins::instance()->getInstance<LightBird::IOnResume>(this->getValidator(true, false))).second)
    {
        LOG_TRACE("Calling IOnResume::onResume()", Properties("id", this->id).add("plugin", instance.first), "Client", "_onResume");
        instance.second->onResume(*this, timeout);
        Plugins::instance()->release(instance.first);
    }
    this->pauseState = Pause::NONE;
}

void    Client::disconnect(bool fatal)
{
    Mutex  mutex(this->mutex, "Client", "disconnect");

    if (!mutex || this->disconnectState != Disconnect::NONE)
        return ;
    this->runStateDisconnect = RunState();
    this->disconnectFatal = fatal;
    // If the client is not running, a new task can be created to disconnect it.
    // Otherwise it will be disconnected after the current task is completed.
    this->disconnectState = Disconnect::DISCONNECT;
    if (!this->running || this->pauseState == Pause::PAUSED)
        this->_newTask(Client::DISCONNECT);
}

bool    Client::_disconnect()
{
    // If IOnDisconnect returned true or the disconnection is fatal, the client is destroyed now
    if (this->_onDisconnect() || this->disconnectFatal)
    {
        this->_finish();
        return (true);
    }
    // Otherwise it will be destroyed when all the data have been processed
    Mutex  mutex(this->mutex, "Client", "_disconnect");
    this->disconnectState = Disconnect::DISCONNECTING;
    // The network workflow is paused
    if (this->pauseState == Pause::PAUSED)
        return (true);
    return (false);
}

bool    Client::doRead(QByteArray &data)
{
    QPair<QString, LightBird::IDoRead *> instance;

    data.clear();
    if ((instance = Plugins::instance()->getInstance<LightBird::IDoRead>(this->getValidator(true, false))).second)
    {
        LOG_TRACE("Calling IDoRead::doRead()", Properties("id", this->id).add("plugin", instance.first), "Client", "doRead");
        if (!instance.second->doRead(*this, data))
            LOG_DEBUG("IDoRead::doRead() returned false", Properties("id", this->id).add("plugin", instance.first).add("dataSize", data.size()), "Client", "doRead");
        Plugins::instance()->release(instance.first);
        return (true);
    }
    return (false);
}

bool    Client::doWrite(const char *data, qint64 size, qint64 &result)
{
    QPair<QString, LightBird::IDoWrite *> instance;

    if ((instance = Plugins::instance()->getInstance<LightBird::IDoWrite>(this->getValidator(true, false))).second)
    {
        LOG_TRACE("Calling IDoWrite::doWrite()", Properties("id", this->id).add("plugin", instance.first).add("size", size), "Client", "doWrite");
        result = instance.second->doWrite(*this, data, size);
        Plugins::instance()->release(instance.first);
        return (true);
    }
    return (false);
}

bool    Client::_onConnect()
{
    bool    accept = true;

    // Get the instances of all connected plugins that implements LightBird::IOnConnect and corresponds to the context
    QMapIterator<QString, LightBird::IOnConnect *> it(Plugins::instance()->getInstances<LightBird::IOnConnect>(this->getValidator(false, false)));
    while (it.hasNext())
    {
        LOG_TRACE("Calling IOnConnect::onConnect()", Properties("id", this->id).add("plugin", it.peekNext().key()), "Client", "_onConnect");
        // Calls onConnect
        if (!it.peekNext().value()->onConnect(*this))
        {
            LOG_TRACE("IOnConnect::onConnect() returned false. The client will be disconnected", Properties("id", this->id).add("plugin", it.peekNext().key()), "Client", "_onConnect");
            accept = false;
        }
        // Every plugins in the map must be released
        Plugins::instance()->release(it.next().key());
    }
    // If at least one onConnect call returned false, _onConnect return false, and the client will be disconnected
    return (accept);
}

bool    Client::_onDisconnect()
{
    bool    finish = true;

    QMapIterator<QString, LightBird::IOnDisconnect *> it(Plugins::instance()->getInstances<LightBird::IOnDisconnect>(this->getValidator(true, false)));
    while (it.hasNext())
    {
        LOG_TRACE("Calling IOnDisconnect::onDisconnect()", Properties("id", this->id).add("plugin", it.peekNext().key()), "Client", "_onDisconnect");
        if (!it.peekNext().value()->onDisconnect(*this, this->disconnectFatal))
            finish = false;
        Plugins::instance()->release(it.next().key());
    }
    LOG_INFO("Client disconnected", Properties("id", this->id).add("disconnecting", !finish).add("still connected", (this->socket->state() == QAbstractSocket::ConnectedState)).add("fatal", this->disconnectFatal), "Client", "run");
    return (finish);
}

void    Client::_onDestroy()
{
    QMapIterator<QString, LightBird::IOnDestroy *> it(Plugins::instance()->getInstances<LightBird::IOnDestroy>(this->getValidator(false, false)));
    while (it.hasNext())
    {
        LOG_TRACE("Calling IOnDestroy::onDestroy()", Properties("id", this->id).add("plugin", it.peekNext().key()), "Client", "_onDestroy");
        it.peekNext().value()->onDestroy(*this);
        Plugins::instance()->release(it.next().key());
    }
}

bool    Client::isFinished() const
{
    return (this->disconnectState == Disconnect::DISCONNECTED);
}

const QString   &Client::getId() const
{
    return (this->id);
}

QAbstractSocket &Client::getSocket()
{
    return (*this->socket);
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

int Client::getSocketDescriptor() const
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

quint64 Client::getBufferSize() const
{
    return (this->data.size() + this->socket->size());
}

LightBird::IClient::Mode Client::getMode() const
{
    return (this->mode);
}

QStringList &Client::getContexts()
{
    return (this->contexts);
}

QVariantMap &Client::getInformations()
{
    return (this->informations);
}

LightBird::TableAccounts &Client::getAccount()
{
    return (this->account);
}

LightBird::IRequest &Client::getRequest()
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

LightBird::Session  Client::getSession(const QString &id_account) const
{
    QStringList         id;
    LightBird::Session  session;

    if (!(id = ApiSessions::instance()->getSessions(id_account, this->getId())).isEmpty())
        session = ApiSessions::instance()->getSession(id.first());
    return (session);
}

bool    Client::isPaused() const
{
    return (this->pauseState == Pause::PAUSING || this->pauseState == Pause::PAUSED);
}

bool    Client::isDisconnecting() const
{
    return (this->disconnectState == Disconnect::DISCONNECTING);
}

QByteArray  &Client::getData()
{
    return (this->data);
}

void    Client::setPort(unsigned short port)
{
    this->port = port;
}

QString Client::getProtocol(QString protocol)
{
    // If the protocol is defined we check that it is in the protocols handled by the client
    if (!protocol.isEmpty() && !this->protocols.contains("all") && !this->protocols.contains(protocol))
    {
        LOG_DEBUG("The protocol is not handled by the client", Properties("id", this->id).add("protocol", protocol), "Clients", "send");
        return (QString());
    }
    // Otherwise the protocol is the first in the protocols list
    if (protocol.isEmpty() && !this->protocols.isEmpty() && !this->protocols.contains("all"))
        protocol = this->protocols.first();
    // No protocol has been found
    if (protocol.isEmpty())
    {
        LOG_DEBUG("No protocol defined for the request", Properties("id", this->id), "Clients", "send");
        return (QString());
    }
    return (protocol);
}

Context::Validator  &Client::getValidator(bool useName, bool requestProtocol, bool useMethodType)
{
    if (!useName)
        this->validator.names = NULL;
    else if (!this->validator.names)
        this->validator.names = &this->contexts;
    if (!requestProtocol)
        this->validator.protocols = &this->protocols;
    else if (this->validator.protocols == &this->protocols)
        this->validator.protocols = &this->engine->getRequestObject().getProtocols();
    if (useMethodType)
    {
        this->validator.method = &this->engine->getRequest().getMethod();
        this->validator.type = &this->engine->getRequest().getType();
    }
    else if (this->validator.method)
    {
        this->validator.method = NULL;
        this->validator.type = NULL;
    }
    return (this->validator);
}

void    Client::getInformations(LightBird::INetwork::Client &client, Future<bool> *future)
{
    Mutex   mutex(this->mutex, "Client", "getInformations");

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

void    Client::_finish()
{
    // Calls IOnResume before finishing the client if it is paused
    if (this->pauseState != Pause::NONE)
        this->_onResume();
    // Finishes the client
    this->_onDestroy();
    this->disconnectState = Disconnect::DISCONNECTED;
    emit this->finished();
}
