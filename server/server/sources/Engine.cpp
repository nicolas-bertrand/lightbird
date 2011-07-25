#include "Engine.h"
#include "IOnRead.h"
#include "IOnWrite.h"
#include "Plugins.hpp"
#include "Tools.h"

Engine::Engine(Client &c) : client(c)
{
}

Engine::~Engine()
{
}

void    Engine::read(const QList<QByteArray *> &data)
{
    // If there is only one data we can add it directly
    if (data.size() == 1)
    {
        this->_onRead(*data.first());
        this->data.append(*data.first());
        return ;
    }
    // Otherwise the data must be aggregated
    QListIterator<QByteArray *> it(data);
    unsigned int                newSize = this->data.size();
    unsigned int                oldSize = this->data.size();
    // Get the new size of the data to aggregate
    while (it.hasNext())
    {
        this->_onRead(*it.peekNext());
        newSize += it.peekNext()->size();
        it.next();
    }
    this->data.resize(newSize);
    it.toFront();
    // Aggregates the new data
    while (it.hasNext())
    {
        newSize = it.peekNext()->size();
        memcpy(this->data.data() + oldSize, it.peekNext()->data(), newSize);
        oldSize += newSize;
        it.next();
    }
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

    if (Log::instance()->isTrace())
        Log::trace("Data received", Properties("id", this->client.getId()).add("data", Tools::simplify(data)).add("size", data.size()), "Engine", "_onRead");
    else if (Log::instance()->isDebug())
        Log::debug("Data received", Properties("id", this->client.getId()).add("size", data.size()), "Engine", "_onRead");
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
