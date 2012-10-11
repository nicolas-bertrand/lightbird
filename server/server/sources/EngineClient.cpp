#include "IDoSend.h"
#include "IOnSend.h"
#include "IDoSerializeHeader.h"
#include "IDoSerializeContent.h"
#include "IDoSerializeFooter.h"
#include "IDoDeserializeHeader.h"
#include "IDoDeserializeContent.h"
#include "IDoDeserializeFooter.h"
#include "IDoExecution.h"
#include "IOnExecution.h"
#include "IOnFinish.h"

#include "EngineClient.h"
#include "Plugins.hpp"

EngineClient::EngineClient(Client &client) : Engine(client)
{
    // Initialize the Engine
    this->_clear();
}

EngineClient::~EngineClient()
{
    Log::trace("EngineClient destroyed!", Properties("id", this->client.getId()), "EngineClient", "~EngineClient");
}

bool    EngineClient::run()
{
    if (this->state)
        return ((this->*state)());
    return (false);
}

bool    EngineClient::send(const QString &id, const QString &protocol, const QVariantMap &informations)
{
    QVariantMap request;

    // Checks that the plugin does not have a request already pending
    QListIterator<QVariantMap> it(this->requests);
    request["id"] = id;
    request["protocol"] = protocol;
    request["informations"] = informations;
    this->requests.push_back(request);
    while (it.hasNext())
        if (it.peekNext().value("id").toString() == id)
            this->requests.pop_back();
    // If the engine is ready to process a new request
    if (this->state == NULL)
    {
        this->state = &EngineClient::_doSend;
        return (true);
    }
    return (false);
}

bool    EngineClient::receive(const QString &protocol, const QVariantMap &informations)
{
    if (!this->isIdle())
        return (false);
    this->request.setProtocol(protocol);
    this->request.getInformations() = informations;
    this->_onSerialize(LightBird::IOnSerialize::IDoSerialize);
    this->state = &EngineClient::_doDeserializeHeader;
    return (true);
}

bool    EngineClient::isIdle()
{
    return (!this->state && !this->data.isEmpty());
}

void    EngineClient::_clear()
{
    this->state = NULL;
    Engine::_clear();
}

bool        EngineClient::_doSend()
{
    bool    found = false;
    bool    result = false;
    QString id = this->requests.front().value("id").toString();

    this->request.setProtocol(this->requests.front().value("protocol").toString());
    this->request.getInformations() = this->requests.front().value("informations").toMap();
    this->requests.pop_front();
    // Gets the plugins that asked to send a request
    QMapIterator<QString, LightBird::IDoSend *> it(Plugins::instance()->getInstances<LightBird::IDoSend>(this->client.getMode(), this->client.getTransport(), this->request.getProtocol(), this->client.getPort()));
    while (it.hasNext())
    {
        if (!found && it.peekNext().key() == id)
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
    // No valid plugin found, or the send has been canceled. Tries to process an other request.
    if (!found || !result)
    {
        if (!found)
            Log::debug("The plugin does not implempents IDoSend for this context", Properties("id", this->client.getId()).add("plugin", id).add("protocol", this->request.getProtocol()), "EngineClient", "_doSend");
        else
            Log::debug("The send of the request has been canceled by a plugin", Properties("id", this->client.getId()).add("plugin", id).add("protocol", this->request.getProtocol()), "EngineClient", "_doSend");
        // Try to execute the next request
        if (!this->requests.isEmpty())
            return (true);
        // Nothing to do
        this->_clear();
        return (false);
    }
    // Go to the next step
    this->state = &EngineClient::_doSerializeHeader;
    return (true);
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

bool    EngineClient::_doSerializeHeader()
{
    QPair<QString, LightBird::IDoSerializeHeader *> instance;

    if ((instance = Plugins::instance()->getInstance<LightBird::IDoSerializeHeader>(this->client.getMode(), this->client.getTransport(), this->request.getProtocol(), this->client.getPort())).second)
    {
        QByteArray *data = new QByteArray();
        this->_onSerialize(LightBird::IOnSerialize::IDoSerializeHeader);
        Log::trace("Calling IDoSerializeHeader::doSerializeHeader()", Properties("id", this->client.getId()).add("plugin", instance.first), "EngineClient", "_doSerializeHeader");
        instance.second->doSerializeHeader(this->client, *data);
        Plugins::instance()->release(instance.first);
        this->done = true;
        if (data->size())
        {
            this->_onWrite(*data);
            this->client.write(data);
        }
        else
            delete data;
    }
    else
        Log::trace("No plugin implempents IDoSerializeHeader for this context", Properties("id", this->client.getId()), "EngineClient", "_doSerializeHeader");
    this->state = &EngineClient::_doSerializeContent;
    return (true);
}

bool    EngineClient::_doSerializeContent()
{
    QPair<QString, LightBird::IDoSerializeContent *> instance;
    bool    result = true;

    if ((instance = Plugins::instance()->getInstance<LightBird::IDoSerializeContent>(this->client.getMode(), this->client.getTransport(), this->request.getProtocol(), this->client.getPort())).second)
    {
        QByteArray *data = new QByteArray();
        this->_onSerialize(LightBird::IOnSerialize::IDoSerializeContent);
        Log::trace("Calling IDoSerializeContent::doSerializeContent()", Properties("id", this->client.getId()).add("plugin", instance.first), "EngineClient", "_doSerializeContent");
        if ((result = instance.second->doSerializeContent(this->client, *data)))
            Log::trace("Content serialized", Properties("id", this->client.getId()).add("plugin", instance.first), "EngineClient", "_doSerializeContent");
        Plugins::instance()->release(instance.first);
        this->done = true;
        if (data->size())
        {
            this->_onWrite(*data);
            this->client.write(data);
        }
        // There is no content to serialize
        else
        {
            result = true;
            delete data;
        }
    }
    else
        Log::trace("No plugin implempents IDoSerializeContent for this context", Properties("id", this->client.getId()), "EngineClient", "_doSerializeContent");
    // The content has been serialized
    if (result)
        this->state = &EngineClient::_doSerializeFooter;
    // There is more data to serialize
    else
        this->state = &EngineClient::_doSerializeContent;
    return (true);
}

bool    EngineClient::_doSerializeFooter()
{
    QPair<QString, LightBird::IDoSerializeFooter *> instance;

    if ((instance = Plugins::instance()->getInstance<LightBird::IDoSerializeFooter>(this->client.getMode(), this->client.getTransport(), this->request.getProtocol(), this->client.getPort())).second)
    {
        QByteArray *data = new QByteArray();
        this->_onSerialize(LightBird::IOnSerialize::IDoSerializeFooter);
        Log::trace("Calling IDoSerializeFooter::doSerializeFooter()", Properties("id", this->client.getId()).add("plugin", instance.first), "EngineClient", "_doSerializeFooter");
        instance.second->doSerializeFooter(this->client, *data);
        Plugins::instance()->release(instance.first);
        this->done = true;
        if (data->size())
        {
            this->_onWrite(*data);
            this->client.write(data);
        }
        else
            delete data;
    }
    else
        Log::trace("No plugin implempents IDoSerializeFooter for this context", Properties("id", this->client.getId()), "EngineClient", "_doSerializeFooter");
    // The request has been sent and we are ready to execute the response
    if (this->done)
    {
        // Calls the final IOnSerialize that define if the request needs a response
        if (this->_onSerialize(LightBird::IOnSerialize::IDoSerialize))
        {
            this->state = &EngineClient::_doDeserializeHeader;
            // If there are pending data they are deserialized
            if (!this->data.isEmpty())
                return (true);
            // Otherwise we are waiting for a response
            else
                return (false);
        }
        // No response needed
        else
        {
            Log::trace("The request does not need a response. An other request is going to be processed.", Properties("id", this->client.getId()), "EngineClient", "_doSerializeFooter");
            return (this->_onFinish());
        }
    }
    // The request has not been serialized
    else
    {
        Log::warning("The data has not been serialized because no plugin implements IDoSerialize* for this context.", Properties("id", this->client.getId()), "EngineClient", "_doSerializeFooter");
        this->_clear();
        if (this->requests.isEmpty())
            return (false);
        this->state = &EngineClient::_doSend;
        return (true);
    }
}

bool    EngineClient::_doDeserializeHeader()
{
    quint64 used = 0;
    bool    result;
    QPair<QString, LightBird::IDoDeserializeHeader *> instance;

    this->done = false;
    // If a plugin matches
    if ((instance = Plugins::instance()->getInstance<LightBird::IDoDeserializeHeader>(this->client.getMode(), this->client.getTransport(), this->request.getProtocol(), this->client.getPort())).second)
    {
        Log::trace("Calling IDoDeserializeHeader::doDeserializeHeader()", Properties("id", this->client.getId()).add("plugin", instance.first), "EngineClient", "_doDeserializeHeader");
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
            if (Log::instance()->isTrace())
                Log::trace("Header complete", Properties("id", this->client.getId()).add("plugin", instance.first).add("used", used), "EngineClient", "_doDeserializeHeader");
            // Calls onDeserialize
            this->_onDeserialize(LightBird::IOnDeserialize::IDoDeserializeHeader);
            this->state = &EngineClient::_doDeserializeContent;
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
        Log::trace("No plugin implempents IDoDeserializeHeader for this context", Properties("id", this->client.getId()), "EngineClient", "_doDeserializeHeader");
        this->state = &EngineClient::_doDeserializeContent;
    }
    // Go to the next step
    if (this->state == &EngineClient::_doDeserializeContent)
        return (true);
    // Otherwise the engine waits for more data
    return (false);
}

bool    EngineClient::_doDeserializeContent()
{
    quint64 used = 0;
    bool    result;
    QPair<QString, LightBird::IDoDeserializeContent *> instance;

    // If a plugin matches
    if ((instance = Plugins::instance()->getInstance<LightBird::IDoDeserializeContent>(this->client.getMode(), this->client.getTransport(), this->request.getProtocol(), this->client.getPort())).second)
    {
        Log::trace("Calling IDoDeserializeContent::doDeserializeContent()", Properties("id", this->client.getId()).add("plugin", instance.first), "EngineClient", "_doDeserializeContent");
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
            if (Log::instance()->isTrace())
                Log::trace("Content complete", Properties("id", this->client.getId()).add("plugin", instance.first).add("used", used), "EngineClient", "_doDeserializeContent");
            this->state = &EngineClient::_doDeserializeFooter;
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
        Log::trace("No plugin implempents IDoDeserializeContent for this context", Properties("id", this->client.getId()), "EngineClient", "_doDeserializeContent");
        this->state = &EngineClient::_doDeserializeFooter;
    }
    // Go to the next step
    if (this->state == &EngineClient::_doDeserializeFooter)
        return (true);
    // Otherwise the engine waits for more data
    return (false);
}

bool    EngineClient::_doDeserializeFooter()
{
    quint64 used = 0;
    bool    result;
    QPair<QString, LightBird::IDoDeserializeFooter *> instance;

    // If a plugin matches
    if ((instance = Plugins::instance()->getInstance<LightBird::IDoDeserializeFooter>(this->client.getMode(), this->client.getTransport(), this->request.getProtocol(), this->client.getPort())).second)
    {
        Log::trace("Calling IDoDeserializeFooter::doDeserializeFooter()", Properties("id", this->client.getId()).add("plugin", instance.first), "EngineClient", "_doDeserializeFooter");
        result = instance.second->doDeserializeFooter(this->client, this->data, used);
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
                Log::trace("Footer complete", Properties("id", this->client.getId()).add("plugin", instance.first).add("used", used), "EngineClient", "_doDeserializeFooter");
            // Calls onDeserialize
            this->_onDeserialize(LightBird::IOnDeserialize::IDoDeserializeFooter);
            this->state = &EngineClient::_doExecution;
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
        Log::trace("No plugin implempents IDoDeserializeFooter for this context", Properties("id", this->client.getId()), "EngineClient", "_doDeserializeFooter");
        this->state = &EngineClient::_doExecution;
    }
    // If the response has been deserialized
    if (this->state == &EngineClient::_doExecution)
    {
        // The response is going to be executed
        if (this->done)
        {
            if (Log::instance()->isDebug())
                Log::debug("Response complete", Properties("id", this->client.getId()), "EngineClient", "_doDeserializeFooter");
            // Calls onDeserialize
            this->_onDeserialize(LightBird::IOnDeserialize::IDoDeserialize);
            // Executes the deserialized response if there is no error
            if (!this->response.isError())
                return (true);
            // Otherwise it is not executed
            else
            {
                Log::debug("An error has been found in the response", Properties("id", this->client.getId()), "EngineClient", "_doDeserializeFooter");
                return (this->_onFinish());
            }
        }
        // If the data has never been deserialized in header, content, or footer, they are cleared
        else
        {
            Log::warning("The data has not been deserialized because no plugin implements IDoDeserialize* for this context, or the data are never used. The data has been cleared",
                         Properties("id", this->client.getId()), "EngineClient", "_doDeserializeFooter");
            this->data.clear();
            return (this->_onFinish());
        }
    }
    // Otherwise the engine waits for more data
    return (false);
}

bool        EngineClient::_doExecution()
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
    this->state = &EngineClient::_onExecution;
    return (true);
}

bool        EngineClient::_onExecution()
{
    QMapIterator<QString, LightBird::IOnExecution *> it(Plugins::instance()->getInstances<LightBird::IOnExecution>(this->client.getMode(), this->client.getTransport(), this->request.getProtocol(), this->client.getPort()));
    while (it.hasNext())
    {
        Log::trace("Calling IOnExecution::onExecution()", Properties("id", this->client.getId()).add("plugin", it.peekNext().key()), "EngineClient", "_onExecution");
        it.peekNext().value()->onExecution(this->client);
        Plugins::instance()->release(it.next().key());
    }
    return (this->_onFinish());
}

bool    EngineClient::_onFinish()
{
    QMapIterator<QString, LightBird::IOnFinish *> it(Plugins::instance()->getInstances<LightBird::IOnFinish>(this->client.getMode(), this->client.getTransport(), this->client.getProtocols(), this->client.getPort()));
    while (it.hasNext())
    {
        Log::trace("Calling IOnFinish::onFinish()", Properties("id", this->client.getId()).add("plugin", it.peekNext().key()).add("size", data.size()), "EngineClient", "_onFinish");
        it.peekNext().value()->onFinish(this->client);
        Plugins::instance()->release(it.next().key());
    }
    this->_clear();
    if (this->requests.isEmpty())
        return (false);
    this->state = &EngineClient::_doSend;
    return (true);
}
