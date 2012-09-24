#ifndef PARSERDATA_H
# define PARSERDATA_H

# include "Parser.h"

/// @brief Parses the data connection.
class ParserData : public Parser
{
public:
    ParserData(LightBird::IApi *api, LightBird::IClient &client);
    ~ParserData();

    bool    doUnserializeContent(const QByteArray &data, quint64 &used);
    bool    doSerializeContent(QByteArray &data);
    void    onFinish();
    bool    onSerialize(LightBird::IOnSerialize::Serialize type);
    bool    onDisconnect();
};

#endif // PARSERDATA_H
