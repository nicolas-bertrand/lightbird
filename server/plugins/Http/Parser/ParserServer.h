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
    bool    doUnserializeHeader(const QByteArray &data, quint64 &used);
    bool    doUnserializeContent(const QByteArray &data, quint64 &used);
    void    doSerializeHeader(QByteArray &data);
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

    quint64 contentLength;  ///< The length of the content of the request.
};

#endif // PARSERSERVER_H
