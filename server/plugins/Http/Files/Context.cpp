#include "Context.h"
#include "Plugin.h"

Context::Context(Plugin *p)
    : plugin(p)
{
}

bool    Context::doExecution(LightBird::IClient &client)
{
    return (this->plugin->doExecution(client));
}
