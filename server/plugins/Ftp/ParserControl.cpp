#include "ClientHandler.h"
#include "ParserControl.h"

ParserControl::ParserControl(LightBird::IApi &api, LightBird::IClient &client) : Parser(api, client)
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
        // Left trim
        for (i = 0, j = this->buffer.size(); this->buffer.at(i) == ' ' && i < j; ++i)
            ;
        QString command = this->buffer.right(this->buffer.size() - i);
        QString parameter;
        if ((j = command.indexOf(' ')) >= 0)
        {
            parameter = command.right(command.size() - j - 1);
            command.truncate(j);
        }
        command = command.toUpper();
        this->client.getRequest().setMethod(command);
        this->client.getRequest().getInformations()["parameter"] = parameter;
        this->buffer.clear();
    }
    // The command line is too long
    else if (this->buffer.size() > MAX_LINE_SIZE)
    {
        this->buffer.clear();
        used = data.size();
        QVariantMap &informations = this->client.getRequest().getInformations();
        informations[CONTROL_SEND_MESSAGE] = true;
        informations[CONTROL_CODE] = 500;
        informations[CONTROL_MESSAGE] = "Command line too long";
    }
    return (used != 0);
}

bool    ParserControl::doSerializeContent(QByteArray &data)
{
    QString message = this->client.getResponse().getMessage();
    int code = this->client.getResponse().getCode();

    if (code != 0)
    {
        if (message.endsWith("\r\n"))
            message.chop(2);
        QStringList lines = message.split("\r\n");
        QMutableStringListIterator it(lines);
        while (it.hasNext())
        {
            QString &line = it.next();
            line.prepend(QString::number(this->client.getResponse().getCode()) + (it.hasNext() ? "-" : " "));
            line.append("\r\n");
        }
        data = lines.join("").toUtf8();
    }
    // Send the message as it is. May be empty for the transfer methods.
    else
        data = message.toUtf8();
    return (true);
}

bool    ParserControl::onExecution()
{
    // Ensures that a response is needed
    return (!this->client.getResponse().getMessage().isEmpty());
}

void    ParserControl::onDestroy()
{
    LightBird::Session  session = this->client.getSession();

    // Destroy the session if there is no data connection
    if (session)
    {
        session->removeInformation(SESSION_CONTROL_ID);
        session->removeClient(this->client.getId());
        if (session->getInformation(SESSION_DATA_ID).toString().isEmpty())
            session->destroy();
    }
}
