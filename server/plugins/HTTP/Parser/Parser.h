#ifndef PARSER_H
# define PARSER_H

# include <QByteArray>
# include "IClient.h"

# define END_OF_LINE            "\r\n"      // End of line characters.
# define END_OF_HEADER          "\r\n\r\n"  // End of header characters.

class Parser
{
public:
    Parser(Streamit::IClient *client = NULL);
    ~Parser();
    Parser(const Parser &parser);
    Parser  &operator=(const Parser &parser);

    /// @brief Find the protocol of the client (HTTP or WebClient).
    bool    onProtocol(const QByteArray &data, QString &protocol, bool &error);
    /// @brief Unserialize the header.
    bool    doUnserializeHeader(const QByteArray &data, quint64 &used);
    /// @brief Unserialize the content.
    bool    doUnserializeContent(const QByteArray &data, quint64 &used);
    /// @brief Serialize the header.
    void    doSerializeHeader(QByteArray &data);
    /// @brief Serialize the content.
    bool    doSerializeContent(QByteArray &data);

private:
    // Unserialize Header
    /// @brief Check that the characters are correct.
    bool    _checkHeaderCharacters();
    /// @brief Check that the first line is correct, and unserialize its data.
    bool    _parseHeaderFirstLine();
    /// @brief Check that the properties are correct.
    bool    _parseHeaderProperties();
    /// @brief Send an error to the client.
    bool    _error(int code, const QString &message, const QByteArray &content = "");

    Streamit::IClient   *client;
    Streamit::IRequest  *request;
    Streamit::IResponse *response;
    QByteArray          header;
    quint64             contentSent;
};

#endif // PARSER_H
