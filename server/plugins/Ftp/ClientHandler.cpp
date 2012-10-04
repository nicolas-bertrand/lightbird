#include "ClientHandler.h"
#include "Plugin.h"
#include "SmartMutex.h"

#include "IClient.h"

ClientHandler::ClientHandler(LightBird::IApi &api) : api(api)
{
    this->commands = new Commands(api);
}

ClientHandler::~ClientHandler()
{
    delete commands;
}

bool    ClientHandler::onConnect(LightBird::IClient &client)
{
    QVariantMap informations;

    // Initializes a new session for this control connection. Only the informations
    // used by both the control and data connections are stored here.
    LightBird::Session session = this->api.sessions().create();
    session->setClient(client.getId());
    informations["control-id"] = client.getId(); // The id of the control connection. Defined while it is opened.
//  informations["data-id"] = QString();         // The id of the data connection. Defined while it is opened.
    informations["working-dir"] = QString();     // The id of the working directory. Empty for the root.
    informations["last-command"] = QString();    // The last command executed by the control connection.
    informations["disconnect-data"] = false;     // Allows to abort the data connection if true.
    informations["binary-flag"] = false;         // Whether we are in Ascii or Image mode.
//  informations["restart"] = 0;                 // The RESTart position of the next RETR or STOR command.
    informations["transfer-ip"] = QString();     // In active mode these two variables contains the information gived by the PORT command,
    informations["transfer-port"] = 0;           // however in passive mode they contains the control client informations.
//  informations["transfer-command"] = "";       // The command that initiated the transfer. Defined only during the transfer.
//  informations["transfer-parameter"] = "";     // The paramater of the command. Defined only during the transfer.
    informations["transfer-mode"] = (int)Commands::NONE; // The selected transfer mode of the data.
    session->setInformations(informations);
    // Initializes the control connection specific informations
    informations.clear();
//  informations["user"] = QString();            // The name of the account gived by the USER command.
//  informations["oldName"] = QString();         // The name of the file to rename in RNTO.
    client.getInformations() = informations;
    // Here is the list of the data connection specific informations
//  informations["download"] = true;             // Defined when a download is in progress.
//  informations["upload"] = true;               // Defined when an upload is in progress.
//  informations["message"] = QString();         // Defined if a message have to be sent through the control connection after the transfert.
//  informations["code"] = 0;                    // Sent along with the message when a transfert is completed.
//  informations["upload-id"] = QString();       // The id of the uploaded file. Used to identify it in the timer thread.
//  informations["download-completed"] = true;   // Defined when the download has been completed. An error message is sent otherwise.
    // Be polite and welcome the user
    Commands::Result greeting(220, "Welcome to Lightbird's FTP server.\r\n"
                                   "Please authenticate.\r\n"
                                   "And of course, have fun!\r\n");
    Plugin::sendControlMessage(client.getId(), greeting);
    return (true);
}

bool    ClientHandler::doControlExecute(LightBird::IClient &client)
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
        // Make sure we capitalize it, as we do case insensitive matching
        QString command = request.getMethod().toUpper();
        QString parameter = request.getInformations()["parameter"].toString();
        if (this->commands->isControl(command))
            result = this->commands->executeControl(command, parameter, session, client);
        else if (this->commands->isTransfer(command))
            result = this->_prepareTransferMethod(command, parameter, session, client);
        else
            result = Commands::Result(500, "Unknow command.");
    }
    response.setCode(result.first);
    response.setMessage(result.second);
    return (true);
}

Commands::Result ClientHandler::_prepareTransferMethod(const QString &command, const QString &parameter, LightBird::Session &session, LightBird::IClient &client)
{
    Commands::TransferMode mode = (Commands::TransferMode)session->getInformation("transfer-mode").toInt();
    QString                address = session->getInformation("transfer-ip").toString();
    int                    port = session->getInformation("transfer-port").toInt();
    Commands::Result       result;
    QString                dataId;

    if (session->getAccount().isEmpty())
        result = Commands::Result(530, "Please login with USER and PASS.");
    else if (mode == Commands::NONE)
        result = Commands::Result(425, "No Data Connection.");
    else
    {
        session->setInformation("transfer-command", command);
        session->setInformation("transfer-parameter", parameter);
        if (mode == Commands::PASSIVE)
        {
            this->mutex.lock();
            QMutableListIterator<QPair<QHostAddress, QString> > it(this->passiveClients);
            while (it.hasNext())
                // A client with the same IP is already connected, so we initiate the transfer
                if (it.next().first == client.getPeerAddress())
                {
                    dataId = it.peekPrevious().second;
                    session->setClient(dataId);
                    session->setInformation("data-id", dataId);
                    QString command = session->getInformation("transfer-command").toString();
                    // Starts the transfer
                    if (this->commands->isSender(command))
                        this->api.network().send(dataId);
                    else
                        this->api.network().receive(dataId);
                    // Wakes the data connection up if it was waiting for the control connection
                    if (this->wait.contains(dataId))
                        this->wait.value(dataId)->wakeAll();
                    it.remove();
                    break;
                }
            // Otherwise we wait for a valid client
            if (dataId.isEmpty())
                session->setInformation("data-id", QString());
            this->mutex.unlock();
        }
        else if (mode == Commands::ACTIVE)
        {
            // Connection to the client. The mutex guarantees the atomicity of assignment of the session.
            this->mutex.lock();
            dataId = this->api.network().connect(QHostAddress(address), port, QStringList(Plugin::getConfiguration().dataProtocolName))->getResult();
            session->setClient(dataId);
            if (!dataId.isEmpty())
                session->setInformation("data-id", dataId);
            else
                result = Commands::Result(425, "Could not open data connection.");
            this->mutex.unlock();
        }
    }
    return (result);
}

bool    ClientHandler::onDataConnect(LightBird::IClient &client)
{
    LightBird::IDatabase &database = this->api.database();
    LightBird::Session   session;
    QVector<QVariantMap> result;
    QSqlQuery            query;
    QString              command;

    // Passive mode
    if (client.getMode() == LightBird::IClient::SERVER)
    {
        SmartMutex mutex(this->mutex, "ClientHandler", "onDataConnect");
        if (!mutex)
            return (false);
        // The first control connection with the same ip as the client is used
        query.prepare(database.getQuery("Ftp", "select_session"));
        query.bindValue(":ip", client.getPeerAddress().toString());
        if (!database.query(query, result))
            return (false);
        // No control connection found. The client may have been connected before the control connection was prepared.
        if (result.isEmpty() || result.first().value("count").toUInt() != 2)
        {
            this->passiveClients.append(QPair<QHostAddress, QString>(client.getPeerAddress(), client.getId()));
            return (true);
        }
        // The control connection has been found
        if (!(session = this->api.sessions().getSession(result.first().value("id").toString())))
            return (false);
        session->setClient(client.getId());
        session->setInformation("data-id", client.getId());
        mutex.unlock();
    }
    // Active mode. The mutex ensures that we have the time to assign the session to the client, in _prepareTransferMethod
    else if (client.getMode() == LightBird::IClient::CLIENT)
    {
        this->mutex.lock();
        session = client.getSession();
        this->mutex.unlock();
    }
    if (session == NULL)
        return (false);
    command = session->getInformation("transfer-command").toString();
    // Starts the transfer
    if (this->commands->isSender(command))
        this->api.network().send(client.getId());
    else
        this->api.network().receive(client.getId());
    return (true);
}

bool    ClientHandler::doDataExecute(LightBird::IClient &client)
{
    LightBird::Session session = client.getSession();
    Commands::Result   controlOut;

    if (session == NULL)
    {
        SmartMutex mutex(this->mutex, "ClientHandler", "doDataExecute");
        // If the control connection is still not ready, we have to wait
        if (mutex && this->passiveClients.contains(QPair<QHostAddress, QString>(client.getPeerAddress(), client.getId()))
            && client.getMode() == LightBird::IClient::SERVER)
        {
            QWaitCondition wait;
            this->wait.insert(client.getId(), &wait);
            wait.wait(&this->mutex, Plugin::getConfiguration().timeWaitControl);
            this->wait.remove(client.getId());
        }
        if (!(session = client.getSession()))
            return (false);
    }
    QString command = session->getInformation("transfer-command").toString();
    QString parameter = session->getInformation("transfer-parameter").toString();
    QString control = session->getInformation("control-id").toString();
    if (!this->commands->isTransfer(command))
        return (false);
    if (this->commands->isSender(command))
        client.getInformations().insert("download", true);
    else
        client.getInformations().insert("upload", true);
    controlOut = this->commands->executeTransfer(command, parameter, session, client);
    Plugin::sendControlMessage(control, controlOut);
    session->removeInformation("transfer-command");
    session->removeInformation("transfer-parameter");
    return (true);
}

void    ClientHandler::onDataDestroy(LightBird::IClient &client)
{
    SmartMutex  mutex(this->mutex, "ClientHandler", "onDataDestroyed");

    if (!mutex)
        return ;
    QMutableListIterator<QPair<QHostAddress, QString> > it(this->passiveClients);
    while (it.hasNext())
        if (it.next().second == client.getId())
            it.remove();
}
