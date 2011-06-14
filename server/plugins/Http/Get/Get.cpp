#include <QtPlugin>

#include "Get.h"

Get::Get()
{
}

Get::~Get()
{
}

bool    Get::onLoad(LightBird::IApi *api)
{
    this->api = api;
    return (true);
}

void    Get::onUnload()
{
}

bool    Get::onInstall(LightBird::IApi *api)
{
    this->api = api;
    return (true);
}

void    Get::onUninstall(LightBird::IApi *api)
{
    this->api = api;
}

void    Get::getMetadata(LightBird::IMetadata &metadata) const
{
    metadata.name = "HTTP Get";
    metadata.brief = "Implements the GET HTTP method.";
    metadata.description = "Allows clients to download files from the server, and get data of its files.";
    metadata.autor = "LightBird team";
    metadata.site = "lightbird.cc";
    metadata.email = "team@lightbird.cc";
    metadata.version = "1.0";
    metadata.licence = "CC BY-NC-SA 3.0";
}

bool    Get::doExecution(LightBird::IClient &client)
{
    LightBird::IMetadata metadata;

    if (client.getMode() == LightBird::IClient::SERVER)
    {
        metadata = this->api->plugins().getMetadata(this->api->getId());
        QString s = this->api->network().connect(QHostAddress("188.165.253.129"), 80, QStringList("HTTP"))->getResult();
        if (!this->api->network().send(s))
            s = "hello world !";
        else
        {
            this->mutex.lock();
            this->wait.wait(&this->mutex);
            this->mutex.unlock();
            s = this->string;
        }
        client.getResponse().getContent().setContent(s.toAscii());
    }
    return (true);
}

bool    Get::doSend(LightBird::IClient &client)
{
    client.getRequest().getHeader().insert("Host", "pcinpact.com");
    client.getRequest().getHeader().insert("User-Agent", "Mozilla/5.0 (Windows NT 6.1; WOW64; rv:2.0.1) Gecko/20100101 Firefox/4.0.1");
    client.getRequest().getHeader().insert("Accept", "text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8");
    client.getRequest().getHeader().insert("Accept-Language", "fr,fr-fr;q=0.8,en-us;q=0.5,en;q=0.3");
    client.getRequest().getHeader().insert("Accept-Charset", "ISO-8859-1,utf-8;q=0.7,*;q=0.7");
    client.getRequest().getHeader().insert("Keep-Alive", "115");
    client.getRequest().getHeader().insert("Connection", "keep-alive");
    client.getRequest().getHeader().insert("Cache-Control", "max-age=0");
    return (true);
}

void    Get::onUnserialize(LightBird::IClient &client, LightBird::IOnUnserialize::Unserialize type)
{
    if (client.getMode() == LightBird::IClient::CLIENT && type == LightBird::IOnUnserialize::IDoUnserialize)
    {
        this->string = client.getResponse().getContent().getContent();
        this->wait.wakeAll();
    }
}

Q_EXPORT_PLUGIN2(Get, Get)
