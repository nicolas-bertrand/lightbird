#ifndef PARSER_H
# define PARSER_H

# include "IOnSerialize.h"
# include "IPlugin.h"

class Parser
{
public:
    Parser(LightBird::IApi *api, LightBird::IClient &client);
    virtual ~Parser();

    virtual bool doUnserializeContent(const QByteArray &data, quint64 &used) = 0;
    virtual bool doSerializeContent(QByteArray &data) = 0;
    virtual bool onExecution();
    virtual bool onSerialize(LightBird::IOnSerialize::Serialize type);
    virtual void onFinish();
    virtual bool onDisconnect();

protected:
    LightBird::IApi     *api;
    LightBird::IClient  &client;
};

#endif // PARSER_H
