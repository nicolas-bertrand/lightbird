#ifndef PARSER_H
# define PARSER_H

# include <QByteArray>
# include <QString>

# include "IClient.h"

# define END_OF_LINE            "\r\n"      // End of line characters.
# define END_OF_HEADER          "\r\n\r\n"  // End of header characters.

class Parser
{
public:
    Parser(LightBird::IClient &client);
    virtual ~Parser();

    /// @brief Search the protocol of the client (HTTP).
    virtual bool    onProtocol(const QByteArray &data, QString &protocol, bool &error);
    /// @brief Unserialize the header.
    virtual bool    doUnserializeHeader(const QByteArray &data, quint64 &used) = 0;
    /// @brief Unserialize the content.
    virtual bool    doUnserializeContent(const QByteArray &data, quint64 &used) = 0;
    /// @brief Serialize the header.
    virtual void    doSerializeHeader(QByteArray &data) = 0;
    /// @brief Serialize the content.
    virtual bool    doSerializeContent(QByteArray &data) = 0;

protected:
    LightBird::IClient   &client;
    LightBird::IRequest  &request;
    LightBird::IResponse &response;
    QByteArray           header;
    quint64              contentSent;
};

#endif // PARSER_H
