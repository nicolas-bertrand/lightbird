#include "Ftp.h"
#include "Control.h"
#include "Plugin.h"

Control::Control(LightBird::IApi *api)
    : api(api)
{
}

Control::~Control()
{
}

bool    Control::onConnect(LightBird::IClient &client)
{
    QVariantMap informations;

    // Initializes a new session for this control connection.
    LightBird::Session session = this->api->sessions().create();
    session->setClient(client.getId());
    informations[SESSION_CONTROL_ID] = client.getId();
    informations[SESSION_WORKING_DIR] = QString();
    informations[SESSION_LAST_COMMAND] = QString();
    informations[SESSION_BINARY_FLAG] = false;
    informations[SESSION_TRANSFER_IP] = QString();
    informations[SESSION_TRANSFER_PORT] = 0;
    informations[SESSION_TRANSFER_MODE] = (int)Commands::NONE;
    session->setInformations(informations);
    // Be polite and welcome the user
    Commands::Result greeting(220, "Welcome to Lightbird's FTP server.\r\n"
                                   "Please authenticate.\r\n"
                                   "And of course, have fun!\r\n");
    Plugin::sendControlMessage(client.getId(), greeting);
    return (true);
}

bool    Control::doDeserializeContent(LightBird::IClient &client, const QByteArray &data, quint64 &used)
{
    Plugin::getTimer().stopTimeout(client.getId());
    return (Plugin::getParser(client).doDeserializeContent(data, used));
}

bool    Control::doExecution(LightBird::IClient &client)
{
    Commands::Result     result(0, "");
    LightBird::IRequest  &request = client.getRequest();
    LightBird::IResponse &response = client.getResponse();
    LightBird::Session   session = client.getSession();
    QVariantMap          &informations = request.getInformations();

    if (informations.contains(CONTROL_SEND_MESSAGE) && informations.value(CONTROL_SEND_MESSAGE).toBool())
    {
        result.first = informations.value(CONTROL_CODE).toInt();
        result.second = informations.value(CONTROL_MESSAGE).toString();
    }
    else
    {
        QString command = request.getMethod();
        QString parameter = informations["parameter"].toString();
        if (Plugin::getCommands().isControl(command))
            result = Plugin::getCommands().executeControl(command, parameter, session, client);
        else if (Plugin::getCommands().isTransfer(command))
            result = this->_prepareTransferCommand(command, parameter, session, client);
        else
            result = Commands::Result(500, "Unknow command.");
    }
    response.setCode(result.first);
    response.setMessage(result.second);
    return (true);
}

void    Control::onResume(LightBird::IClient &client, bool timeout)
{
    LightBird::Session session;

    if (timeout)
    {
        // Abort the transfert
        if ((session = client.getSession()))
        {
            session->removeInformation(SESSION_TRANSFER_COMMAND);
            session->removeInformation(SESSION_TRANSFER_PARAMETER);
        }
        Plugin::sendControlMessage(client.getId(), Commands::Result(426, "Transfert aborted. Passive data connection timeout."));
    }
}

bool    Control::onExecution(LightBird::IClient &client)
{
    // Ensures that a response is needed
    return (!client.getResponse().getMessage().isEmpty());
}

bool    Control::doSerializeContent(LightBird::IClient &client, QByteArray &data)
{
    return (Plugin::getParser(client).doSerializeContent(data));
}

void    Control::onFinish(LightBird::IClient &client)
{
    Plugin::getTimer().startTimeout(client.getId());
}

bool    Control::onDisconnect(LightBird::IClient &, bool)
{
    // We want to finish the current request before disconnecting the client
    return (false);
}

void    Control::onDestroy(LightBird::IClient &client)
{
    LightBird::Session  session = client.getSession();

    // Destroys the session if there is no data connection
    if (session)
    {
        session->removeInformation(SESSION_CONTROL_ID);
        session->removeClient(client.getId());
        if (session->getInformation(SESSION_DATA_ID).toString().isEmpty())
            session->destroy();
    }
}

Commands::Result    Control::_prepareTransferCommand(const QString &command, const QString &parameter, LightBird::Session &session, LightBird::IClient &client)
{
    Commands::TransferMode mode = (Commands::TransferMode)session->getInformation(SESSION_TRANSFER_MODE).toInt();
    QString                address = session->getInformation(SESSION_TRANSFER_IP).toString();
    int                    port = session->getInformation(SESSION_TRANSFER_PORT).toInt();
    Commands::Result       result;
    QString                dataId;

    if (session->getAccount().isEmpty())
        result = Commands::Result(530, "Please login with USER and PASS.");
    else if (mode == Commands::NONE)
        result = Commands::Result(425, "No Data Connection.");
    else
    {
        session->setInformation(SESSION_TRANSFER_COMMAND, command);
        session->setInformation(SESSION_TRANSFER_PARAMETER, parameter);
        if (mode == Commands::PASSIVE)
        {
            QSharedPointer<Mutex> mutex(Plugin::getDataConnection(session, client, dataId));
            // A data connection has been found
            if (!dataId.isEmpty())
            {
                session->setClient(dataId);
                session->setInformation(SESSION_DATA_ID, dataId);
                // Starts the transfer
                if (Plugin::getCommands().isSender(command))
                    this->api->network().send(dataId);
                else
                    this->api->network().receive(dataId);
                // Resumes the data client which was waiting for the control connection
                this->api->network().resume(dataId);
            }
            // Overwise we pause the control client, waiting for the data connection
            else
                this->api->network().pause(client.getId(), Plugin::getConfiguration().waitConnectionTime);
        }
        // Direct connection to the client
        else if (mode == Commands::ACTIVE)
        {
            // The mutex guarantees the atomicity of assignment of the session
            QSharedPointer<Mutex> mutex(Plugin::getMutex("Control", "_prepareTransferCommand"));
            dataId = this->api->network().connect(QHostAddress(address), port, QStringList(FTP_PROTOCOL_NAME))->getResult();
            session->setClient(dataId);
            if (!dataId.isEmpty())
                session->setInformation(SESSION_DATA_ID, dataId);
            else
                result = Commands::Result(425, "Could not open data connection.");
        }
    }
    return (result);
}
