#ifndef PARSERSERVER_H
# define PARSERSERVER_H

# include <QByteArray>
# include <QString>

# include "IClient.h"

# include "Parser.h"

class ParserServer : public Parser
{
public:
    ParserServer(LightBird::IClient &client);
    ~ParserServer();

    // Parser
    bool    onProtocol(const QByteArray &data, QString &protocol, bool &error);
    bool    doDeserializeHeader(const QByteArray &data, quint64 &used);
    bool    doDeserializeContent(const QByteArray &data, quint64 &used);
    void    doSerializeHeader(QByteArray &data);
    bool    doSerializeContent(QByteArray &data);

private:
    // Deserialize Header
    /// @brief Check that the characters are correct.
    bool    _checkHeaderCharacters();
    /// @brief Check that the first line is correct, and deserialize its data.
    bool    _parseHeaderFirstLine();
    /// @brief Check that the properties are correct.
    bool    _parseHeaderProperties();
    /// @brief Parse the content-type and its parameters.
    void    _parseContentType(const QByteArray &value);
    /// @brief Send an error to the client.
    bool    _error(int code, const QString &message, const QByteArray &content = "");

    quint64 contentLength;  ///< The length of the content of the request.
    quint64 contentStored;  ///< The size of the content already stored.
};

#endif // PARSERSERVER_H
