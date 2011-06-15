#include "IOnRead.h"
#include "IDoSend.h"
#include "IOnSend.h"
#include "IDoSerializeHeader.h"
#include "IDoSerializeContent.h"
#include "IDoSerializeFooter.h"
#include "IOnWrite.h"
#include "IOnFinish.h"
#include "IDoUnserializeHeader.h"
#include "IDoUnserializeContent.h"
#include "IDoUnserializeFooter.h"
#include "IDoExecution.h"
#include "IOnExecution.h"

#include "EngineClient.h"
#include "Plugins.hpp"

EngineClient::EngineClient(Client &client, QObject *parent) : Engine(client, parent)
{
    // Connect the Engine's signals/slots
    QObject::connect(this, SIGNAL(doSend()), this, SLOT(_doSend()), Qt::QueuedConnection);
    QObject::connect(this, SIGNAL(doSerializeHeader()), this, SLOT(_doSerializeHeader()), Qt::QueuedConnection);
    QObject::connect(this, SIGNAL(doSerializeContent()), this, SLOT(_doSerializeContent()), Qt::QueuedConnection);
    QObject::connect(this, SIGNAL(doSerializeFooter()), this, SLOT(_doSerializeFooter()), Qt::QueuedConnection);
    QObject::connect(this, SIGNAL(doUnserializeHeader()), this, SLOT(_doUnserializeHeader()), Qt::QueuedConnection);
    QObject::connect(this, SIGNAL(doUnserializeContent()), this, SLOT(_doUnserializeContent()), Qt::QueuedConnection);
    QObject::connect(this, SIGNAL(doUnserializeFooter()), this, SLOT(_doUnserializeFooter()), Qt::QueuedConnection);
    QObject::connect(this, SIGNAL(doExecution()), this, SLOT(_doExecution()), Qt::QueuedConnection);
    QObject::connect(this, SIGNAL(onExecution()), this, SLOT(_onExecution()), Qt::QueuedConnection);
}

EngineClient::~EngineClient()
{
    Log::trace("EngineClient destroyed!", Properties("id", this->client.getId()), "EngineClient", "~EngineClient");
}

void    EngineClient::read(QByteArray &data)
{
    this->_onRead(data);
    this->data.append(data);
    if (!this->running && this->state != Engine::READY)
    {
        this->running = true;
        if (this->state == Engine::HEADER)
            this->_doUnserializeHeader();
        else if (this->state == Engine::CONTENT)
            this->_doUnserializeContent();
        else if (this->state == Engine::FOOTER)
            this->_doUnserializeFooter();
    }
}

void        EngineClient::send(const QString &id, const QString &protocol)
{
    // Checks that the plugin does not have a request already pending
    QListIterator<QPair<QString, QString> > it(this->requests);
    this->requests.push_back(QPair<QString, QString>(id, protocol));
    while (it.hasNext())
        if (it.peekNext().first == id)
            this->requests.pop_back();
    // If the engine is not running a new request can be processed
    if (!this->running && this->state == Engine::READY)
        emit this->doSend();
}

bool    EngineClient::isRunning()
{
    return (this->running);
}

LightBird::IRequest &EngineClient::getRequest()
{
    return (this->request);
}

LightBird::IResponse &EngineClient::getResponse()
{
    return (this->response);
}

void    EngineClient::_onRead(QByteArray &data)
{
    QMapIterator<QString, LightBird::IOnRead *> it(Plugins::instance()->getInstances<LightBird::IOnRead>(this->client.getMode(), this->client.getTransport(), this->client.getProtocols(), this->client.getPort()));

    while (it.hasNext())
    {
        Log::trace("Calling IOnRead::onRead()", Properties("id", this->client.getId()).add("plugin", it.peekNext().key()), "EngineClient", "_onRead");
        it.peekNext().value()->onRead(this->client, data);
        Plugins::instance()->release(it.next().key());
    }
}

void        EngineClient::_doSend()
{
    bool    found = false;
    bool    result = false;

    // Checks if the engine is ready to process a new request
    if (this->state != Engine::READY || this->running || this->requests.isEmpty())
        return ;
    this->request.setProtocol(this->requests.front().second);
    // Gets the plugins that asked to send a request
    QMapIterator<QString, LightBird::IDoSend *> it(Plugins::instance()->getInstances<LightBird::IDoSend>(this->client.getMode(), this->client.getTransport(), this->request.getProtocol(), this->client.getPort()));
    while (it.hasNext())
    {
        if (!found && it.peekNext().key() == this->requests.front().first)
        {
            Log::trace("Calling IDoSend::doSend()", Properties("id", this->client.getId()).add("plugin", it.peekNext().key()), "EngineClient", "_doSend");
            result = it.peekNext().value()->doSend(this->client);
            found = true;
        }
        Plugins::instance()->release(it.next().key());
    }
    // Calls IOnSend and cancels the send if it returns false
    if (found && !this->_onSend())
        result = false;
    // Go to the next step
    if (found && result)
    {
        this->running = true;
        emit this->doSerializeHeader();
    }
    // No valid plugin found, or the send has been canceled. Tries to process an other request.
    else
    {
        if (!found)
            Log::debug("The plugin does not implempents IDoSend for this context", Properties("id", this->client.getId()).add("plugin", this->requests.front().first).add("protocol", this->requests.front().second), "EngineClient", "_doSend");
        else
            Log::debug("The send of the request has been canceled by a plugin", Properties("id", this->client.getId()).add("plugin", this->requests.front().first).add("protocol", this->requests.front().second), "EngineClient", "_doSend");
        emit this->doSend();
    }
    this->requests.pop_front();
}

bool        EngineClient::_onSend()
{
    bool    result = true;

    QMapIterator<QString, LightBird::IOnSend *> it(Plugins::instance()->getInstances<LightBird::IOnSend>(this->client.getMode(), this->client.getTransport(), this->request.getProtocol(), this->client.getPort()));
    while (it.hasNext())
    {
        Log::trace("Calling IOnSend::onSend()", Properties("id", this->client.getId()).add("plugin", it.peekNext().key()), "EngineClient", "_onSend");
        if (!it.peekNext().value()->onSend(this->client))
            result = false;
        Plugins::instance()->release(it.next().key());
    }
    return (result);
}

void    EngineClient::_doSerializeHeader()
{
    QPair<QString, LightBird::IDoSerializeHeader *> instance;
    QByteArray  data;

    this->done = false;
    if ((instance = Plugins::instance()->getInstance<LightBird::IDoSerializeHeader>(this->client.getMode(), this->client.getTransport(), this->request.getProtocol(), this->client.getPort())).second)
    {
        this->_onSerialize(LightBird::IOnSerialize::IDoSerializeHeader);
        Log::trace("Calling IDoSerializeHeader::doSerializeHeader()", Properties("id", this->client.getId()).add("plugin", instance.first), "EngineClient", "_doSerializeHeader");
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
        Log::trace("No plugin implempents IDoSerializeHeader for this context", Properties("id", this->client.getId()), "EngineClient", "_doSerializeHeader");
    emit this->doSerializeContent();
}

void    EngineClient::_doSerializeContent()
{
    QPair<QString, LightBird::IDoSerializeContent *> instance;
    QByteArray  data;
    bool        result = true;

    if ((instance = Plugins::instance()->getInstance<LightBird::IDoSerializeContent>(this->client.getMode(), this->client.getTransport(), this->request.getProtocol(), this->client.getPort())).second)
    {
        this->_onSerialize(LightBird::IOnSerialize::IDoSerializeContent);
        Log::trace("Calling IDoSerializeContent::doSerializeContent()", Properties("id", this->client.getId()).add("plugin", instance.first), "EngineClient", "_doSerializeContent");
        if ((result = instance.second->doSerializeContent(this->client, data)))
            Log::trace("Content serialized", Properties("id", this->client.getId()).add("plugin", instance.first), "EngineClient", "_doSerializeContent");
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
        Log::trace("No plugin implempents IDoSerializeContent for this context", Properties("id", this->client.getId()), "EngineClient", "_doSerializeContent");
    // The content has been serialized
    if (result)
        emit this->doSerializeFooter();
    // There is more data to serialize
    else
        emit this->doSerializeContent();
}

void    EngineClient::_doSerializeFooter()
{
    QPair<QString, LightBird::IDoSerializeFooter *> instance;
    QByteArray  data;

    if ((instance = Plugins::instance()->getInstance<LightBird::IDoSerializeFooter>(this->client.getMode(), this->client.getTransport(), this->request.getProtocol(), this->client.getPort())).second)
    {
        this->_onSerialize(LightBird::IOnSerialize::IDoSerializeFooter);
        Log::trace("Calling IDoSerializeFooter::doSerializeFooter()", Properties("id", this->client.getId()).add("plugin", instance.first), "EngineClient", "_doSerializeFooter");
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
        Log::trace("No plugin implempents IDoSerializeFooter for this context", Properties("id", this->client.getId()), "EngineClient", "_doSerializeFooter");
    // The request has been sent and we are ready to execute the response
    if (this->done)
    {
        // Calls the final IOnSerialize that define if the request needs a response
        if (this->_onSerialize(LightBird::IOnSerialize::IDoSerialize))
        {
            this->state = Engine::HEADER;
            // If there are pending data they are unserialized
            if (!this->data.isEmpty())
                this->_doUnserializeHeader();
            // Otherwise we are waiting for a response
            else
                this->running = false;
        }
        // No response reeded
        else
        {
            Log::trace("The request does not need a response. An other request is going to be processed.", Properties("id", this->client.getId()), "EngineClient", "_doSerializeFooter");
            this->_onFinish();
        }
    }
    // Tries to generate an other request
    else
    {
        Log::warning("The data has not been serialized because no plugin implements IDoSerialize* for this context.", Properties("id", this->client.getId()), "EngineClient", "_doSerializeFooter");
        this->clear();
        emit this->doSend();
    }
}

bool        EngineClient::_onSerialize(LightBird::IOnSerialize::Serialize type)
{
    bool    result = true;

    QMapIterator<QString, LightBird::IOnSerialize *> it(Plugins::instance()->getInstances<LightBird::IOnSerialize>(this->client.getMode(), this->client.getTransport(), this->request.getProtocol(), this->client.getPort()));
    while (it.hasNext())
    {
        Log::trace("Calling IOnSerialize::onSerialize()", Properties("id", this->client.getId()).add("plugin", it.peekNext().key()), "EngineClient", "_onSerialize");
        if (!it.peekNext().value()->onSerialize(this->client, type))
            result = false;
        Plugins::instance()->release(it.next().key());
    }
    return (result);
}

void    EngineClient::_onWrite(QByteArray &data)
{
    QMapIterator<QString, LightBird::IOnWrite *> it(Plugins::instance()->getInstances<LightBird::IOnWrite>(this->client.getMode(), this->client.getTransport(), this->client.getProtocols(), this->client.getPort()));
    while (it.hasNext())
    {
        Log::trace("Calling IOnWrite::onWrite()", Properties("id", this->client.getId()).add("plugin", it.peekNext().key()).add("size", data.size()), "EngineClient", "_onWrite");
        it.peekNext().value()->onWrite(this->client, data);
        Plugins::instance()->release(it.next().key());
    }
}

void    EngineClient::_doUnserializeHeader()
{
    quint64 used = 0;
    bool    result;
    QPair<QString, LightBird::IDoUnserializeHeader *> instance;

    this->done = false;
    // If a plugin matches
    if ((instance = Plugins::instance()->getInstance<LightBird::IDoUnserializeHeader>(this->client.getMode(), this->client.getTransport(), this->request.getProtocol(), this->client.getPort())).second)
    {
        Log::trace("Calling IDoUnserializeHeader::doUnserializeHeader()", Properties("id", this->client.getId()).add("plugin", instance.first), "EngineClient", "_doUnserializeHeader");
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
                Log::trace("Header complete", Properties("id", this->client.getId()).add("plugin", instance.first).add("used", used), "EngineClient", "_doUnserializeHeader");
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
        Log::trace("No plugin implempents IDoUnserializeHeader for this context", Properties("id", this->client.getId()), "EngineClient", "_doUnserializeHeader");
        this->state = Engine::CONTENT;
    }
    // Go to the next step
    if (this->state == Engine::CONTENT)
        this->_doUnserializeContent();
    // Otherwise the engine waits for more data
    else
        this->running = false;
}

void    EngineClient::_doUnserializeContent()
{
    quint64 used = 0;
    bool    result;
    QPair<QString, LightBird::IDoUnserializeContent *> instance;

    // If a plugin matches
    if ((instance = Plugins::instance()->getInstance<LightBird::IDoUnserializeContent>(this->client.getMode(), this->client.getTransport(), this->request.getProtocol(), this->client.getPort())).second)
    {
        Log::trace("Calling IDoUnserializeContent::doUnserializeContent()", Properties("id", this->client.getId()).add("plugin", instance.first), "EngineClient", "_doUnserializeContent");
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
                Log::trace("Content complete", Properties("id", this->client.getId()).add("plugin", instance.first).add("used", used), "EngineClient", "_doUnserializeContent");
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
        Log::trace("No plugin implempents IDoUnserializeContent for this context", Properties("id", this->client.getId()), "EngineClient", "_doUnserializeContent");
        this->state = Engine::FOOTER;
    }
    // Go to the next step
    if (this->state == Engine::FOOTER)
        this->_doUnserializeFooter();
    // Otherwise the engine waits for more data
    else if (this->data.isEmpty())
        this->running = false;
}

void    EngineClient::_doUnserializeFooter()
{
    quint64 used = 0;
    bool    result;
    QPair<QString, LightBird::IDoUnserializeFooter *> instance;

    // If a plugin matches
    if ((instance = Plugins::instance()->getInstance<LightBird::IDoUnserializeFooter>(this->client.getMode(), this->client.getTransport(), this->request.getProtocol(), this->client.getPort())).second)
    {
        Log::trace("Calling IDoUnserializeFooter::doUnserializeFooter()", Properties("id", this->client.getId()).add("plugin", instance.first), "EngineClient", "_doUnserializeFooter");
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
                Log::trace("Footer complete", Properties("id", this->client.getId()).add("plugin", instance.first).add("used", used), "EngineClient", "_doUnserializeFooter");
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
        Log::trace("No plugin implempents IDoUnserializeFooter for this context", Properties("id", this->client.getId()), "EngineClient", "_doUnserializeFooter");
        this->state = Engine::HEADER;
    }
    // If the response has been unserialized
    if (this->state == Engine::HEADER)
    {
        // The response is going to be executed
        if (this->done)
        {
            if (Log::instance()->isDebug())
                Log::debug("Request complete", Properties("id", this->client.getId()), "EngineClient", "_doUnserializeFooter");
            // Calls onUnserialize
            this->_onUnserialize(LightBird::IOnUnserialize::IDoUnserialize);
            // Executes the unserialized response if there is no error
            if (this->response.isError())
                this->_doExecution();
            // Otherwise it is not executed
            else
            {
                Log::debug("An error has been found in the response", Properties("id", this->client.getId()), "EngineClient", "_doUnserializeFooter");
                this->_onFinish();
            }
        }
        // If the data has never been unserialized in header, content, or footer, it is cleared
        else
        {
            Log::warning("The data has not been unserialized because no plugin implements IDoUnserialize* for this context, or the data are never used. The data has been cleared",
                         Properties("id", this->client.getId()), "EngineClient", "_doUnserializeFooter");
            this->clear();
            this->data.clear();
            emit this->doSend();
        }
    }
    // Otherwise the engine waits for more data
    else if (this->data.isEmpty())
        this->running = false;
}

void    EngineClient::_onUnserialize(LightBird::IOnUnserialize::Unserialize type)
{
    QMapIterator<QString, LightBird::IOnUnserialize *> it(Plugins::instance()->getInstances<LightBird::IOnUnserialize>(this->client.getMode(), this->client.getTransport(), this->request.getProtocol(), this->client.getPort()));
    while (it.hasNext())
    {
        Log::trace("Calling IOnUnserialize::onUnserialize()", Properties("id", this->client.getId()).add("plugin", it.peekNext().key()), "EngineClient", "_onUnserialize");
        it.peekNext().value()->onUnserialize(this->client, type);
        Plugins::instance()->release(it.next().key());
    }
}

void        EngineClient::_doExecution()
{
    QPair<QString, LightBird::IDoExecution *> instance;

    if ((instance = Plugins::instance()->getInstance<LightBird::IDoExecution>(this->client.getMode(), this->client.getTransport(), this->request.getProtocol(), this->client.getPort(), this->request.getMethod(), this->request.getType(), true)).second)
    {
        Log::trace("Calling IDoExecution::doExecution()", Properties("id", this->client.getId()).add("plugin", instance.first), "EngineClient", "_doExecution");
        instance.second->doExecution(this->client);
        Plugins::instance()->release(instance.first);
    }
    else
        Log::trace("No plugin implempents IDoExecution for this context", Properties("id", this->client.getId()), "EngineClient", "_doExecution");
    this->_onExecution();
}

void        EngineClient::_onExecution()
{
    QMapIterator<QString, LightBird::IOnExecution *> it(Plugins::instance()->getInstances<LightBird::IOnExecution>(this->client.getMode(), this->client.getTransport(), this->request.getProtocol(), this->client.getPort()));
    while (it.hasNext())
    {
        Log::trace("Calling IOnExecution::onExecution()", Properties("id", this->client.getId()).add("plugin", it.peekNext().key()), "EngineClient", "_onExecution");
        it.peekNext().value()->onExecution(this->client);
        Plugins::instance()->release(it.next().key());
    }
    this->_onFinish();
}

void    EngineClient::_onFinish()
{
    QMapIterator<QString, LightBird::IOnFinish *> it(Plugins::instance()->getInstances<LightBird::IOnFinish>(this->client.getMode(), this->client.getTransport(), this->client.getProtocols(), this->client.getPort()));
    while (it.hasNext())
    {
        Log::trace("Calling IOnFinish::onFinish()", Properties("id", this->client.getId()).add("plugin", it.peekNext().key()).add("size", data.size()), "EngineClient", "_onFinish");
        it.peekNext().value()->onFinish(this->client);
        Plugins::instance()->release(it.next().key());
    }
    // The process is finished, so we try to generate an other request
    this->clear();
    emit this->doSend();
}
