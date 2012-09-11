#include "ControlParser.h"

ControlParser::ControlParser(LightBird::IApi *api, LightBird::IClient *client) : Parser(api, client)
{
}

bool        ControlParser::doUnserializeContent(const QByteArray &data, quint64 &used)
{
    bool    result = false;
    QString command = QString::fromUtf8(data.constData(), data.size());
    int     i = command.indexOf("\r\n");

    if (i >= 0)
    {
        result = true;
        used = i + 2;
        command.truncate(i); // Keep only the first line of data
        QString verb = command, parameter;
        int j = -1;
        if ((j = command.indexOf(' ')) >= 0)
        {
            verb.truncate(j);
            parameter = command;
            parameter.remove(0,j+1);
        }
        this->client->getRequest().setMethod(verb);
        this->client->getRequest().getContent().setStorage(LightBird::IContent::VARIANT);
        *this->client->getRequest().getContent().getVariant() = parameter;
    }
    return (result);
}

bool ControlParser::doSerializeContent(QByteArray &data)
{
    QString message =  client->getResponse().getMessage();
    if (message.endsWith("\r\n"))
        message.chop(2);
    QStringList lines = message.split("\r\n");
    QMutableStringListIterator it(lines);
    while (it.hasNext())
    {
        QString &line = it.next();
        line.prepend(QString::number(client->getResponse().getCode()) + (it.hasNext() ? "-" : " "));
        line.append("\r\n");
    }
    data.append(lines.join(""));
    return (true);
}

bool ControlParser::onExecution()
{
    return (client->getResponse().getCode() != 0); // If the code is zero, don't send a response
}

