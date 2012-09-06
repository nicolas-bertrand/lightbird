#include <QtPlugin>
#include <QDomNode>

#include "Plugin.h"
#include "IApi.h"
#include "IRequest.h"

#include "ClientHandler.h"

Plugin::Plugin()
{
}

Plugin::~Plugin()
{
}

bool    Plugin::onLoad(LightBird::IApi *api)
{
    this->api = api;
    this->handler = new ClientHandler(api);
    return (true);
}


void    Plugin::onUnload()
{
    delete this->handler;
    this->handler = NULL;
}

bool    Plugin::onInstall(LightBird::IApi *api)
{
    this->api = api;
    return (true);
}

void    Plugin::onUninstall(LightBird::IApi *api)
{
    this->api = api;
}

void    Plugin::getMetadata(LightBird::IMetadata &metadata) const
{
    metadata.name = "Execute FTP";
    metadata.brief = "The FTP executor.";
    metadata.description = "Executes FTP requests.";
    metadata.autor = "LightBird team";
    metadata.site = "lightbird.cc";
    metadata.email = "team@lightbird.cc";
    metadata.version = "1.0";
    metadata.licence = "CC BY-NC-SA 3.0";
}

bool    Plugin::onConnect(LightBird::IClient &client)
{
    bool ok = false;

    this->mutex.lockForWrite();
    
    if (client.getProtocols().first() == "FTP")
    {
        ok = handler->onConnect(&client);
    }
    else if (client.getProtocols().first() == "FTP-DATA")
    {
        ok = handler->onDataConnect(&client);
    }

    
    this->mutex.unlock();
    return ok;
}

bool    Plugin::doExecution(LightBird::IClient &client)
{
    bool ret = false;
    if (client.getRequest().getProtocol() == "FTP")
    {
        ret = handler->doControlExecute(&client);
    }
    else if (client.getRequest().getProtocol() == "FTP-DATA")
    {
        ret = handler->doDataExecute(&client);
    }

    return ret;
}

bool    Plugin::doSend(LightBird::IClient &client)
{
    bool ok = false;

    this->mutex.lockForWrite();
    
    if (client.getRequest().getProtocol() == "FTP-DATA")
    {
        ok = handler->doDataExecute(&client);
    }
    
    this->mutex.unlock();
    return ok;
}

void    Plugin::onFinish(LightBird::IClient &client)
{
    if (client.getRequest().isError() || client.getResponse().isError())
        this->api->network().disconnect(client.getId());
}

Q_EXPORT_PLUGIN2(Plugin, Plugin)
