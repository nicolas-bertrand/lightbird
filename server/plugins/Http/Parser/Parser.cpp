#include <QMap>
#include <QStringList>

#include "Parser.h"
#include "Plugin.h"

Parser::Parser(LightBird::IClient *c) : client(c), request(&c->getRequest()), response(&c->getResponse())
{
    this->contentSent = 0;
}

Parser::~Parser()
{
}

Parser::Parser(const Parser &parser)
{
    *this = parser;
}

Parser  &Parser::operator=(const Parser &parser)
{
    if (this != &parser)
    {
        this->client = parser.client;
        this->request = parser.request;
        this->response = parser.response;
        this->contentSent = 0;
    }
    return (*this);
}

bool    Parser::onProtocol(const QByteArray &data, QString &protocol, bool &error)
{
    int j = data.indexOf('\r');
    int i = data.indexOf(" HTTP");

    // Search the protocol used
    if (i < j && i >= 0)
    {
        protocol = "HTTP";
        return (true);
    }
    // The protocol of the request is unknown
    if (j >= 0 || data.size() > (int)Plugin::getConfiguration().maxHeaderSize)
        error = true;
    return (false);
}

bool    Parser::doUnserializeHeader(const QByteArray &data, quint64 &used)
{
    this->request->getContent().setStorage(LightBird::IContent::BYTEARRAY);
    this->contentSent = 0;
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
            this->request->setError();
        this->header.clear();
        // The header has been completely received
        return (true);
    }
    // If the header is too big, an error is sent to the client
    else if ((quint32)this->header.size() > Plugin::getConfiguration().maxHeaderSize)
    {
        this->request->setError();
        this->response->setCode(400);
        this->response->setMessage("Bad Request");
        this->response->getContent().setContent("Error: the header size of a request can't be larger than " + QByteArray::number(Plugin::getConfiguration().maxHeaderSize) + " bytes.", false);
    }
    // The header is not complete
    return (false);
}

bool        Parser::doUnserializeContent(const QByteArray &data, quint64 &used)
{
    quint64 contentLength;
    quint64 rest;

    // If there is a content
    if ((contentLength = this->request->getHeader().value("content-length").toULongLong()) > 0)
    {
        // The first time a content is received, we determine if it has to be stored in a temporary file or in the memory
        if (this->request->getContent().size() == 0 && contentLength > Plugin::getConfiguration().maxContentInMemory)
            this->request->getContent().setStorage(LightBird::IContent::TEMPORARYFILE);
        // Calculates the size of the remaining content to receive
        rest = contentLength - this->request->getContent().size();
        // All the data has been used
        if ((quint64)data.size() <= rest)
        {
            this->request->getContent().setContent(data);
            used = data.size();
        }
        // Only a part of the data has been used
        else
        {
            this->request->getContent().setContent(data.left(rest));
            used = rest;
        }
        // All the content has been reveived
        if (contentLength <= (quint64)this->request->getContent().size())
            return (true);
        // All the content has not been received yet
        return (false);
    }
    // There is no content
    return (true);
}

void            Parser::doSerializeHeader(QByteArray &data)
{
    QByteArray  version = this->request->getVersion().toAscii();
    QByteArray  code = "200";
    QByteArray  message = "OK";
    QString     type;

    // Defines the first line
    if (!this->response->getVersion().isEmpty())
        version = this->response->getVersion().toAscii();
    else if (version.isEmpty())
        version = "HTTP/1.1";
    if (this->response->getCode())
        code = QByteArray::number(this->response->getCode());
    if (!this->response->getMessage().isEmpty())
        message = this->response->getMessage().toAscii();
    data.append(version + " " + code + " " + message + END_OF_LINE);
    // If the content length is not defined, we fill it
    if (!this->response->getHeader().contains("content-length"))
        this->response->getHeader().insert("content-length", QString::number(this->response->getContent().size()));
    // Adds the type
    if (!(type = this->response->getType()).isEmpty())
        this->response->getHeader().insert("content-type", type);
    // Write the properties
    QMapIterator<QString, QString> it(this->response->getHeader());
    while (it.hasNext())
    {
        it.next();
        data.append(it.key() + ": " + it.value() + END_OF_LINE);
    }
    data.append(END_OF_LINE);
}

bool        Parser::doSerializeContent(QByteArray &data)
{
    quint64 length;

    // If there is a content to send
    if ((length = this->response->getHeader().value("content-length").toULongLong()) && length > this->contentSent)
    {
        // Get the data
        if (length < Plugin::getConfiguration().maxPacketSize)
            data = this->response->getContent().getContent(length);
        else
            data = this->response->getContent().getContent(Plugin::getConfiguration().maxPacketSize);
        this->contentSent += data.size();
        // More data have to be sent
        if (this->contentSent < length)
            return (false);
        // If there is not enough data, they are pad
        else if (!data.size())
            data.append(QByteArray(length - this->contentSent, 0));
    }
    return (true);
}

bool            Parser::_checkHeaderCharacters()
{
    int         i;
    int         s;
    char        c;
    QByteArray  content;

    for (i = 0, s = this->header.size() - 4; i < s; ++i)
    {
        c = this->header.at(i);
        // Check that the characters are printable ascii
        if ((c < 32 || c >= 127) && c != '\r' && c != '\n')
            return (this->_error(400, "Bad Request", "Error on character " + QByteArray::number(i) + " of the header: The header of the request must contain only printable ascii characters, or \\r and \\n for the end of line."));
        // Check if the end of line characters are correct
        if ((c == '\r' && this->header.at(i + 1) != '\n') ||
            (i > 0 && c == '\n' && this->header.at(i - 1) != '\r') ||
            (c == '\n' && this->header.at(i + 1) == '\r'))
            return (this->_error(400, "Bad Request", "Error on character " + QByteArray::number(i) + " of the header: Invalid end of line. The correct EOF format is \\r\\n."));
    }
    return (true);
}

bool            Parser::_parseHeaderFirstLine()
{
    QByteArray  line;
    QByteArray  uri;

    // Separates the first line
    line = this->header.left(this->header.indexOf(END_OF_LINE));
    this->header = this->header.right(this->header.size() - this->header.indexOf(END_OF_LINE) - 2);
    // There must be 2 spaces
    if (line.count(' ') != 2)
        return (this->_error(400, "Bad Request", "The first line of the header of a request must be in the following format: \"METHOD URI VERION\"."));
    // Find the method
    this->request->setMethod(line.left(line.indexOf(' ')));
    if (this->request->getMethod().isEmpty())
        return (this->_error(400, "Bad Request", "The method of the header must not be empty."));
    if (!this->request->getMethod().contains(QRegExp("^[A-Z]+$")))
        return (this->_error(400, "Bad Request", "The method must contain only uppercase characters."));
    line = line.right(line.size() - this->request->getMethod().size() - 1);
    // Find the URI
    uri = line.left(line.indexOf(' '));
    this->request->setUri(QUrl(uri));
    if (uri.isEmpty())
        return (this->_error(400, "Bad Request", "The uri of the header must not be empty. It must be at least \"/\"."));
    if (!this->request->getUri().isValid())
        return (this->_error(400, "Bad Request", "The uri is not valid."));
    // Find the version
    this->request->setVersion(line.right(line.size() - uri.size() - 1));
    if (!Plugin::getConfiguration().protocols.contains(this->request->getVersion()))
        return (this->_error(505, "Version Not Supported", "This version of the protocol is not supported."));
    return (true);
}

bool    Parser::_parseHeaderProperties()
{
    QString                     line;
    QString                     key;
    QString                     value;
    int                         l;
    QMap<QString, QStringList>  multiMap;
    QMap<QString, QString>      &header = this->request->getHeader();

    l = 0;
    while (this->header.size() > 4)
    {
        l++;
        // Separates the line
        line = this->header.left(this->header.indexOf(END_OF_LINE)).data();
        this->header = this->header.right(this->header.size() - line.size() - 2);
        if (line.count(':') < 1)
            return (this->_error(400, "Bad Request", "Invalid property on line " + QByteArray::number(l + 1) + " of the header, there is no character \":\"."));
        key = line.left(line.indexOf(':')).trimmed().toLower();
        if (key.isEmpty())
            return (this->_error(400, "Bad Request", "Error on line " + QByteArray::number(l + 1) + " of the header: The name of a property cannot be empty."));
        value = line.right(line.size() - line.indexOf(':') - 1).trimmed();
        if (!header.contains(key))
            header.insertMulti(key, value);
        else
            multiMap[key].push_front(value);
        if (key == "content-type")
            this->request->setType(value);
    }
    // Add the values that have the same key to the header
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
    // If the request has a content for a method that is not allowed
    if (this->request->getHeader().contains("content-length") &&
        this->request->getHeader().value("content-length").toUInt() != 0 &&
        !Plugin::getConfiguration().methodContent.contains(this->request->getMethod()))
        return (this->_error(400, "Bad Request", "This method doesn't require a content."));
    return (true);
}

bool    Parser::_error(int code, const QString &message, const QByteArray &content)
{
    this->response->setCode(code);
    this->response->setMessage(message);
    if (!content.isEmpty())
        this->response->getContent().setContent(content, false);
    return (false);
}
