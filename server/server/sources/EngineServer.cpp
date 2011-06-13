#include "IOnRead.h"
#include "IOnProtocol.h"
#include "IDoUnserializeHeader.h"
#include "IDoUnserializeContent.h"
#include "IDoUnserializeFooter.h"
#include "IDoExecution.h"
#include "IOnExecution.h"
#include "IDoSerializeHeader.h"
#include "IDoSerializeContent.h"
#include "IDoSerializeFooter.h"
#include "IOnWrite.h"
#include "IOnFinish.h"

#include "EngineServer.h"
#include "Plugins.hpp"

EngineServer::EngineServer(Client &client, QObject *parent) : Engine(client, parent)
{
    // Connect the Engine's signals/slots
    QObject::connect(this, SIGNAL(onProtocol()), this, SLOT(_onProtocol()), Qt::QueuedConnection);
    QObject::connect(this, SIGNAL(doUnserializeHeader()), this, SLOT(_doUnserializeHeader()), Qt::QueuedConnection);
    QObject::connect(this, SIGNAL(doUnserializeContent()), this, SLOT(_doUnserializeContent()), Qt::QueuedConnection);
    QObject::connect(this, SIGNAL(doUnserializeFooter()), this, SLOT(_doUnserializeFooter()), Qt::QueuedConnection);
    QObject::connect(this, SIGNAL(doExecution()), this, SLOT(_doExecution()), Qt::QueuedConnection);
    QObject::connect(this, SIGNAL(onExecution()), this, SLOT(_onExecution()), Qt::QueuedConnection);
    QObject::connect(this, SIGNAL(doSerializeHeader()), this, SLOT(_doSerializeHeader()), Qt::QueuedConnection);
    QObject::connect(this, SIGNAL(doSerializeContent()), this, SLOT(_doSerializeContent()), Qt::QueuedConnection);
    QObject::connect(this, SIGNAL(doSerializeFooter()), this, SLOT(_doSerializeFooter()), Qt::QueuedConnection);
}

EngineServer::~EngineServer()
{
    Log::trace("EngineServer destroyed!", Properties("id", this->client.getId()), "EngineServer", "~EngineServer");
}

void    EngineServer::read(QByteArray &data)
{
    this->_onRead(data);
    this->data.append(data);
    if (!this->running)
    {
        this->running = true;
        if (this->state == Engine::READY)
            emit this->onProtocol();
        else if (this->state == Engine::HEADER)
            emit this->doUnserializeHeader();
        else if (this->state == Engine::CONTENT)
            emit this->doUnserializeContent();
        else if (this->state == Engine::FOOTER)
            emit this->doUnserializeFooter();
    }
}

bool    EngineServer::isRunning()
{
    return (this->running);
}

LightBird::IRequest &EngineServer::getRequest()
{
    return (this->request);
}

LightBird::IResponse &EngineServer::getResponse()
{
    return (this->response);
}

void    EngineServer::_onRead(QByteArray &data)
{
    QMapIterator<QString, LightBird::IOnRead *> it(Plugins::instance()->getInstances<LightBird::IOnRead>(this->client.getMode(), this->client.getTransport(), this->client.getProtocols(), this->client.getPort()));

    while (it.hasNext())
    {
        Log::trace("Calling IOnRead::onRead()", Properties("id", this->client.getId()).add("plugin", it.peekNext().key()), "EngineServer", "_onRead");
        it.peekNext().value()->onRead(this->client, data);
        Plugins::instance()->release(it.next().key());
    }
}

void        EngineServer::_onProtocol()
{
    QString protocol;
    bool    unknow;
    bool    result = false;

    QMap<QString, LightBird::IOnProtocol *> plugins = Plugins::instance()->getInstances<LightBird::IOnProtocol>(this->client.getMode(), this->client.getTransport(), this->client.getProtocols(), this->client.getPort());
    QMapIterator<QString, LightBird::IOnProtocol *> it(plugins);
    if (!it.hasNext())
        Log::trace("No plugin implempents IOnProtocol for this context", Properties("id", this->client.getId()), "EngineServer", "_onProtocol");
    while (it.hasNext() && !result)
        if (!this->protocolUnknow.contains(it.next().key()))
        {
            protocol.clear();
            unknow = false;
            Log::trace("Calling IOnProtocol::onProtocol()", Properties("id", this->client.getId()).add("plugin", it.key()), "EngineServer", "_onProtocol");
            result = it.value()->onProtocol(this->client, this->data, protocol, unknow);
            Plugins::instance()->release(it.key());
            // The protocol of the request has been found
            if (result)
            {
                // Check the protocol returned
                if (this->client.getProtocols().contains(protocol) ||
                    this->client.getProtocols().contains("all", Qt::CaseInsensitive))
                {
                    this->request.setProtocol(protocol);
                    this->state = Engine::HEADER;
                    Log::trace("Protocol found", Properties("id", this->client.getId()).add("plugin", it.key()).add("protocol", protocol), "EngineServer", "_onProtocol");
                }
                else
                {
                    Log::warning("Invalid protocol", Properties("id", this->client.getId()).add("plugin", it.key()).add("protocol", protocol), "EngineServer", "_onProtocol");
                    unknow = true;
                }
            }
            // The plugin doesn't know the protocol
            if (unknow)
                this->protocolUnknow << it.key();
        }
        else
            Plugins::instance()->release(it.key());
    // If there is no more plugins that can find the protocol, the data are cleared
    if (this->protocolUnknow.size() >= plugins.size())
    {
        this->clear();
        this->data.clear();
        this->client.read();
    }
    else
    {
        // Go to the next step
        if (this->state == Engine::HEADER)
            this->doUnserializeHeader();
        // Otherwise the engine waits for more data
        else
            this->running = false;
    }
}

void    EngineServer::_doUnserializeHeader()
{
    quint64 used = 0;
    bool    result;
    QPair<QString, LightBird::IDoUnserializeHeader *> instance;

    // If a plugin matches
    if ((instance = Plugins::instance()->getInstance<LightBird::IDoUnserializeHeader>(this->client.getMode(), this->client.getTransport(), this->request.getProtocol(), this->client.getPort())).second)
    {
        Log::trace("Calling IDoUnserializeHeader::doUnserializeHeader()", Properties("id", this->client.getId()).add("plugin", instance.first), "EngineServer", "_doUnserializeHeader");
        result = instance.second->doUnserializeHeader(this->client, this->data, used);
        Plugins::instance()->release(instance.first);
        // If true is returned, we go to the next step
        if (result)
        {
            // Removes the bytes used
            if (used < (quint64)this->data.size() && used)
                this->data = this->data.right(this->data.size() - used);
            // All the bytes has been used
            else if (used)
                this->data.clear();
            if (Log::instance()->isTrace())
                Log::trace("Header complete", Properties("id", this->client.getId()).add("plugin", instance.first).add("used", used), "EngineServer", "_doUnserializeHeader");
            // Calls onUnserialize
            this->_onUnserialize(LightBird::IOnUnserialize::IDoUnserializeHeader);
            this->state = Engine::CONTENT;
        }
        else
            // All the data has been used, but the header is not complete
            this->data.clear();
        // If the data have been used
        if (!result || used)
            this->done = true;
    }
    // If no plugin matches, we go to the next step
    else
    {
        Log::trace("No plugin implempents IDoUnserializeHeader for this context", Properties("id", this->client.getId()), "EngineServer", "_doUnserializeHeader");
        this->state = Engine::CONTENT;
    }
    // Go to the next step
    if (this->state == Engine::CONTENT)
        this->doUnserializeContent();
    // Otherwise the engine waits for more data
    else
        this->running = false;
}

void    EngineServer::_doUnserializeContent()
{
    quint64 used = 0;
    bool    result;
    QPair<QString, LightBird::IDoUnserializeContent *> instance;

    // If a plugin matches
    if ((instance = Plugins::instance()->getInstance<LightBird::IDoUnserializeContent>(this->client.getMode(), this->client.getTransport(), this->request.getProtocol(), this->client.getPort())).second)
    {
        Log::trace("Calling IDoUnserializeContent::doUnserializeContent()", Properties("id", this->client.getId()).add("plugin", instance.first), "EngineServer", "_doUnserializeContent");
        result = instance.second->doUnserializeContent(this->client, this->data, used);
        Plugins::instance()->release(instance.first);
        // If true is returned, we go to the next step
        if (result)
        {
            // Removes the bytes used
            if (used < (quint64)this->data.size() && used)
                this->data = this->data.right(this->data.size() - used);
            // All the bytes has been used
            else if (used)
                this->data.clear();
            if (Log::instance()->isTrace())
                Log::trace("Content complete", Properties("id", this->client.getId()).add("plugin", instance.first).add("used", used), "EngineServer", "_doUnserializeContent");
            this->state = Engine::FOOTER;
        }
        else
            // All the data has been used, but the content is not complete
            this->data.clear();
        // Calls onUnserialize
        this->_onUnserialize(LightBird::IOnUnserialize::IDoUnserializeContent);
        // If the data have been used
        if (!result || used)
            this->done = true;
    }
    // If no plugin matches, we go to the next step
    else
    {
        Log::trace("No plugin implempents IDoUnserializeContent for this context", Properties("id", this->client.getId()), "EngineServer", "_doUnserializeContent");
        this->state = Engine::FOOTER;
    }
    // Go to the next step
    if (this->state == Engine::FOOTER)
        this->doUnserializeFooter();
    // Otherwise the engine waits for more data
    else if (this->data.isEmpty())
        this->running = false;
}

void    EngineServer::_doUnserializeFooter()
{
    quint64 used = 0;
    bool    result;
    QPair<QString, LightBird::IDoUnserializeFooter *> instance;

    // If a plugin matches
    if ((instance = Plugins::instance()->getInstance<LightBird::IDoUnserializeFooter>(this->client.getMode(), this->client.getTransport(), this->request.getProtocol(), this->client.getPort())).second)
    {
        Log::trace("Calling IDoUnserializeFooter::doUnserializeFooter()", Properties("id", this->client.getId()).add("plugin", instance.first), "EngineServer", "_doUnserializeFooter");
        result = instance.second->doUnserializeFooter(this->client, this->data, used);
        Plugins::instance()->release(instance.first);
        // If true is returned, we go to the next step
        if (result)
        {
            // Removes the bytes used
            if (used < (quint64)this->data.size() && used)
                this->data = this->data.right(this->data.size() - used);
            // All the bytes has been used
            else if (used)
                this->data.clear();
            if (Log::instance()->isTrace())
                Log::trace("Footer complete", Properties("id", this->client.getId()).add("plugin", instance.first).add("used", used), "EngineServer", "_doUnserializeFooter");
            // Calls onUnserialize
            this->_onUnserialize(LightBird::IOnUnserialize::IDoUnserializeFooter);
            this->state = Engine::HEADER;
        }
        else
            // All the data has been used, but the footer is not complete
            this->data.clear();
        // If the data have been used
        if (!result || used)
            this->done = true;
    }
    // If no plugin matches, we go to the next step
    else
    {
        Log::trace("No plugin implempents IDoUnserializeFooter for this context", Properties("id", this->client.getId()), "EngineServer", "_doUnserializeFooter");
        this->state = Engine::HEADER;
    }
    // If the request has been unserialized
    if (this->state == Engine::HEADER)
    {
        // If the data has never been unserialize in header, content, or footer, they are cleared
        if (!this->done)
        {
            Log::warning("The data has not been unserialized because no plugin implements IDoUnserialize* for this context, or the data are never used. The data has been cleared",
                         Properties("id", this->client.getId()), "EngineServer", "_doUnserializeFooter");
            this->clear();
            this->data.clear();
            this->client.read();
        }
        // If the request has been unserialized, it is executed
        else
        {
            if (Log::instance()->isDebug())
                Log::debug("Request complete", Properties("id", this->client.getId()), "EngineServer", "_doUnserializeFooter");
            // Calls onUnserialize
            this->_onUnserialize(LightBird::IOnUnserialize::IDoUnserialize);
            // Execute the unserialized request if there is no error
            if (!this->request.isError())
                this->doExecution();
            // Otherwise the response, which may contains the error, is directly sent
            else
            {
                Log::debug("An error has been found in the request", Properties("id", this->client.getId()), "EngineServer", "_doUnserializeFooter");
                emit this->doSerializeHeader();
            }
        }
        this->done = false;
    }
    // Otherwise the engine waits for more data
    else if (this->data.isEmpty())
        this->running = false;
}

void    EngineServer::_onUnserialize(LightBird::IOnUnserialize::Unserialize type)
{
    QMapIterator<QString, LightBird::IOnUnserialize *> it(Plugins::instance()->getInstances<LightBird::IOnUnserialize>(this->client.getMode(), this->client.getTransport(), this->request.getProtocol(), this->client.getPort()));
    while (it.hasNext())
    {
        Log::trace("Calling IOnUnserialize::onUnserialize()", Properties("id", this->client.getId()).add("plugin", it.peekNext().key()), "EngineServer", "_onUnserialize");
        it.peekNext().value()->onUnserialize(this->client, type);
        Plugins::instance()->release(it.next().key());
    }
}

void        EngineServer::_doExecution()
{
    QPair<QString, LightBird::IDoExecution *> instance;

    this->needResponse = true;
    if ((instance = Plugins::instance()->getInstance<LightBird::IDoExecution>(this->client.getMode(), this->client.getTransport(), this->request.getProtocol(), this->client.getPort(), this->request.getMethod(), this->request.getType(), true)).second)
    {
        Log::trace("Calling IDoExecution::doExecution()", Properties("id", this->client.getId()).add("plugin", instance.first), "EngineServer", "_doExecution");
        if (!(this->needResponse = instance.second->doExecution(this->client)))
            Log::trace("IDoExecution::doExecution() returned false", Properties("id", this->client.getId()).add("plugin", instance.first), "EngineServer", "_doExecution");
        Plugins::instance()->release(instance.first);
    }
    else
        Log::trace("No plugin implempents IDoExecution for this context", Properties("id", this->client.getId()), "EngineServer", "_doExecution");
    this->onExecution();
}

void        EngineServer::_onExecution()
{
    QMapIterator<QString, LightBird::IOnExecution *> it(Plugins::instance()->getInstances<LightBird::IOnExecution>(this->client.getMode(), this->client.getTransport(), this->request.getProtocol(), this->client.getPort()));
    while (it.hasNext())
    {
        Log::trace("Calling IOnExecution::onExecution()", Properties("id", this->client.getId()).add("plugin", it.peekNext().key()), "EngineServer", "_onExecution");
        if (!it.peekNext().value()->onExecution(this->client))
        {
            this->needResponse = false;
            Log::trace("IOnExecution::onExecution() returned false", Properties("id", this->client.getId()).add("plugin", it.peekNext().key()), "EngineServer", "_onExecution");
        }
        Plugins::instance()->release(it.next().key());
    }
    // If a response to the request is needed (if doExecution and onExecutions always returned true)
    if (this->needResponse)
        emit this->doSerializeHeader();
    else
    {
        this->_onFinish();
        this->clear();
        // If there are pending data, they are processed
        if (this->data.size())
            emit this->onProtocol();
        // Otherwise, we tell the client that it can send more data if possible
        else
            this->client.read();
    }
}

void    EngineServer::_onSerialize(LightBird::IOnSerialize::Serialize type)
{
    QMapIterator<QString, LightBird::IOnSerialize *> it(Plugins::instance()->getInstances<LightBird::IOnSerialize>(this->client.getMode(), this->client.getTransport(), this->request.getProtocol(), this->client.getPort()));
    while (it.hasNext())
    {
        Log::trace("Calling IOnSerialize::onSerialize()", Properties("id", this->client.getId()).add("plugin", it.peekNext().key()), "EngineServer", "_onSerialize");
        it.peekNext().value()->onSerialize(this->client, type);
        Plugins::instance()->release(it.next().key());
    }
}

void    EngineServer::_doSerializeHeader()
{
    QPair<QString, LightBird::IDoSerializeHeader *> instance;
    QByteArray  data;

    this->done = false;
    this->_onSerialize(LightBird::IOnSerialize::IDoSerialize);
    if ((instance = Plugins::instance()->getInstance<LightBird::IDoSerializeHeader>(this->client.getMode(), this->client.getTransport(), this->request.getProtocol(), this->client.getPort())).second)
    {
        this->_onSerialize(LightBird::IOnSerialize::IDoSerializeHeader);
        Log::trace("Calling IDoSerializeHeader::doSerializeHeader()", Properties("id", this->client.getId()).add("plugin", instance.first), "EngineServer", "_doSerializeHeader");
        instance.second->doSerializeHeader(this->client, data);
        Plugins::instance()->release(instance.first);
        if (data.size())
        {
            this->_onWrite(data);
            this->client.write(data);
            this->done = true;
        }
    }
    else
        Log::trace("No plugin implempents IDoSerializeHeader for this context", Properties("id", this->client.getId()), "EngineServer", "_doSerializeHeader");
    emit this->doSerializeContent();
}

void    EngineServer::_doSerializeContent()
{
    QPair<QString, LightBird::IDoSerializeContent *> instance;
    QByteArray  data;
    bool        result = true;

    if ((instance = Plugins::instance()->getInstance<LightBird::IDoSerializeContent>(this->client.getMode(), this->client.getTransport(), this->request.getProtocol(), this->client.getPort())).second)
    {
        this->_onSerialize(LightBird::IOnSerialize::IDoSerializeContent);
        Log::trace("Calling IDoSerializeContent::doSerializeContent()", Properties("id", this->client.getId()).add("plugin", instance.first), "EngineServer", "_doSerializeContent");
        if ((result = instance.second->doSerializeContent(this->client, data)))
            Log::trace("Content serialized", Properties("id", this->client.getId()).add("plugin", instance.first), "EngineServer", "_doSerializeContent");
        Plugins::instance()->release(instance.first);
        if (data.size())
        {
            this->_onWrite(data);
            this->client.write(data);
            this->done = true;
        }
        // There is no content to serialize
        else
            result = true;
    }
    else
        Log::trace("No plugin implempents IDoSerializeContent for this context", Properties("id", this->client.getId()), "EngineServer", "_doSerializeContent");
    // The content has been serialized
    if (result)
        emit this->doSerializeFooter();
    // There is more data to serialize
    else
        emit this->doSerializeContent();
}

void    EngineServer::_doSerializeFooter()
{
    QPair<QString, LightBird::IDoSerializeFooter *> instance;
    QByteArray  data;

    if ((instance = Plugins::instance()->getInstance<LightBird::IDoSerializeFooter>(this->client.getMode(), this->client.getTransport(), this->request.getProtocol(), this->client.getPort())).second)
    {
        this->_onSerialize(LightBird::IOnSerialize::IDoSerializeFooter);
        Log::trace("Calling IDoSerializeFooter::doSerializeFooter()", Properties("id", this->client.getId()).add("plugin", instance.first), "EngineServer", "_doSerializeFooter");
        instance.second->doSerializeFooter(this->client, data);
        Plugins::instance()->release(instance.first);
        if (data.size())
        {
            this->_onWrite(data);
            this->client.write(data);
            this->done = true;
        }
    }
    else
        Log::trace("No plugin implempents IDoSerializeFooter for this context", Properties("id", this->client.getId()), "EngineServer", "_doSerializeFooter");
    if (this->done)
        this->_onFinish();
    else
        Log::warning("The data has not been serialized because no plugin implements IDoSerialize* for this context.", Properties("id", this->client.getId()), "EngineServer", "_doSerializeFooter");
    this->clear();
    // If there are pending data, they are processed
    if (this->data.size())
        emit this->onProtocol();
    // Otherwise, we tell the client that it can send more data if possible
    else
        this->client.read();
}

void    EngineServer::_onWrite(QByteArray &data)
{
    QMapIterator<QString, LightBird::IOnWrite *> it(Plugins::instance()->getInstances<LightBird::IOnWrite>(this->client.getMode(), this->client.getTransport(), this->client.getProtocols(), this->client.getPort()));
    while (it.hasNext())
    {
        Log::trace("Calling IOnWrite::onWrite()", Properties("id", this->client.getId()).add("plugin", it.peekNext().key()).add("size", data.size()), "EngineServer", "_onWrite");
        it.peekNext().value()->onWrite(this->client, data);
        Plugins::instance()->release(it.next().key());
    }
}

void    EngineServer::_onFinish()
{
    QMapIterator<QString, LightBird::IOnFinish *> it(Plugins::instance()->getInstances<LightBird::IOnFinish>(this->client.getMode(), this->client.getTransport(), this->client.getProtocols(), this->client.getPort()));
    while (it.hasNext())
    {
        Log::trace("Calling IOnFinish::onFinish()", Properties("id", this->client.getId()).add("plugin", it.peekNext().key()).add("size", data.size()), "EngineServer", "_onFinish");
        it.peekNext().value()->onFinish(this->client);
        Plugins::instance()->release(it.next().key());
    }
}
