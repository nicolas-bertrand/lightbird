#include "Engine.h"
#include "IOnRead.h"
#include "IOnWrite.h"
#include "Plugins.hpp"

Engine::Engine(Client &c) : client(c)
{
}

Engine::~Engine()
{
}

void    Engine::read(QByteArray &data)
{
    this->_onRead(data);
    this->data.append(data);
}

void    Engine::clear()
{
    this->request.clear();
    this->response.clear();
    this->done = false;
}

LightBird::IRequest &Engine::getRequest()
{
    return (this->request);
}

LightBird::IResponse &Engine::getResponse()
{
    return (this->response);
}

void    Engine::_onRead(QByteArray &data)
{
    QMapIterator<QString, LightBird::IOnRead *> it(Plugins::instance()->getInstances<LightBird::IOnRead>(this->client.getMode(), this->client.getTransport(), this->client.getProtocols(), this->client.getPort()));

    while (it.hasNext())
    {
        Log::trace("Calling IOnRead::onRead()", Properties("id", this->client.getId()).add("plugin", it.peekNext().key()), "Engine", "_onRead");
        it.peekNext().value()->onRead(this->client, data);
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
