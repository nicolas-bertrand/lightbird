#include "Parser.h"

Parser::Parser(LightBird::IApi &api, LightBird::IClient &client) : api(api), client(client)
{
}

Parser::~Parser()
{
}

bool    Parser::onExecution()
{
    return (true);
}

bool    Parser::onSerialize(LightBird::IOnSerialize::Serialize)
{
    return (true);
}

void    Parser::onFinish()
{
}

bool    Parser::onDisconnect()
{
    return (false);
}

void    Parser::onDestroy()
{
}
