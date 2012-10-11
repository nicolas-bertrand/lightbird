#ifndef PARSERCONTROL_H
# define PARSERCONTROL_H

# include "Parser.h"

# define MAX_LINE_SIZE 10000

/// @brief Parses the control connection.
class ParserControl : public Parser
{
public:
    ParserControl(LightBird::IApi &api, LightBird::IClient &client);
    ~ParserControl();

    bool    doDeserializeContent(const QByteArray &data, quint64 &used);
    bool    doSerializeContent(QByteArray &data);
    bool    onExecution();
    void    onDestroy();

private:
    QString buffer;
};

#endif // PARSERCONTROL_H
