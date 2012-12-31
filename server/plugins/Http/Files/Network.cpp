#include "Network.h"
#include "Plugin.h"

Network::Network(Plugin *p)
    : plugin(p)
{
}

bool    Network::doExecution(LightBird::IClient &client)
{
    return (this->plugin->doExecution(client));
}
