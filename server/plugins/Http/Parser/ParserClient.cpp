#include <QMap>
#include <QStringList>

#include "ParserClient.h"
#include "Plugin.h"

ParserClient::ParserClient(LightBird::IClient &client) : Parser(client)
{
    this->contentSent = 0;
}

ParserClient::~ParserClient()
{
}

void            ParserClient::doSerializeHeader(QByteArray &data)
{
    QByteArray  method;
    QByteArray  uri;
    QByteArray  version;
    QByteArray  type;

    // Defines the first line
    if ((method = this->request.getMethod().toAscii()).isEmpty())
        method = "GET";
    if ((uri = this->request.getUri().toString().toAscii()).isEmpty())
        uri = "/";
    if ((version = this->request.getVersion().toAscii()).isEmpty())
        version = "HTTP/1.1";
    data.append(method + " " + uri + " " + version + END_OF_LINE);
    // There is no content-length in chunk transfer encoding
    if ((this->chunkEncoding = (this->response.getHeader().value("transfer-encoding") == "chunked")))
        this->request.getHeader().remove("content-length");
    // If the content length is not defined, we fill it
    else if (!this->request.getHeader().contains("content-length"))
        this->request.getHeader().insert("content-length", QString::number(this->request.getContent().size()));
    // Adds the type
    if (!(type = this->request.getType().toAscii()).isEmpty())
        this->request.getHeader().insert("content-type", type);
    // Write the properties
    QMapIterator<QString, QString> it(this->request.getHeader());
    while (it.hasNext())
    {
        it.next();
        data.append(it.key() + ": " + it.value() + END_OF_LINE);
    }
    data.append(END_OF_LINE);
    this->contentLength = this->request.getHeader().value("content-length").toULongLong();
}

bool        ParserClient::doSerializeContent(QByteArray &data)
{
    return (Parser::doSerializeContent(this->request.getContent(), data));
}

bool    ParserClient::doDeserializeHeader(const QByteArray &data, quint64 &used)
{
    this->header.append(data);
    // If the header contains the characteres END_OF_HEADER, this mean that all the header
    // has been received. The data after that is the content.
    if (this->header.contains(END_OF_HEADER) == true)
    {
        // Split the header from the content
        used = this->header.size();
        this->header.resize(this->header.indexOf(END_OF_HEADER) + strlen(END_OF_HEADER));
        used = this->header.size() - (used - data.size());
        // Parse the header
        if (!this->_checkHeaderCharacters() || !this->_parseHeaderFirstLine() || !this->_parseHeaderProperties())
            this->response.setError();
        this->header.clear();
        this->contentSent = 0;
        this->contentLength = this->response.getHeader().value("content-length").toULongLong();
        this->response.getContent().setStorage(LightBird::IContent::BYTEARRAY);
        // The header has been completely received
        return (true);
    }
    // If the header is too big, an error is sent to the client
    else if ((quint32)this->header.size() > Plugin::getConfiguration().maxHeaderSize)
    {
        used = data.size();
        this->response.setError();
        return (true);
    }
    // The header is not complete
    return (false);
}

bool        ParserClient::doDeserializeContent(const QByteArray &data, quint64 &used)
{
    quint64 rest;

    // If there is an error the content is not deserialized
    if (this->response.isError())
        return (true);
    // If there is a content
    if (this->contentLength > 0)
    {
        // The first time a content is received, we determine if it has to be stored in a temporary file or in the memory
        if (this->response.getContent().size() == 0 && this->contentLength > Plugin::getConfiguration().maxContentInMemory)
            this->response.getContent().setStorage(LightBird::IContent::TEMPORARYFILE);
        // Calculates the size of the remaining content to receive
        rest = this->contentLength - this->response.getContent().size();
        // All the data are used
        if ((quint64)data.size() <= rest)
        {
            this->response.getContent().setData(data);
            used = data.size();
        }
        // Only a part of the data are used
        else
        {
            this->response.getContent().setData(data.left(rest));
            used = rest;
        }
        // All the content has been reveived
        if (this->contentLength <= (quint64)this->response.getContent().size())
            return (true);
        // All the content has not been received yet
        return (false);
    }
    // There is no content
    return (true);
}

bool            ParserClient::_checkHeaderCharacters()
{
    int         i;
    int         s;
    char        c;

    for (i = 0, s = this->header.size() - 4; i < s; ++i)
    {
        c = this->header.at(i);
        // Check that the characters are printable ascii
        if ((c < 32 || c >= 127) && c != '\r' && c != '\n')
            return (false);
        // Check if the end of line characters are correct
        if ((c == '\r' && this->header.at(i + 1) != '\n') ||
            (i > 0 && c == '\n' && this->header.at(i - 1) != '\r') ||
            (c == '\n' && this->header.at(i + 1) == '\r'))
            return (false);
    }
    return (true);
}

bool            ParserClient::_parseHeaderFirstLine()
{
    QByteArray  line;

    // Separates the first line
    line = this->header.left(this->header.indexOf(END_OF_LINE));
    this->header = this->header.right(this->header.size() - this->header.indexOf(END_OF_LINE) - 2);
    // There must be at least 2 spaces
    if (line.count(' ') < 2)
        return (false);
    // Find the version
    this->response.setVersion(line.left(line.indexOf(' ')));
    if (!Plugin::getConfiguration().protocols.contains(this->response.getVersion()))
        return (false);
    line = line.right(line.size() - this->response.getVersion().size() - 1);
    // Find the code
    this->response.setCode(line.left(line.indexOf(' ')).toInt());
    // Find the message
    this->response.setMessage(line.right(line.size() - line.indexOf(' ') - 1));
    if (this->response.getMessage().isEmpty())
        return (false);
    return (true);
}

bool    ParserClient::_parseHeaderProperties()
{
    QByteArray                  line;
    QByteArray                  key;
    QByteArray                  value;
    int                         l;
    QMap<QString, QStringList>  multiMap;
    QMap<QString, QString>      &header = this->response.getHeader();

    l = 0;
    while (this->header.size() > 4)
    {
        l++;
        // Separates the line
        line = this->header.left(this->header.indexOf(END_OF_LINE)).data();
        this->header = this->header.right(this->header.size() - line.size() - 2);
        if (line.count(':') < 1)
            return (false);
        key = line.left(line.indexOf(':')).trimmed().toLower();
        if (key.isEmpty())
            return (false);
        value = line.right(line.size() - line.indexOf(':') - 1).trimmed();
        if (!header.contains(key))
            header.insert(key, value);
        else
            multiMap[key].push_front(value);
        if (key == "content-type")
            this->_parseContentType(value);
    }
    // Adds the values that have the same key to the header
    // This is done to ensure that the order of the values is kept
    QMapIterator<QString, QStringList> it1(multiMap);
    while (it1.hasNext())
    {
        it1.next();
        multiMap[it1.key()].push_back(header[it1.key()]);
        header.remove(it1.key());
        QStringListIterator it2(multiMap[it1.key()]);
        while (it2.hasNext())
        {
            header.insertMulti(it1.key(), it2.peekNext());
            it2.next();
        }
    }
    return (true);
}

void            ParserClient::_parseContentType(const QByteArray &value)
{
    QVariantMap parameters;
    int         i;

    QListIterator<QByteArray> it(value.split(';'));
    while (it.hasNext())
        if ((i = it.next().indexOf('=')) > 0)
            parameters.insert(it.peekPrevious().left(i).trimmed(), it.peekPrevious().right(it.peekPrevious().size() - i - 1));
    // Saves the parameters of the media type
    this->request.getInformations().insert("media-type", parameters);
    // Gets the content-type
    if ((i = value.indexOf(';')) < 0)
        this->request.setType(value);
    else
        this->request.setType(value.left(i));
}
