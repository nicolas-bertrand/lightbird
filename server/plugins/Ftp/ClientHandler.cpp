#include "ClientHandler.h"

#include "IClient.h"

ClientHandler::ClientHandler(LightBird::IApi *api) : api(api)
{
    this->commands = new Commands(api);
}

ClientHandler::~ClientHandler()
{
    delete commands;
}

bool ClientHandler::onConnect(LightBird::IClient &client)
{
    // Initialize a new session for this control connection
    LightBird::Session session = this->api->sessions().create();
    session->setClient(client.getId());
    session->setInformation("control-id", client.getId());
    session->setInformation("working-dir", "");
    session->setInformation("binary-flag", false);
    session->setInformation("transfer-mode", (int)Commands::TransferModeNone);
    session->setInformation("transfer-ip", QString());
    session->setInformation("transfer-port", 0);
    // Be polite and welcome the user
    Commands::Result greeting(220, "Welcome to Lightbird's FTP server.\r\n"
                                   "Please authenticate.\r\n"
                                   "And of course, have fun!\r\n");
    this->_sendControlMessage(client.getId(), greeting);
    return (true);
}

bool ClientHandler::doControlExecute(LightBird::IClient &client)
{
    Commands::Result     result(0, "");
    LightBird::IRequest  &request = client.getRequest();
    LightBird::IResponse &response = client.getResponse();
    LightBird::Session   session = client.getSession();
    QVariantMap          &info = request.getInformations();

    if (info.contains("send-message") && info.value("send-message").toBool())
    {
        result.first = info.value("code").toInt();
        result.second = info.value("message").toString();
    }
    else
    {
        QString command = request.getMethod().toUpper(); // Make sure we capitalize it, as we do case insensitive matching
        QString parameter = request.getInformations()["parameter"].toString();
        if (this->commands->isControl(command))
            result = this->commands->executeControl(command, parameter, session);
        else if (this->commands->isTransfert(command))
            result = this->_prepareTransferMethod(command, parameter, session);
        else
            result = Commands::Result(500, "Unknow command");
    }
    response.setCode(result.first);
    response.setMessage(result.second);
    return (true);
}

void ClientHandler::_sendControlMessage(QString id, Commands::Result message)
{
    QVariantMap informations;
    informations.insert("send-message", true); // The presence of the information indicates that we need to send a message and not process a request
    informations.insert("code", message.first); // The code of the message
    informations.insert("message", message.second); // And it's description
    this->api->network().send(id, "FTP", informations);
}

Commands::Result ClientHandler::_prepareTransferMethod(QString command, QString parameter, LightBird::Session session)
{
    Commands::TransferMode mode = (Commands::TransferMode)session->getInformation("transfer-mode").toInt();
    Commands::Result result;
    QString address = session->getInformation("transfer-ip").toString();
    int port = session->getInformation("transfer-port").toInt();

    if (session->getAccount().isEmpty())
        result = Commands::Result(530, "Please login with USER and PASS.");
    else if (mode == Commands::TransferModeNone)
        result = Commands::Result(425, "No Data Connection.");
    else
    {
        session->setInformation("pending-transfer-command", command);
        session->setInformation("pending-transfer-parameter", parameter);
        if (mode == Commands::TransferModePort)
        {
            QString dataId = this->api->network().connect(QHostAddress(address), port, QStringList("FTP-DATA"), LightBird::INetwork::TCP)->getResult();
            session->setClient(dataId);
            if (!dataId.isEmpty())
                session->setInformation("data-id", dataId);
            else
                result = Commands::Result(425, "Could not open data connection.");
        }
        else // TODO: implement Passive mode
        {
        }
    }
    return (result);
}

bool ClientHandler::onDataConnect(LightBird::IClient &client)
{
    LightBird::Session session = client.getSession();
    QString command;

    if (session == NULL)
        return (false);
    command = session->getInformation("pending-transfer-command").toString();
    if (this->commands->isSender(command))
        this->api->network().send(client.getId(), "FTP-DATA");
    else // TODO: implement receiving data
        this->api->network().receive(client.getId(), "FTP-DATA");
    this->_sendControlMessage(session->getInformation("control-id").toString(), Commands::Result(150, "Accepted data connection."));
    return (true);
}

bool ClientHandler::doDataExecute(LightBird::IClient &client)
{
    LightBird::Session session = client.getSession();
    Commands::Result   controlOut;

    if (session == NULL)
        return (false);
    QString command = session->getInformation("pending-transfer-command").toString();
    QString parameter = session->getInformation("pending-transfer-parameter").toString();
    QString control = session->getInformation("control-id").toString();
    if (!this->commands->isTransfert(command))
        return (false);
    controlOut = this->commands->executeTransfert(command, parameter, session, client);
    this->_sendControlMessage(control, controlOut);
    return (true);
}
