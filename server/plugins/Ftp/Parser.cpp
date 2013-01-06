#include "Parser.h"

Parser::Parser(LightBird::IApi &api, LightBird::IClient &client)
    : api(api)
    , client(client)
{
}

Parser::~Parser()
{
}
