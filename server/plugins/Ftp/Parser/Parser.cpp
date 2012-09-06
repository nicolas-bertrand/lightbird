#include "Parser.h"

Parser::Parser(LightBird::IApi *api, LightBird::IClient *client) : api(api), client(client)
{
}

bool Parser::onExecution()
{
    return (true);
}

bool Parser::onSerialize(LightBird::IOnSerialize::Serialize type)
{
    return (true);
}

void Parser::onFinish()
{
}

bool Parser::onDisconnect()
{
    return (true);
}
