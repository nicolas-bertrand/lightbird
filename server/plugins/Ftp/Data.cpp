#include "Ftp.h"
#include "Data.h"
#include "LightBird.h"
#include "Mutex.h"
#include "Plugin.h"

Data::Data(LightBird::IApi *api)
    : api(api)
{
}

Data::~Data()
{
}

bool    Data::onConnect(LightBird::IClient &client)
{
    LightBird::Session session;
    QString            command;
    QString            controlId;
    bool               isValid;

    // Passive mode
    if (client.getMode() == LightBird::IClient::SERVER)
    {
        QSharedPointer<Mutex> mutex(Plugin::getControlConnection(client, controlId, isValid));
        // No passive data connection was expected from this ip and port
        if (!isValid)
            return (false);
        // The control connection has been found
        else if (!controlId.isEmpty())
        {
            QStringList sessions = this->api->sessions().getSessions("", controlId);
            if (sessions.isEmpty() || !(session = this->api->sessions().getSession(sessions.first())))
                return (false);
            session->setClient(client.getId());
            session->setInformation(SESSION_DATA_ID, client.getId());
            this->api->network().resume(controlId);
        }
        // The control connection is not ready, so we pause the data client while waiting
        else
        {
            this->api->network().pause(client.getId(), Plugin::getConfiguration().waitConnectionTime);
            return (true);
        }
    }
    // Active mode
    else if (client.getMode() == LightBird::IClient::CLIENT)
    {
        if (!(session = this->api->sessions().getSession(client.getInformations().value(DATA_SESSION_ID).toString())))
            return (false);
        session->setClient(client.getId());
        session->setInformation(SESSION_DATA_ID, client.getId());
    }
    if (session == NULL)
        return (false);
    command = session->getInformation(SESSION_TRANSFER_COMMAND).toString();
    // Starts the transfer
    if (Plugin::getCommands().isSender(command))
        this->api->network().send(client.getId());
    else
        this->api->network().receive(client.getId());
    return (true);
}

void    Data::onResume(LightBird::IClient &client, bool timeout)
{
    if (timeout)
        this->api->network().disconnect(client.getId(), true);
}

bool    Data::doDeserializeHeader(LightBird::IClient &client, const QByteArray &, quint64 &)
{
    this->_execute(client);
    return (true);
}

bool    Data::doDeserializeContent(LightBird::IClient &client, const QByteArray &data, quint64 &used)
{
    Plugin::getTimer().stopTimeout(client.getId());
    return (Plugin::getParser(client).doDeserializeContent(data, used));
}

bool    Data::doExecution(LightBird::IClient &client)
{
    if (client.getMode() == LightBird::IClient::SERVER)
        return (this->_execute(client));
    return (false);
}

bool    Data::doSend(LightBird::IClient &client)
{
    return (this->_execute(client));
}

bool    Data::onSerialize(LightBird::IClient &, LightBird::IOnSerialize::Serialize)
{
    return (false);
}

bool    Data::doSerializeContent(LightBird::IClient &client, QByteArray &data)
{
    return (Plugin::getParser(client).doSerializeContent(data));
}

void    Data::onFinish(LightBird::IClient &client)
{
    this->api->network().disconnect(client.getId(), true);
}

bool    Data::onDisconnect(LightBird::IClient &client, bool)
{
    // If the client is downloading data we can close the connection directly.
    // Otherwise we have to wait that the upload is finished.
    return (client.getInformations().contains(DATA_DOWNLOAD));
}

void    Data::onDestroy(LightBird::IClient &client)
{
    LightBird::Session  session = client.getSession();
    QVariantMap         &informations = client.getInformations();

    // Handles the case in which an empty file is uploaded (doDeserializeHeader is never called)
    if (!client.getInformations().contains(DATA_DOWNLOAD) && !client.getInformations().contains(DATA_UPLOAD))
        this->_execute(client);
    if (session)
    {
        // If we just uploaded a file, we identify it
        if (informations.contains(DATA_UPLOAD) && informations.contains(DATA_UPLOAD_ID))
            LightBird::identify(informations.value(DATA_UPLOAD_ID).toString());
        // If the download was not completed before the disconnection, an error occurred
        if (informations.contains(DATA_DOWNLOAD) && informations.contains(DATA_MESSAGE) && !informations.contains(DATA_DOWNLOAD_COMPLETED))
            Plugin::sendControlMessage(session->getInformation(SESSION_CONTROL_ID).toString(), Commands::Result(426, "Transfer aborted."));
        // A message have to be sent after the transfer
        else if (informations.contains(DATA_MESSAGE))
            Plugin::sendControlMessage(session->getInformation(SESSION_CONTROL_ID).toString(), Commands::Result(informations.value(DATA_CODE).toUInt(), informations.value(DATA_MESSAGE).toString()));
        session->removeInformation(SESSION_DATA_ID);
        session->removeClient(client.getId());
        // Destroys the session if there is no control connection
        if (session->getInformation(SESSION_CONTROL_ID).toString().isEmpty())
            session->destroy();
    }
}

bool    Data::_execute(LightBird::IClient &client)
{
    LightBird::Session session;
    Commands::Result   controlOut;

    if (!(session = client.getSession()))
        return (false);
    QString command = session->getInformation(SESSION_TRANSFER_COMMAND).toString();
    QString parameter = session->getInformation(SESSION_TRANSFER_PARAMETER).toString();
    QString control = session->getInformation(SESSION_CONTROL_ID).toString();
    if (!Plugin::getCommands().isTransfer(command))
        return (false);
    if (Plugin::getCommands().isSender(command))
        client.getInformations().insert(DATA_DOWNLOAD, true);
    else
        client.getInformations().insert(DATA_UPLOAD, true);
    controlOut = Plugin::getCommands().executeTransfer(command, parameter, session, client);
    Plugin::sendControlMessage(control, controlOut);
    session->removeInformation(SESSION_TRANSFER_COMMAND);
    session->removeInformation(SESSION_TRANSFER_PARAMETER);
    return (true);
}
