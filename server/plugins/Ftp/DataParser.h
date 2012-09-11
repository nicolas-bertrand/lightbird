#ifndef DATAPARSER_H
# define DATAPARSER_H

# include "Parser.h"

class DataParser : public Parser
{
    public:
        DataParser(LightBird::IApi *api, LightBird::IClient *client);

        bool doUnserializeContent(const QByteArray &data, quint64 &used);
        bool doSerializeContent(QByteArray &data);
        void onFinish();
        bool onSerialize(LightBird::IOnSerialize::Serialize type);
        bool onDisconnect();
};

#endif // DATAPARSER_H
