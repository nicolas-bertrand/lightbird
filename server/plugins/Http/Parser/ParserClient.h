#ifndef PARSERCLIENT_H
# define PARSERCLIENT_H

# include <QByteArray>
# include <QString>

# include "IClient.h"

# include "Parser.h"

class ParserClient : public Parser
{
public:
    ParserClient(LightBird::IClient &client);
    ~ParserClient();

    // Parser
    void    doSerializeHeader(QByteArray &data);
    bool    doSerializeContent(QByteArray &data);
    bool    doUnserializeHeader(const QByteArray &data, quint64 &used);
    bool    doUnserializeContent(const QByteArray &data, quint64 &used);

private:
    // Unserialize Header
    /// @brief Check that the characters are correct.
    bool    _checkHeaderCharacters();
    /// @brief Check that the first line is correct, and unserialize its data.
    bool    _parseHeaderFirstLine();
    /// @brief Check that the properties are correct.
    bool    _parseHeaderProperties();
    /// @brief Parse the content-type and its parameters.
    void    _parseContentType(const QByteArray &value);

    quint64 contentLength;  ///< The length of the content of the request.
};

#endif // PARSERCLIENT_H
