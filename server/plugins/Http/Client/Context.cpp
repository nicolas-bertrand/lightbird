#include "Context.h"
#include "Plugin.h"

void    Context::onDeserialize(LightBird::IClient &client, LightBird::IOnDeserialize::Deserialize type)
{
    Plugin::instance().onDeserializeContext(client, type);
}

bool    Context::doExecution(LightBird::IClient &client)
{
    return (Plugin::instance().doExecution(client));
}

bool    Context::onSerialize(LightBird::IClient &client, LightBird::IOnSerialize::Serialize type)
{
    return (Plugin::instance().onSerialize(client, type));
}

void    Context::onFinish(LightBird::IClient &client)
{
    Plugin::instance().onFinish(client);
}

bool    Context::onDisconnect(LightBird::IClient &client)
{
    return (Plugin::instance().onDisconnect(client));
}

void    Context::onDestroy(LightBird::IClient &client)
{
    Plugin::instance().onDestroy(client);
}
