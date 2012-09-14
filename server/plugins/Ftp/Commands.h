#ifndef COMMANDS_H
# define COMMANDS_H

# include <QPair>
# include <QString>

# include "IApi.h"
# include "ISessions.h"
# include "IClient.h"

class Commands
{
public:
    Commands(LightBird::IApi *api);
    ~Commands();

    enum TransferMode
    {
        TransferModeNone,
        TransferModePort,
        TransferModePasv
    };

    struct Upload
    {
        QTemporaryFile *file;
        QString name;
        QString parent;
    };

    /// The code and the message associated of a response on control connetcion.
    typedef QPair<int, QString> Result;
    /// A control method takes in a FTP command and the corresponding parmater string (possibly empty),
    /// as well as the session corresponding to the connection,
    /// and returns a pair with a code and a message to send on the control connection.
    typedef Result (Commands::*ControlMethod)(const QString &parameter, LightBird::Session &session);
    /// A transfer method is similar to a control method
    /// However if either uses a IRequest or fills in a IResponse with the contents of the data connection,
    /// depending if it is  a in or out transfer.
    typedef Result (Commands::*TransferMethod)(const QString &parameter, LightBird::Session &session, LightBird::IClient &client);

    /// @brief Returns true if the parameter is a control command.
    bool    isControl(const QString &command);
    /// @brief Returns true if the parameter is a transfert command.
    bool    isTransfert(const QString &command);
    /// @brief Returns true if the transfert command is going to send data on
    /// the transfert connection. Otherwise it will receive data from the client.
    bool    isSender(const QString &command);
    /// @brief Executes a control command, and return its response.
    Result  executeControl(const QString &command, const QString parameter, LightBird::Session &session);
    /// @brief Executes a control command, and return its response.
    Result  executeTransfert(const QString &command, const QString &parameter, LightBird::Session &session, LightBird::IClient &client);

private:
    // Control methods
    Result  _user(const QString &parameter, LightBird::Session &session);
    Result  _pass(const QString &parameter, LightBird::Session &session);
    Result  _acct(const QString &parameter, LightBird::Session &session);
    Result  _help(const QString &parameter, LightBird::Session &session);
    Result  _pwd (const QString &parameter, LightBird::Session &session);
    Result  _cwd (const QString &parameter, LightBird::Session &session);
    Result  _cdup(const QString &parameter, LightBird::Session &session);
    Result  _mkd (const QString &parameter, LightBird::Session &session);
    Result  _rmd (const QString &parameter, LightBird::Session &session);
    Result  _syst(const QString &parameter, LightBird::Session &session);
    Result  _type(const QString &parameter, LightBird::Session &session);
    Result  _stru(const QString &parameter, LightBird::Session &session);
    Result  _mode(const QString &parameter, LightBird::Session &session);
    Result  _port(const QString &parameter, LightBird::Session &session);
    Result  _noop(const QString &parameter, LightBird::Session &session);
    Result  _abor(const QString &parameter, LightBird::Session &session);
    Result  _quit(const QString &parameter, LightBird::Session &session);
    // Transfer methods
    Result  _list(const QString &parameter, LightBird::Session &session, LightBird::IClient &client);
    Result  _retr(const QString &parameter, LightBird::Session &session, LightBird::IClient &client);
    Result  _stor(const QString &parameter, LightBird::Session &session, LightBird::IClient &client);

    /// @brief Double-quotes in the path are escaped by double-quotes.
    QString _escapePath(const QString &path);

    LightBird::IApi *api;
    QStringList     anonymousCommands; ///< The list of the commands that can be used without being identified.
    QMap<QString, Commands::ControlMethod> controlCommands; ///< The list of the control commands.
    QMap<QString, QPair<bool, Commands::TransferMethod> > transferCommands; ///< The bool of the pair is true if the command sends data on transfer connection, false if it receives some.
};

#endif // COMMANDS_H
