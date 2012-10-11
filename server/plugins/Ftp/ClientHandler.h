#ifndef CLIENTHANDLER_H
# define CLIENTHANDLER_H

# include <QHash>
# include <QHostAddress>
# include <QMutex>
# include <QWaitCondition>

# include "ISessions.h"

# include "Commands.h"

// The informations stored in the session. Only the informations used by both.
// the control and data connections are stored here.
# define SESSION_CONTROL_ID         "control-id"         // The id of the control connection. Defined while it is opened.
# define SESSION_DATA_ID            "data-id"            // The id of the data connection. Defined while it is opened.
# define SESSION_WORKING_DIR        "working-dir"        // The id of the working directory. Empty for the root.
# define SESSION_LAST_COMMAND       "last-command"       // The last command executed by the control connection.
# define SESSION_DISCONNECT_DATA    "disconnect-data"    // Allows to abort the data connection if true.
# define SESSION_BINARY_FLAG        "binary-flag"        // Whether we are in Ascii or Image mode.
# define SESSION_RESTART            "restart"            // The RESTart position of the next RETR or STOR command.
# define SESSION_TRANSFER_IP        "transfer-ip"        // In active mode these two variables contains the information gived by the PORT command,
# define SESSION_TRANSFER_PORT      "transfer-port"      // however in passive mode they contains the control client informations.
# define SESSION_TRANSFER_COMMAND   "transfer-command"   // The command that initiated the transfer. Defined only during the transfer.
# define SESSION_TRANSFER_PARAMETER "transfer-parameter" // The paramater of the command. Defined only during the transfer.
# define SESSION_TRANSFER_MODE      "transfer-mode"      // The selected transfer mode of the data.
// The informations stored in the control client
# define CONTROL_USER               "user"               // The name of the account gived by the USER command.
# define CONTROL_OLDNAME            "oldName"            // The name of the file to rename in RNTO.
# define CONTROL_SEND_MESSAGE       "send-message"       // Allows send a message through the control connection if true.
# define CONTROL_CODE               "code"               // The code of the message to send when SEND_MESSAGE is defined.
# define CONTROL_MESSAGE            "message"            // The text of the message to send when SEND_MESSAGE is defined.
// The informations stored in the data client
# define DATA_DOWNLOAD              "download"           // Defined when a download is in progress.
# define DATA_UPLOAD                "upload"             // Defined when an upload is in progress.
# define DATA_CODE                  "code"               // Sent along with the message when a transfer is completed.
# define DATA_MESSAGE               "message"            // Defined if a message have to be sent through the control connection after the transfer.
# define DATA_UPLOAD_ID             "upload-id"          // The id of the uploaded file. Used to identify it in the timer thread.
# define DATA_DOWNLOAD_COMPLETED    "download-completed" // Defined when the download has been completed. An error message is sent otherwise.

/// @brief Manages the clients on the control and data connections.
class ClientHandler
{
public:
    ClientHandler(LightBird::IApi &api);
    ~ClientHandler();

    bool    onConnect(LightBird::IClient &client);
    bool    onDataConnect(LightBird::IClient &client);
    bool    doControlExecute(LightBird::IClient &client);
    bool    doDataExecute(LightBird::IClient &client);
    void    onDataDestroy(LightBird::IClient &client);

private:
    Commands::Result _prepareTransferMethod(const QString &command, const QString &parameter, LightBird::Session &session, LightBird::IClient &client);

    LightBird::IApi  &api;
    Commands         *commands;
    QMutex           mutex; ///< Makes the class thread safe.
    /// The list of the clients that are waiting to be associated with a control connection in passive mode.
    QList<QPair<QHostAddress, QString> > passiveClients;
    /// The list of the clients that are waiting the control connection to be ready before the start of the deserialization.
    QHash<QString, QWaitCondition *> wait;
};

#endif // CLIENTHANDLER_H
