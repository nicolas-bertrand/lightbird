#include <QMap>
#include <QStringList>

#include "Parser.h"
#include "Plugin.h"

Parser::Parser(LightBird::IClient &c) : client(c), request(c.getRequest()), response(c.getResponse())
{
    this->contentSent = 0;
}

Parser::~Parser()
{
}

bool    Parser::onProtocol(const QByteArray &, QString &, bool &)
{
    return (false);
}
