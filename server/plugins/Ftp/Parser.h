#ifndef PARSER_H
# define PARSER_H

# include "IClient.h"
# include "IPlugin.h"

class Parser
{
public:
    Parser(LightBird::IApi &api, LightBird::IClient &client);
    virtual ~Parser();

    virtual bool doDeserializeContent(const QByteArray &data, quint64 &used) = 0;
    virtual bool doSerializeContent(QByteArray &data) = 0;

protected:
    LightBird::IApi     &api;
    LightBird::IClient  &client;
};

#endif // PARSER_H
