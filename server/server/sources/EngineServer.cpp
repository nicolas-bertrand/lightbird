#include "IDoProtocol.h"
#include "IOnProtocol.h"
#include "IDoDeserializeHeader.h"
#include "IDoDeserializeContent.h"
#include "IDoDeserializeTrailer.h"
#include "IDoExecution.h"
#include "IOnExecution.h"
#include "IDoSerializeHeader.h"
#include "IDoSerializeContent.h"
#include "IDoSerializeTrailer.h"
#include "IOnFinish.h"

#include "EngineServer.h"
#include "Plugins.hpp"

EngineServer::EngineServer(Client &client)
    : Engine(client)
{
    // Initialize the Engine
    this->_clear();
}

EngineServer::~EngineServer()
{
    LOG_TRACE("EngineServer destroyed!", Properties("id", this->client.getId()), "EngineServer", "~EngineServer");
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
    this->_onDeserialize(LightBird::IOnDeserialize::IDoDeserialize);
    this->state = &EngineServer::_doExecution;
    return (true);
}

bool    EngineServer::isIdle()
{
    return (this->state == &EngineServer::_doProtocol && this->data.isEmpty() && this->idle);
}

void    EngineServer::_clear()
{
    this->state = &EngineServer::_doProtocol;
    this->needResponse = true;
    this->protocolUnknow.clear();
    this->idle = true;
    Engine::_clear();
}

bool        EngineServer::_doProtocol()
{
    QString protocol;
    bool    unknow;
    bool    result = false;

    this->idle = false;
    QMap<QString, LightBird::IDoProtocol *> plugins = Plugins::instance()->getInstances<LightBird::IDoProtocol>(this->client.getValidator(true, false));
    QMapIterator<QString, LightBird::IDoProtocol *> it(plugins);
    if (!it.hasNext())
        LOG_TRACE("No plugin implempents IDoProtocol for this context", Properties("id", this->client.getId()), "EngineServer", "_doProtocol");
    while (it.hasNext() && !result)
        if (!this->protocolUnknow.contains(it.next().key()))
        {
            protocol.clear();
            unknow = false;
            LOG_TRACE("Calling IDoProtocol::doProtocol()", Properties("id", this->client.getId()).add("plugin", it.key()), "EngineServer", "_doProtocol");
            result = it.value()->doProtocol(this->client, this->data, protocol, unknow);
            Plugins::instance()->release(it.key());
            protocol = protocol.toLower();
            // The protocol of the request has been found
            if (result)
            {
                // Check the protocol returned
                if (this->client.getProtocols().contains(protocol, Qt::CaseInsensitive) ||
                    this->client.getProtocols().contains("all", Qt::CaseInsensitive))
                {
                    this->request.setProtocol(protocol);
                    this->state = &EngineServer::_doDeserializeHeader;
                    LOG_DEBUG("Protocol found", Properties("id", this->client.getId()).add("plugin", it.key()).add("protocol", protocol), "EngineServer", "_doProtocol");
                }
                else
                {
                    LOG_WARNING("Invalid protocol", Properties("id", this->client.getId()).add("plugin", it.key()).add("protocol", protocol), "EngineServer", "_doProtocol");
                    unknow = true;
                }
            }
            // The plugin doesn't know the protocol
            if (unknow)
                this->protocolUnknow << it.key();
        }
        else
            Plugins::instance()->release(it.key());
    // If no plugin implements IDoProtocol the first protocol is used
    if (plugins.isEmpty() && !this->client.getProtocols().isEmpty() && !this->client.getProtocols().contains("all", Qt::CaseInsensitive))
    {
        this->request.setProtocol(this->client.getProtocols().first());
        this->state = &EngineServer::_doDeserializeHeader;
        LOG_DEBUG("Default protocol used", Properties("id", this->client.getId()).add("protocol", this->request.getProtocol()), "EngineServer", "_doProtocol");
    }
    // If there is no more plugin that can find the protocol, the data are cleared
    else if (this->protocolUnknow.size() >= plugins.size())
    {
        this->_clear();
        this->data.clear();
        LOG_WARNING("Protocol of the request not found", Properties("id", this->client.getId()), "EngineServer", "_doProtocol");
    }
    // The protocol has been found, and the engine can execute the next step
    if (this->state == &EngineServer::_doDeserializeHeader)
    {
        // Calls IOnProtocol before IDoDeserializeHeader
        this->_onProtocol();
        return (true);
    }
    // Otherwise the engine waits for more data
    return (false);
}

void    EngineServer::_onProtocol()
{
    QMapIterator<QString, LightBird::IOnProtocol *> it(Plugins::instance()->getInstances<LightBird::IOnProtocol>(this->client.getValidator()));
    while (it.hasNext())
    {
        LOG_TRACE("Calling IOnProtocol::onProtocol()", Properties("id", this->client.getId()).add("plugin", it.peekNext().key()), "EngineServer", "_onProtocol");
        it.peekNext().value()->onProtocol(this->client);
        Plugins::instance()->release(it.next().key());
    }
}

bool    EngineServer::_doDeserializeHeader()
{
    quint64 used = 0;
    bool    result;
    QPair<QString, LightBird::IDoDeserializeHeader *> instance;

    // If a plugin matches
    if ((instance = Plugins::instance()->getInstance<LightBird::IDoDeserializeHeader>(this->client.getValidator())).second)
    {
        LOG_TRACE("Calling IDoDeserializeHeader::doDeserializeHeader()", Properties("id", this->client.getId()).add("plugin", instance.first), "EngineServer", "_doDeserializeHeader");
        result = instance.second->doDeserializeHeader(this->client, this->data, used);
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
            LOG_TRACE("Header complete", Properties("id", this->client.getId()).add("plugin", instance.first).add("used", used), "EngineServer", "_doDeserializeHeader");
            // Calls onDeserialize
            this->_onDeserialize(LightBird::IOnDeserialize::IDoDeserializeHeader);
            this->state = &EngineServer::_doDeserializeContent;
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
        LOG_TRACE("No plugin implempents IDoDeserializeHeader for this context", Properties("id", this->client.getId()), "EngineServer", "_doDeserializeHeader");
        this->state = &EngineServer::_doDeserializeContent;
    }
    // Go to the next step
    if (this->state == &EngineServer::_doDeserializeContent)
        return (true);
    // Otherwise the engine waits for more data
    return (false);
}

bool    EngineServer::_doDeserializeContent()
{
    quint64 used = 0;
    bool    result;
    QPair<QString, LightBird::IDoDeserializeContent *> instance;

    // If a plugin matches
    if ((instance = Plugins::instance()->getInstance<LightBird::IDoDeserializeContent>(this->client.getValidator())).second)
    {
        LOG_TRACE("Calling IDoDeserializeContent::doDeserializeContent()", Properties("id", this->client.getId()).add("plugin", instance.first), "EngineServer", "_doDeserializeContent");
        result = instance.second->doDeserializeContent(this->client, this->data, used);
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
            LOG_TRACE("Content complete", Properties("id", this->client.getId()).add("plugin", instance.first).add("used", used), "EngineServer", "_doDeserializeContent");
            this->state = &EngineServer::_doDeserializeTrailer;
        }
        else
            // All the data has been used, but the content is not complete
            this->data.clear();
        // Calls onDeserialize
        this->_onDeserialize(LightBird::IOnDeserialize::IDoDeserializeContent);
        // If the data have been used
        if (!result || used)
            this->done = true;
    }
    // If no plugin matches, we go to the next step
    else
    {
        LOG_TRACE("No plugin implempents IDoDeserializeContent for this context", Properties("id", this->client.getId()), "EngineServer", "_doDeserializeContent");
        this->state = &EngineServer::_doDeserializeTrailer;
    }
    // Go to the next step
    if (this->state == &EngineServer::_doDeserializeTrailer)
        return (true);
    // Otherwise the engine waits for more data
    return (false);
}

bool    EngineServer::_doDeserializeTrailer()
{
    quint64 used = 0;
    bool    result;
    QPair<QString, LightBird::IDoDeserializeTrailer *> instance;

    // If a plugin matches
    if ((instance = Plugins::instance()->getInstance<LightBird::IDoDeserializeTrailer>(this->client.getValidator())).second)
    {
        LOG_TRACE("Calling IDoDeserializeTrailer::doDeserializeTrailer()", Properties("id", this->client.getId()).add("plugin", instance.first), "EngineServer", "_doDeserializeTrailer");
        result = instance.second->doDeserializeTrailer(this->client, this->data, used);
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
            LOG_TRACE("Trailer complete", Properties("id", this->client.getId()).add("plugin", instance.first).add("used", used), "EngineServer", "_doDeserializeTrailer");
            // Calls onDeserialize
            this->_onDeserialize(LightBird::IOnDeserialize::IDoDeserializeTrailer);
            this->state = &EngineServer::_doExecution;
        }
        else
            // All the data has been used, but the trailer is not complete
            this->data.clear();
        // If the data have been used
        if (!result || used)
            this->done = true;
    }
    // If no plugin matches, we go to the next step
    else
    {
        LOG_TRACE("No plugin implempents IDoDeserializeTrailer for this context", Properties("id", this->client.getId()), "EngineServer", "_doDeserializeTrailer");
        this->state = &EngineServer::_doExecution;
    }
    // If the request has been deserialized
    if (this->state == &EngineServer::_doExecution)
    {
        // If the request has been deserialized, it is executed
        if (this->done)
        {
            LOG_DEBUG("Request complete", Properties("id", this->client.getId()), "EngineServer", "_doDeserializeTrailer");
            // Calls onDeserialize
            this->_onDeserialize(LightBird::IOnDeserialize::IDoDeserialize);
            // If there is an error in the request, the deserialized request is not executed, and the response which may contains the error is directly sent
            if (this->request.isError())
            {
                LOG_DEBUG("An error has been found in the request", Properties("id", this->client.getId()), "EngineServer", "_doDeserializeTrailer");
                this->state = &EngineServer::_doSerializeHeader;
            }
        }
        // If the data has never been deserialize in header, content, or trailer, they are cleared
        else
        {
            LOG_WARNING("The data has not been deserialized because no plugin implements IDoDeserialize* for this context, or the data are never used. The data has been cleared",
                        Properties("id", this->client.getId()), "EngineServer", "_doDeserializeTrailer");
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
    if ((instance = Plugins::instance()->getInstance<LightBird::IDoExecution>(this->client.getValidator(true, true, true))).second)
    {
        LOG_TRACE("Calling IDoExecution::doExecution()", Properties("id", this->client.getId()).add("plugin", instance.first), "EngineServer", "_doExecution");
        if (!(this->needResponse = instance.second->doExecution(this->client)))
            LOG_TRACE("IDoExecution::doExecution() returned false", Properties("id", this->client.getId()).add("plugin", instance.first), "EngineServer", "_doExecution");
        Plugins::instance()->release(instance.first);
    }
    else
        LOG_TRACE("No plugin implempents IDoExecution for this context", Properties("id", this->client.getId()), "EngineServer", "_doExecution");
    this->state = &EngineServer::_onExecution;
    return (true);
}

bool        EngineServer::_onExecution()
{
    QMapIterator<QString, LightBird::IOnExecution *> it(Plugins::instance()->getInstances<LightBird::IOnExecution>(this->client.getValidator()));
    while (it.hasNext())
    {
        LOG_TRACE("Calling IOnExecution::onExecution()", Properties("id", this->client.getId()).add("plugin", it.peekNext().key()), "EngineServer", "_onExecution");
        if (!it.peekNext().value()->onExecution(this->client))
        {
            this->needResponse = false;
            LOG_TRACE("IOnExecution::onExecution() returned false", Properties("id", this->client.getId()).add("plugin", it.peekNext().key()), "EngineServer", "_onExecution");
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
    if ((instance = Plugins::instance()->getInstance<LightBird::IDoSerializeHeader>(this->client.getValidator())).second)
    {
        QByteArray *data = new QByteArray();
        this->_onSerialize(LightBird::IOnSerialize::IDoSerializeHeader);
        LOG_TRACE("Calling IDoSerializeHeader::doSerializeHeader()", Properties("id", this->client.getId()).add("plugin", instance.first), "EngineServer", "_doSerializeHeader");
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
        LOG_TRACE("No plugin implempents IDoSerializeHeader for this context", Properties("id", this->client.getId()), "EngineServer", "_doSerializeHeader");
    this->state = &EngineServer::_doSerializeContent;
    return (true);
}

bool    EngineServer::_doSerializeContent()
{
    QPair<QString, LightBird::IDoSerializeContent *> instance;
    bool        result = true;

    if ((instance = Plugins::instance()->getInstance<LightBird::IDoSerializeContent>(this->client.getValidator())).second)
    {
        QByteArray *data = new QByteArray();
        this->_onSerialize(LightBird::IOnSerialize::IDoSerializeContent);
        LOG_TRACE("Calling IDoSerializeContent::doSerializeContent()", Properties("id", this->client.getId()).add("plugin", instance.first), "EngineServer", "_doSerializeContent");
        if ((result = instance.second->doSerializeContent(this->client, *data)))
            LOG_TRACE("Content serialized", Properties("id", this->client.getId()).add("plugin", instance.first), "EngineServer", "_doSerializeContent");
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
        LOG_TRACE("No plugin implempents IDoSerializeContent for this context", Properties("id", this->client.getId()), "EngineServer", "_doSerializeContent");
    // The content has been serialized
    if (result)
        this->state = &EngineServer::_doSerializeTrailer;
    // There is more data to serialize
    else
        this->state = &EngineServer::_doSerializeContent;
    return (true);
}

bool    EngineServer::_doSerializeTrailer()
{
    QPair<QString, LightBird::IDoSerializeTrailer *> instance;

    if ((instance = Plugins::instance()->getInstance<LightBird::IDoSerializeTrailer>(this->client.getValidator())).second)
    {
        QByteArray *data = new QByteArray();
        this->_onSerialize(LightBird::IOnSerialize::IDoSerializeTrailer);
        LOG_TRACE("Calling IDoSerializeTrailer::doSerializeTrailer()", Properties("id", this->client.getId()).add("plugin", instance.first), "EngineServer", "_doSerializeTrailer");
        instance.second->doSerializeTrailer(this->client, *data);
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
        LOG_TRACE("No plugin implempents IDoSerializeTrailer for this context", Properties("id", this->client.getId()), "EngineServer", "_doSerializeTrailer");
    if (!this->done)
        LOG_WARNING("The data has not been serialized because no plugin implements IDoSerialize* for this context.", Properties("id", this->client.getId()), "EngineServer", "_doSerializeTrailer");
    this->_onFinish();
    // If there are pending data, they are processed
    if (!this->data.isEmpty())
        return (true);
    // Otherwise, The engine will wait for more data
    return (false);
}

void    EngineServer::_onFinish()
{
    QMapIterator<QString, LightBird::IOnFinish *> it(Plugins::instance()->getInstances<LightBird::IOnFinish>(this->client.getValidator(true, false)));
    while (it.hasNext())
    {
        LOG_TRACE("Calling IOnFinish::onFinish()", Properties("id", this->client.getId()).add("plugin", it.peekNext().key()).add("size", data.size()), "EngineServer", "_onFinish");
        it.peekNext().value()->onFinish(this->client);
        Plugins::instance()->release(it.next().key());
    }
    this->_clear();
}
