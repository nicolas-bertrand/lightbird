#ifndef CONTROLPARSER_H
# define CONTROLPARSER_H

# include "Parser.h"

class ControlParser : public Parser
{
    public:
        ControlParser(LightBird::IApi *api, LightBird::IClient *client);

        bool doUnserializeContent(const QByteArray &data, quint64 &used);
        bool doSerializeContent(QByteArray &data);
        bool onExecution();
};

#endif // CONTROLPARSER_H
