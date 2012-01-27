#include "IOnProtocol.h"
#include "IDoUnserializeHeader.h"
#include "IDoUnserializeContent.h"
#include "IDoUnserializeFooter.h"
#include "IDoExecution.h"
#include "IOnExecution.h"
#include "IDoSerializeHeader.h"
#include "IDoSerializeContent.h"
#include "IDoSerializeFooter.h"
#include "IOnFinish.h"

#include "EngineServer.h"
#include "Plugins.hpp"

EngineServer::EngineServer(Client &client) : Engine(client)
{
    // Initialize the Engine
    this->_clear();
}

EngineServer::~EngineServer()
{
    Log::trace("EngineServer destroyed!", Properties("id", this->client.getId()), "EngineServer", "~EngineServer");
}

bool    EngineServer::run()
{
    return ((this->*state)());
}

bool    EngineServer::send(const QString &protocol, const QVariantMap &informations)
{
    if (!this->isIdle())
        return (false);
    this->request.setProtocol(protocol);
    this->request.getInformations() = informations;
    this->_onUnserialize(LightBird::IOnUnserialize::IDoUnserialize);
    this->state = &EngineServer::_doExecution;
    return (true);
}

bool    EngineServer::isIdle()
{
    return (this->state == &EngineServer::_onProtocol && this->data.isEmpty() && this->idle);
}

void    EngineServer::_clear()
{
    this->state = &EngineServer::_onProtocol;
    this->needResponse = true;
    this->protocolUnknow.clear();
    this->idle = true;
    Engine::_clear();
}

bool        EngineServer::_onProtocol()
{
    QString protocol;
    bool    unknow;
    bool    result = false;

    this->idle = false;
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
                    this->state = &EngineServer::_doUnserializeHeader;
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
    // If no plugin implements IOnProtocol the first protocol is used
    if (plugins.isEmpty() && !this->client.getProtocols().isEmpty() && !this->client.getProtocols().contains("all", Qt::CaseInsensitive))
    {
        this->request.setProtocol(this->client.getProtocols().first());
        this->state = &EngineServer::_doUnserializeHeader;
        Log::trace("Default protocol used", Properties("id", this->client.getId()).add("protocol", this->request.getProtocol()), "EngineServer", "_onProtocol");
    }
    // If there is no more plugin that can find the protocol, the data are cleared
    else if (this->protocolUnknow.size() >= plugins.size())
    {
        this->_clear();
        this->data.clear();
        Log::warning("Protocol of the request not found", Properties("id", this->client.getId()), "EngineServer", "_onProtocol");
    }
    // The protocol has been found, and the engine can execute the next step
    if (this->state == &EngineServer::_doUnserializeHeader)
        return (true);
    // Otherwise the engine waits for more data
    return (false);
}

bool    EngineServer::_doUnserializeHeader()
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
            this->state = &EngineServer::_doUnserializeContent;
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
        this->state = &EngineServer::_doUnserializeContent;
    }
    // Go to the next step
    if (this->state == &EngineServer::_doUnserializeContent)
        return (true);
    // Otherwise the engine waits for more data
    return (false);
}

bool    EngineServer::_doUnserializeContent()
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
            this->state = &EngineServer::_doUnserializeFooter;
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
        this->state = &EngineServer::_doUnserializeFooter;
    }
    // Go to the next step
    if (this->state == &EngineServer::_doUnserializeFooter)
        return (true);
    // Otherwise the engine waits for more data
    return (false);
}

bool    EngineServer::_doUnserializeFooter()
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
            this->state = &EngineServer::_doExecution;
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
        this->state = &EngineServer::_doExecution;
    }
    // If the request has been unserialized
    if (this->state == &EngineServer::_doExecution)
    {
        // If the request has been unserialized, it is executed
        if (this->done)
        {
            if (Log::instance()->isDebug())
                Log::debug("Request complete", Properties("id", this->client.getId()), "EngineServer", "_doUnserializeFooter");
            // Calls onUnserialize
            this->_onUnserialize(LightBird::IOnUnserialize::IDoUnserialize);
            // If there is an error in the request, the unserialized request is not executed, and the response which may contains the error is directly sent
            if (this->request.isError())
            {
                Log::debug("An error has been found in the request", Properties("id", this->client.getId()), "EngineServer", "_doUnserializeFooter");
                this->state = &EngineServer::_doSerializeHeader;
            }
        }
        // If the data has never been unserialize in header, content, or footer, they are cleared
        else
        {
            Log::warning("The data has not been unserialized because no plugin implements IDoUnserialize* for this context, or the data are never used. The data has been cleared",
                         Properties("id", this->client.getId()), "EngineServer", "_doUnserializeFooter");
            this->_clear();
            this->data.clear();
            return (false);
        }
        return (true);
    }
    // Otherwise the engine waits for more data
    return (false);
}

bool        EngineServer::_doExecution()
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
    this->state = &EngineServer::_onExecution;
    return (true);
}

bool        EngineServer::_onExecution()
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
    // If a response to the request is needed, it is going to be serialized
    if (this->needResponse)
        this->state = &EngineServer::_doSerializeHeader;
    // Otherwise the engine tries to process an other request
    else
    {
        this->_onFinish();
        // If there is no pending data the engine waits for a new request
        if (this->data.isEmpty())
            return (false);
    }
    return (true);
}

bool    EngineServer::_doSerializeHeader()
{
    QPair<QString, LightBird::IDoSerializeHeader *> instance;

    this->done = false;
    this->_onSerialize(LightBird::IOnSerialize::IDoSerialize);
    if ((instance = Plugins::instance()->getInstance<LightBird::IDoSerializeHeader>(this->client.getMode(), this->client.getTransport(), this->request.getProtocol(), this->client.getPort())).second)
    {
        QByteArray *data = new QByteArray();
        this->_onSerialize(LightBird::IOnSerialize::IDoSerializeHeader);
        Log::trace("Calling IDoSerializeHeader::doSerializeHeader()", Properties("id", this->client.getId()).add("plugin", instance.first), "EngineServer", "_doSerializeHeader");
        instance.second->doSerializeHeader(this->client, *data);
        Plugins::instance()->release(instance.first);
        if (data->size())
        {
            this->_onWrite(*data);
            this->client.write(data);
            this->done = true;
        }
        else
            delete data;
    }
    else
        Log::trace("No plugin implempents IDoSerializeHeader for this context", Properties("id", this->client.getId()), "EngineServer", "_doSerializeHeader");
    this->state = &EngineServer::_doSerializeContent;
    return (true);
}

bool    EngineServer::_doSerializeContent()
{
    QPair<QString, LightBird::IDoSerializeContent *> instance;
    bool        result = true;

    if ((instance = Plugins::instance()->getInstance<LightBird::IDoSerializeContent>(this->client.getMode(), this->client.getTransport(), this->request.getProtocol(), this->client.getPort())).second)
    {
        QByteArray *data = new QByteArray();
        this->_onSerialize(LightBird::IOnSerialize::IDoSerializeContent);
        Log::trace("Calling IDoSerializeContent::doSerializeContent()", Properties("id", this->client.getId()).add("plugin", instance.first), "EngineServer", "_doSerializeContent");
        if ((result = instance.second->doSerializeContent(this->client, *data)))
            Log::trace("Content serialized", Properties("id", this->client.getId()).add("plugin", instance.first), "EngineServer", "_doSerializeContent");
        Plugins::instance()->release(instance.first);
        if (data->size())
        {
            this->_onWrite(*data);
            this->client.write(data);
            this->done = true;
        }
        // There is no content to serialize
        else
        {
            result = true;
            delete data;
        }
    }
    else
        Log::trace("No plugin implempents IDoSerializeContent for this context", Properties("id", this->client.getId()), "EngineServer", "_doSerializeContent");
    // The content has been serialized
    if (result)
        this->state = &EngineServer::_doSerializeFooter;
    // There is more data to serialize
    else
        this->state = &EngineServer::_doSerializeContent;
    return (true);
}

bool    EngineServer::_doSerializeFooter()
{
    QPair<QString, LightBird::IDoSerializeFooter *> instance;

    if ((instance = Plugins::instance()->getInstance<LightBird::IDoSerializeFooter>(this->client.getMode(), this->client.getTransport(), this->request.getProtocol(), this->client.getPort())).second)
    {
        QByteArray *data = new QByteArray();
        this->_onSerialize(LightBird::IOnSerialize::IDoSerializeFooter);
        Log::trace("Calling IDoSerializeFooter::doSerializeFooter()", Properties("id", this->client.getId()).add("plugin", instance.first), "EngineServer", "_doSerializeFooter");
        instance.second->doSerializeFooter(this->client, *data);
        Plugins::instance()->release(instance.first);
        if (data->size())
        {
            this->_onWrite(*data);
            this->client.write(data);
            this->done = true;
        }
        else
            delete data;
    }
    else
        Log::trace("No plugin implempents IDoSerializeFooter for this context", Properties("id", this->client.getId()), "EngineServer", "_doSerializeFooter");
    if (!this->done)
        Log::warning("The data has not been serialized because no plugin implements IDoSerialize* for this context.", Properties("id", this->client.getId()), "EngineServer", "_doSerializeFooter");
    this->_onFinish();
    // If there are pending data, they are processed
    if (!this->data.isEmpty())
        return (true);
    // Otherwise, The engine will wait for more data
    return (false);
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
    this->_clear();
}
