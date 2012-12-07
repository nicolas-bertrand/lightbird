#include <QMap>
#include <QStringList>

#include "Parser.h"
#include "Plugin.h"

Parser::Parser(LightBird::IClient &c)
    : client(c)
    , request(c.getRequest())
    , response(c.getResponse())
{
    this->contentSent = 0;
}

Parser::~Parser()
{
}

bool    Parser::onProtocol(const QByteArray &, QString &, bool &)
{
    return (false);
}

bool    Parser::doSerializeContent(LightBird::IContent &content, QByteArray &data)
{
    // If there is a content to send
    if (this->contentLength && this->contentLength > this->contentSent)
    {
        // Get the data
        if (this->contentLength < Plugin::getConfiguration().maxPacketSize)
            data = content.getData(this->contentLength);
        else
            data = content.getData(Plugin::getConfiguration().maxPacketSize);
        // If there is not enough data, they are padded
        if (data.isEmpty())
            data.append(QByteArray(((this->contentLength - this->contentSent) < Plugin::getConfiguration().maxPacketSize) ?
                                   (this->contentLength - this->contentSent) : Plugin::getConfiguration().maxPacketSize, 0));
        this->contentSent += data.size();
        // More data have to be sent
        if (this->contentSent < this->contentLength)
            return (false);
    }
    // Chunk transfer encoding
    else if (this->chunkEncoding)
    {
        qint64 size = content.size();
        // Writes a new chunk
        if (size)
        {
            data = QString::number(size, 16).toAscii() + END_OF_LINE;
            data += content.getData(size) + END_OF_LINE;
            return (false);
        }
        // The last chunk
        else
            data.append("0").append(END_OF_LINE);
    }
    return (true);
}
