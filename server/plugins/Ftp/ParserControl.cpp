#include "ParserControl.h"

ParserControl::ParserControl(LightBird::IApi *api, LightBird::IClient *client) : Parser(api, client)
{
}

ParserControl::~ParserControl()
{
}

bool        ParserControl::doUnserializeContent(const QByteArray &data, quint64 &used)
{
    int     i;
    int     j;

    // End of line found
    if (this->buffer.endsWith('\r') && data.startsWith('\n'))
    {
        this->buffer.chop(1);
        used = 1;
    }
    else if ((i = data.indexOf("\r\n")) >= 0)
    {
        this->buffer.append(QString::fromUtf8(data.constData(), i));
        used = i + 2;
    }
    // Still no end of line
    else
        this->buffer.append(QString::fromUtf8(data.constData(), data.size()));
    // Separates the command and its parameter
    if (used)
    {
        QString command = this->buffer.trimmed();
        QString parameter;
        if ((j = command.indexOf(' ')) >= 0)
        {
            parameter = this->buffer.right(command.size() - j - 1);
            command.truncate(j);
        }
        command = command.toUpper();
        this->client->getRequest().setMethod(command);
        this->client->getRequest().getInformations()["parameter"] = parameter;
        this->buffer.clear();
    }
    // The line is too long
    else if (this->buffer.size() > MAX_LINE_SIZE)
        this->buffer.clear();
    return (used != 0);
}

bool    ParserControl::doSerializeContent(QByteArray &data)
{
    QString message = this->client->getResponse().getMessage();
    if (message.endsWith("\r\n"))
        message.chop(2);
    QStringList lines = message.split("\r\n");
    QMutableStringListIterator it(lines);
    while (it.hasNext())
    {
        QString &line = it.next();
        line.prepend(QString::number(this->client->getResponse().getCode()) + (it.hasNext() ? "-" : " "));
        line.append("\r\n");
    }
    data.append(lines.join(""));
    return (true);
}

bool    ParserControl::onExecution()
{
    // If the code is zero, don't send a response
    return (this->client->getResponse().getCode() != 0);
}

