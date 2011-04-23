#include <QUuid>
#include <QHostAddress>
#include <QCoreApplication>

#include "Log.h"
#include "Client.h"
#include "Threads.h"
#include "Plugins.hpp"
#include "Engine.h"

#include "IOnConnect.h"
#include "IDoRead.h"
#include "IDoWrite.h"
#include "IOnDisconnect.h"

Client::Client(QAbstractSocket *s, LightBird::INetwork::Transports t, const QStringList &pr,
               unsigned short p, int sd, const QHostAddress &pa, unsigned short pp,
               const QString &pn, IReadWrite *r) :
               transport(t), protocols(pr), port(p), socketDescriptor(sd), peerAddress(pa),
               peerPort(pp), peerName(pn), readWriteInterface(r), socket(s)
{
    // Generates the uuid of the client
    this->id = QUuid::createUuid().toString().remove(0, 1).remove(36, 1);
    this->data = NULL;
    // Set the connection date at the current date
    this->connectionDate = QDateTime::currentDateTime();
    // This signal ensure that data are read in the client thread
    QObject::connect(this, SIGNAL(readSignal()), this, SLOT(read()));
    // Start the client thread
    this->moveToThread(this);
    Threads::instance()->newThread(this, false);
    this->socket->moveToThread(this);
}

Client::~Client()
{
    if (this->data)
        delete this->data;
    Log::trace("Client destroyed!", Properties("id", this->id), "Client", "~Client");
}

void        Client::run()
{
    QString socketDescriptor;

    // Creates the engine of the client
    this->engine = new Engine(this, this);
    // Connect the getInformations signal
    QObject::connect(this, SIGNAL(getInformationsSignal(LightBird::INetwork::Client*,Future<bool>*)),
                     this, SLOT(_getInformations(LightBird::INetwork::Client*,Future<bool>*)));
    if (this->socketDescriptor >= 0)
        socketDescriptor = QString::number(this->socketDescriptor);
    Log::info("Client connected", Properties("id", this->id).add("port", this->port)
              .add("socket", socketDescriptor, false).add("peerAddress", this->peerAddress.toString())
              .add("peerName", this->peerName, false).add("peerPort", this->peerPort), "Client", "run");
    // Enter in the Client event loop, if onConnect returns true. Otherwise, the client is disconnected
    if (this->_onConnect())
    {
        // Check if there is something to read
        this->read();
        // Enter in the event loop
        this->exec();
    }
    this->_onDisconnect();
    Log::info("Client disconnected", Properties("id", this->id).add("port", this->port)
              .add("socket", socketDescriptor, false).add("peerAddress", this->peerAddress.toString())
              .add("peerName", this->peerName, false).add("peerPort", this->peerPort), "Client", "run");
    delete this->socket;
    // Destroy the engine
    delete this->engine;
    // Nothing must be read from the client after its disconnection
    this->disconnect(SIGNAL(readSignal()));
    // Move the object to the main thread, so that it can be deleted after beeing finished
    this->moveToThread(QCoreApplication::instance()->thread());
}

void            Client::read(QByteArray *newData)
{
    QByteArray  *data = NULL;
    bool        emitReadSignal = true;

    // If the parameter d is not NULL, it contains new data that are added to this->data
    if (newData)
    {
        if (!this->lockData.tryLock(MAXTRYLOCK))
        {
            Log::error("Deadlock", "Client", "read");
            return ;
        }
        // If data is not NULL, the new data are append
        if (this->data)
        {
            this->data->append(*newData);
            delete newData;
            // A signal has already been emited to process this->data...
            emitReadSignal = false;
        }
        else
            this->data = newData;
        this->lockData.unlock();
    }
    // If the current thread is not the thread of the client, the thread is changed using readSignal
    if (this->currentThread() != this)
    {
        if (emitReadSignal)
            emit readSignal();
        return ;
    }
    // If there are pending data, we use it
    if (!this->lockData.tryLock(MAXTRYLOCK))
    {
        Log::error("Deadlock", "Client", "read");
        return ;
    }
    if (this->data)
    {
        data = this->data;
        this->data = NULL;
    }
    this->lockData.unlock();
    // If data is NULL and the engine is not running, the data are read using readWriteInterface
    if (data == NULL)
    {
        data = new QByteArray();
        if (!this->engine->isRunning())
            this->readWriteInterface->read(*data, this);
    }
    // If there is no data, no processing are needed
    if (data->isEmpty())
    {
        if (Log::instance()->isTrace())
            Log::trace("No data read", Properties("id", this->id), "Client", "read");
        delete data;
        return ;
    }
    if (Log::instance()->isTrace())
        Log::trace("Data received", Properties("id", this->id).add("data", this->_simplified(*data)).add("size", data->size()), "Client", "read");
    else if (Log::instance()->isDebug())
        Log::debug("Data received", Properties("id", this->id).add("size", data->size()), "Client", "read");
    // Execution of the request
    this->engine->read(*data);
    delete data;
}

void            Client::write(QByteArray &data)
{
    // Writes the data
    if (this->readWriteInterface->write(data, this))
    {
        if (Log::instance()->isTrace())
            Log::trace("Data written", Properties("id", this->id).add("data", this->_simplified(data)).add("size", data.size()), "Client", "write");
        else if (Log::instance()->isDebug())
            Log::trace("Data written", Properties("id", this->id).add("size", data.size()), "Client", "write");
    }
    // If an error occured
    else
        Log::warning("An error occured while writing data", Properties("id", this->id).add("size", data.size()), "Client", "read");
}

bool        Client::_onConnect()
{
    bool    accept = true;

    // Get the instances of all connected plugins that implements LightBird::IOnConnect and corresponds to the context
    QMapIterator<QString, LightBird::IOnConnect *> it(Plugins::instance()->getInstances<LightBird::IOnConnect>(this->transport, this->protocols, this->port));
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

bool        Client::doRead(QByteArray &data)
{
    QPair<QString, LightBird::IDoRead *> instance;

    data.clear();
    // If a plugin matches
    if ((instance = Plugins::instance()->getInstance<LightBird::IDoRead>(this->transport, this->protocols, this->port)).second)
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

    // If a plugin matches
    if ((instance = Plugins::instance()->getInstance<LightBird::IDoWrite>(this->transport, this->protocols, this->port)).second)
    {
        Log::trace("Calling IDoWrite::doWrite()", Properties("id", this->id).add("plugin", instance.first), "Client", "doWrite");
        if (!instance.second->doWrite(*this, data))
            Log::debug("IDoWrite::doWrite() returned false", Properties("id", this->id).add("plugin", instance.first).add("dataSize", data.size()), "Client", "doWrite");
        Plugins::instance()->release(instance.first);
        return (true);
    }
    return (false);
}

void        Client::_onDisconnect()
{
    QMapIterator<QString, LightBird::IOnDisconnect *> it(Plugins::instance()->getInstances<LightBird::IOnDisconnect>(this->transport, this->protocols, this->port));

    while (it.hasNext())
    {
        Log::trace("Calling IOnDisconnect::onDisconnect()", Properties("id", this->id).add("plugin", it.peekNext().key()), "Client", "_onDisconnect");
        it.peekNext().value()->onDisconnect(*this);
        Plugins::instance()->release(it.next().key());
    }
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

LightBird::INetwork::Transports Client::getTransport() const
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

QVariantMap     &Client::getInformations()
{
    return (this->informations);
}

QAbstractSocket &Client::getSocket() const
{
    return (*this->socket);
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

void    Client::getInformations(LightBird::INetwork::Client *client, void *thread, Future<bool> *future)
{
    // If the thread that require the informations is the client, it is safe
    if (thread == this)
        this->_getInformations(client, future);
    // Otherwise, use the signal to get the informations in the client thread
    else
        emit this->getInformationsSignal(client, future);
}

void    Client::_getInformations(LightBird::INetwork::Client *client, Future<bool> *future)
{
    client->transport = this->transport;
    client->protocols = this->protocols;
    client->port = this->port;
    client->socketDescriptor = this->socketDescriptor;
    client->peerAddress = this->peerAddress;
    client->peerPort = this->peerPort;
    client->peerName = this->peerName;
    client->connectionDate = this->connectionDate;
    client->idAccount = this->account.getId();
    client->informations = this->getInformations();
    future->setResult(true);
    delete future;
}

QByteArray Client::_simplified(QByteArray data)
{
    data = data.left(2000);
    int s = data.size();
    for (int i = 0; i < s; ++i)
        if (data.data()[i] < 32 || data.data()[i] > 126)
            data.data()[i] = '.';
    return (data);
}
