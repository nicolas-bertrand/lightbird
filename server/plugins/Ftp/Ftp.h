#ifndef FTP_H
# define FTP_H

# define FTP_PROTOCOL_NAME  "FTP"     // The name of the FTP protocol.
# define CONTROL_CONNECTION "control" // The name of the control connection context.
# define DATA_CONNECTION    "data"    // The name of the data connection context.
# define PARSER             "parser"  // The name of the client information that contains the parser.

// The informations stored in the session. Only the informations used by both
// the control and data connections are stored here.
# define SESSION_CONTROL_ID         "control-id"         // The id of the control connection. Defined while it is opened.
# define SESSION_DATA_ID            "data-id"            // The id of the data connection. Defined while it is opened.
# define SESSION_WORKING_DIR        "working-dir"        // The id of the working directory. Empty for the root.
# define SESSION_LAST_COMMAND       "last-command"       // The last command executed by the control connection.
# define SESSION_BINARY_FLAG        "binary-flag"        // Whether we are in Ascii or Image mode.
# define SESSION_RESTART            "restart"            // The RESTart position of the next RETR or STOR command.
# define SESSION_TRANSFER_IP        "transfer-ip"        // In active mode these two variables contains the information given by the PORT command,
# define SESSION_TRANSFER_PORT      "transfer-port"      // however in passive mode the port is the server passive port.
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
# define DATA_SESSION_ID            "data-session-id"    // The id of the session for which the data connection has been created (in active mode).
# define DATA_DOWNLOAD              "download"           // Defined when a download is in progress.
# define DATA_UPLOAD                "upload"             // Defined when an upload is in progress.
# define DATA_CODE                  "code"               // Sent along with the message when a transfer is completed.
# define DATA_MESSAGE               "message"            // Defined if a message have to be sent through the control connection after the transfer.
# define DATA_UPLOAD_ID             "upload-id"          // The id of the uploaded file. Used to identify it.
# define DATA_DOWNLOAD_COMPLETED    "download-completed" // Defined when the download has been completed. An error message is sent otherwise.

#endif // FTP_H
