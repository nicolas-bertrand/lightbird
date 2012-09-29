#include "IOnRead.h"
#include "IOnWrite.h"

#include "Engine.h"
#include "LightBird.h"
#include "Plugins.hpp"

Engine::Engine(Client &c) : client(c), data(c.getData())
{
}

Engine::~Engine()
{
}

LightBird::IRequest &Engine::getRequest()
{
    return (this->request);
}

LightBird::IResponse &Engine::getResponse()
{
    return (this->response);
}

void    Engine::_clear()
{
    this->request.clear();
    this->response.clear();
    this->done = false;
}

void    Engine::onRead()
{
    QMapIterator<QString, LightBird::IOnRead *> it(Plugins::instance()->getInstances<LightBird::IOnRead>(this->client.getMode(), this->client.getTransport(), this->client.getProtocols(), this->client.getPort()));

    if (Log::instance()->isTrace())
        Log::trace("Data received", Properties("id", this->client.getId()).add("data", LightBird::simplify(this->data)).add("size", this->data.size()), "Engine", "onRead");
    else if (Log::instance()->isDebug())
        Log::debug("Data received", Properties("id", this->client.getId()).add("size", this->data.size()), "Engine", "onRead");
    while (it.hasNext())
    {
        Log::trace("Calling IOnRead::onRead()", Properties("id", this->client.getId()).add("plugin", it.peekNext().key()), "Engine", "onRead");
        it.peekNext().value()->onRead(this->client, this->data);
        Plugins::instance()->release(it.next().key());
    }
}

void    Engine::_onWrite(QByteArray &data)
{
    QMapIterator<QString, LightBird::IOnWrite *> it(Plugins::instance()->getInstances<LightBird::IOnWrite>(this->client.getMode(), this->client.getTransport(), this->client.getProtocols(), this->client.getPort()));
    while (it.hasNext())
    {
        Log::trace("Calling IOnWrite::onWrite()", Properties("id", this->client.getId()).add("plugin", it.peekNext().key()).add("size", data.size()), "Engine", "_onWrite");
        it.peekNext().value()->onWrite(this->client, data);
        Plugins::instance()->release(it.next().key());
    }
}

void    Engine::_onUnserialize(LightBird::IOnUnserialize::Unserialize type)
{
    QMapIterator<QString, LightBird::IOnUnserialize *> it(Plugins::instance()->getInstances<LightBird::IOnUnserialize>(this->client.getMode(), this->client.getTransport(), this->request.getProtocol(), this->client.getPort()));
    while (it.hasNext())
    {
        Log::trace("Calling IOnUnserialize::onUnserialize()", Properties("id", this->client.getId()).add("plugin", it.peekNext().key()), "Engine", "_onUnserialize");
        it.peekNext().value()->onUnserialize(this->client, type);
        Plugins::instance()->release(it.next().key());
    }
}

bool        Engine::_onSerialize(LightBird::IOnSerialize::Serialize type)
{
    bool    result = true;

    QMapIterator<QString, LightBird::IOnSerialize *> it(Plugins::instance()->getInstances<LightBird::IOnSerialize>(this->client.getMode(), this->client.getTransport(), this->request.getProtocol(), this->client.getPort()));
    while (it.hasNext())
    {
        Log::trace("Calling IOnSerialize::onSerialize()", Properties("id", this->client.getId()).add("plugin", it.peekNext().key()), "Engine", "_onSerialize");
        if (!it.peekNext().value()->onSerialize(this->client, type))
            result = false;
        Plugins::instance()->release(it.next().key());
    }
    return (result);
}
