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
    /// @brief Deserialize the header.
    virtual bool    doDeserializeHeader(const QByteArray &data, quint64 &used) = 0;
    /// @brief Deserialize the content.
    virtual bool    doDeserializeContent(const QByteArray &data, quint64 &used) = 0;
    /// @brief Serialize the header.
    virtual void    doSerializeHeader(QByteArray &data) = 0;
    /// @brief Serialize the content.
    virtual bool    doSerializeContent(QByteArray &data) = 0;

protected:
    bool    doSerializeContent(LightBird::IContent &content, QByteArray &data);

    LightBird::IClient   &client;
    LightBird::IRequest  &request;
    LightBird::IResponse &response;
    QByteArray           header;
    quint64              contentLength; ///< The length of the content of the request.
    quint64              chunkEncoding; ///< The data are send in chunks. Used when the content is dynamically generating.
    quint64              contentSent;   ///< The number of bytes of content sent so far.
};

#endif // PARSER_H
