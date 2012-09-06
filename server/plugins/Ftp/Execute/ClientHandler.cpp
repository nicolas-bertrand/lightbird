#include "ClientHandler.h"

#include "IClient.h"
#include "IApi.h"

#include <QString>

ClientHandler::ClientHandler(LightBird::IApi *api) : api(api)
{
    execute = new Execute(api);

    // Initialize the verb->function mappings
    controlMethods["USER"] = &Execute::doUser;
    controlMethods["PASS"] = &Execute::doPass;
    controlMethods["SYST"] = &Execute::doSyst;
    controlMethods["PWD"] =  &Execute::doPwd;
    controlMethods["TYPE"] = &Execute::doType;
    controlMethods["PORT"] = &Execute::doPort;
    controlMethods["CWD"] = &Execute::doCwd;
    controlMethods["CDUP"] = &Execute::doCdup;

    transferMethods["LIST"] = qMakePair(true, &Execute::doList);
    transferMethods["RETR"] = qMakePair(true, &Execute::doRetr);
    transferMethods["STOR"] = qMakePair(false, &Execute::doStor);
}

ClientHandler::~ClientHandler()
{
    delete execute;
}

bool ClientHandler::onConnect(LightBird::IClient *client)
{
    // Initialize a new session for this control connection
    LightBird::Session sess = this->api->sessions().create();
    sess->setClient(client->getId());
    sess->setInformation("control-id", client->getId());
    sess->setInformation("data-id", QString());
    
    sess->setInformation("working-dir", ""); // Id of the working directory. Empty if at root
    sess->setInformation("binary-flag", false);

    sess->setInformation("transfer-mode", (int)Execute::TransferModeNone);
    sess->setInformation("transfer-ip", "");
    sess->setInformation("transfer-port", 0);

    // Be polite and welcome the user
    this->_sendControlMessage(client->getId(), execute->doGreeting(QString(), sess));

    return true;
}

bool ClientHandler::doControlExecute(LightBird::IClient *client)
{
    Execute::MethodResult ret(0,"");
    LightBird::IRequest &request = client->getRequest();
    LightBird::IResponse &response = client->getResponse();
    LightBird::Session session = client->getSession();

    QVariantMap &info = request.getInformations();

    if (info.contains("send-message") && info.value("send-message").toBool())
    {
        ret.first = info.value("code").toInt();
        ret.second = info.value("message").toString();
    }
    else
    {
        if (request.getContent().getStorage() != LightBird::IContent::VARIANT)
        {
            request.getContent().setStorage(LightBird::IContent::VARIANT);
            *request.getContent().getVariant() = QString();

        }

        QString verb = request.getMethod().toUpper(); // Make sure we capitalize it, as we do case insensitive matching
        QString param = request.getContent().getVariant()->toString();

        if (this->controlMethods.contains(verb))
        {
            ret = (execute->*(this->controlMethods.value(verb)))(param, session);
        }
        else if (this->transferMethods.contains(verb))
        {
            ret = this->_prepareTransferMethod(verb, param, session);
        }
        else
        {
            ret.first = 500;
            ret.second = "Unknown command";
        }
    }

    response.setCode(ret.first);
    response.setMessage(ret.second);

    return true;
}

void ClientHandler::_sendControlMessage(QString id, Execute::MethodResult message)
{
    QVariantMap informations;
    informations.insert("send-message", true); // The presence of the information indicates that we need to send a message and not process a request
    informations.insert("code", message.first); // The code of the message
    informations.insert("message", message.second); // And it's description
    this->api->network().send(id, "FTP", informations);
}

Execute::MethodResult ClientHandler::_prepareTransferMethod(QString verb, QString parameter, LightBird::Session session) 
{
    Execute::TransferMode mode = (Execute::TransferMode)session->getInformation("transfer-mode").toInt();
    Execute::MethodResult ret;
    QString address = session->getInformation("transfer-ip").toString();
    int port = session->getInformation("transfer-port").toInt();
    if (mode == Execute::TransferModeNone)
        ret = Execute::MethodResult(425, "No Data Connection");
    else
    {
        session->setInformation("pending-transfer-verb", verb);
        session->setInformation("pending-transfer-param", parameter);

        if (mode == Execute::TransferModePort)
        {
            QString dataId = api->network().connect(QHostAddress(address), port, QStringList("FTP-DATA"), LightBird::INetwork::TCP)->getResult();
            session->setClient(dataId);
            if (dataId.isEmpty())
            {
                ret = Execute::MethodResult(425, "Could not open data connection");
            }
            else {
                // The connection was successful
//                session->setClient(dataId);
                session->setInformation("data-id", dataId);
   //             ret = Execute::MethodResult(150, "Accepted data connection");
            }

        }
        else // TODO: implement Passive mode
        {
        }

    }
    return ret;
}

bool ClientHandler::onDataConnect(LightBird::IClient *client)
{
    LightBird::Session session = client->getSession();

    if (session == NULL)
        return false;
    
    QString verb = session->getInformation("pending-transfer-verb").toString();
    QPair<bool, Execute::TransferMethod> method = transferMethods.value(verb);
    
    
    if (method.first)
    {
        api->network().send(client->getId(), "FTP-DATA");
    }
    else // TODO: implement receiving data
    {
        api->network().receive(client->getId(), "FTP-DATA");
    }

    this->_sendControlMessage(session->getInformation("control-id").toString(), Execute::MethodResult(150, "Accepted data connection\r\n"));
    return true;
}

bool ClientHandler::doDataExecute(LightBird::IClient *client)
{
    LightBird::Session session = client->getSession();

    if (session == NULL)
        return false;

    QString verb = session->getInformation("pending-transfer-verb").toString();
    QString param = session->getInformation("pending-transfer-param").toString();
    QString control = session->getInformation("control-id").toString();

    Execute::MethodResult controlOut;

    if (!transferMethods.contains(verb))
        return false;
    QPair<bool, Execute::TransferMethod> method = transferMethods.value(verb);

    controlOut = (execute->*(method.second))(param, session, &client->getRequest(), &client->getResponse());

    this->_sendControlMessage(control, controlOut);

    return true;
}

