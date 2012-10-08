#ifndef COMMANDS_H
# define COMMANDS_H

# include <QPair>

# include "IApi.h"
# include "ISessions.h"
# include "IClient.h"

#include "TableFiles.h"

/// @brief Executes the commands of the clients.
class Commands
{
public:
    Commands(LightBird::IApi &api);
    ~Commands();

    enum TransferMode
    {
        NONE,    ///< Mode not set yet.
        ACTIVE,  ///< Active mode. Defined by PORT.
        PASSIVE  ///< Passive mode. Defined by PASV.
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
    typedef Result (Commands::*ControlMethod)(const QString &parameter, LightBird::Session &session, LightBird::IClient &client);
    /// A transfer method is similar to a control method.
    /// However it either uses a IRequest or fills in a IResponse with the contents of the data connection,
    /// depending if it is a in or out transfer.
    typedef Result (Commands::*TransferMethod)(const QString &parameter, LightBird::Session &session, LightBird::IClient &client);

    /// @brief Returns true if the parameter is a control command.
    bool    isControl(const QString &command);
    /// @brief Returns true if the parameter is a transfer command.
    bool    isTransfer(const QString &command);
    /// @brief Returns true if the transfer command is going to send data on
    /// the transfer connection. Otherwise it will receive data from the client.
    bool    isSender(const QString &command);
    /// @brief Executes a control command, and return its response.
    Result  executeControl(const QString &command, const QString parameter, LightBird::Session &session, LightBird::IClient &client);
    /// @brief Executes a control command, and return its response.
    Result  executeTransfer(const QString &command, const QString &parameter, LightBird::Session &session, LightBird::IClient &client);

private:
    // Control methods
    Result  _user(const QString &parameter, LightBird::Session &session, LightBird::IClient &client);
    Result  _pass(const QString &parameter, LightBird::Session &session, LightBird::IClient &client);
    Result  _acct(const QString &parameter, LightBird::Session &session, LightBird::IClient &client);
    Result  _help(const QString &parameter, LightBird::Session &session, LightBird::IClient &client);
    Result  _pwd (const QString &parameter, LightBird::Session &session, LightBird::IClient &client);
    Result  _cwd (const QString &parameter, LightBird::Session &session, LightBird::IClient &client);
    Result  _cdup(const QString &parameter, LightBird::Session &session, LightBird::IClient &client);
    Result  _mkd (const QString &parameter, LightBird::Session &session, LightBird::IClient &client);
    Result  _rmd (const QString &parameter, LightBird::Session &session, LightBird::IClient &client);
    Result  _rnfr(const QString &parameter, LightBird::Session &session, LightBird::IClient &client);
    Result  _rnto(const QString &parameter, LightBird::Session &session, LightBird::IClient &client);
    Result  _dele(const QString &parameter, LightBird::Session &session, LightBird::IClient &client);
    Result  _size(const QString &parameter, LightBird::Session &session, LightBird::IClient &client);
    Result  _mdtm(const QString &parameter, LightBird::Session &session, LightBird::IClient &client);
    Result  _syst(const QString &parameter, LightBird::Session &session, LightBird::IClient &client);
    Result  _stat(const QString &parameter, LightBird::Session &session, LightBird::IClient &client);
    Result  _feat(const QString &parameter, LightBird::Session &session, LightBird::IClient &client);
    Result  _opts(const QString &parameter, LightBird::Session &session, LightBird::IClient &client);
    Result  _type(const QString &parameter, LightBird::Session &session, LightBird::IClient &client);
    Result  _stru(const QString &parameter, LightBird::Session &session, LightBird::IClient &client);
    Result  _mode(const QString &parameter, LightBird::Session &session, LightBird::IClient &client);
    Result  _rest(const QString &parameter, LightBird::Session &session, LightBird::IClient &client);
    Result  _allo(const QString &parameter, LightBird::Session &session, LightBird::IClient &client);
    Result  _pasv(const QString &parameter, LightBird::Session &session, LightBird::IClient &client);
    Result  _epsv(const QString &parameter, LightBird::Session &session, LightBird::IClient &client);
    Result  _port(const QString &parameter, LightBird::Session &session, LightBird::IClient &client);
    Result  _eprt(const QString &parameter, LightBird::Session &session, LightBird::IClient &client);
    Result  _noop(const QString &parameter, LightBird::Session &session, LightBird::IClient &client);
    Result  _abor(const QString &parameter, LightBird::Session &session, LightBird::IClient &client);
    Result  _quit(const QString &parameter, LightBird::Session &session, LightBird::IClient &client);
    // Transfer methods
    Result  _list(const QString &parameter, LightBird::Session &session, LightBird::IClient &client);
    Result  _nlst(const QString &parameter, LightBird::Session &session, LightBird::IClient &client);
    Result  _retr(const QString &parameter, LightBird::Session &session, LightBird::IClient &client);
    Result  _stor(const QString &parameter, LightBird::Session &session, LightBird::IClient &client);
    Result  _stou(const QString &parameter, LightBird::Session &session, LightBird::IClient &client);
    Result  _appe(const QString &parameter, LightBird::Session &session, LightBird::IClient &client);

    /// @brief Returns the file pointed by path.
    /// @param session : Used to get the working directory if the path is relative.
    LightBird::TableFiles _getFile(const QString &path, LightBird::Session &session);
    /// @brief Returns the directory pointed by path.
    LightBird::TableDirectories _getDirectory(const QString &path, LightBird::Session &session);
    /// @brief Double-quotes in the path are escaped by double-quotes.
    QString _escapePath(const QString &path);
    /// @brief Returns the list of the objects in a directory, with the "ls -l" format.
    QStringList _getList(const QString &path, LightBird::Session &session);
    /// @brief Converts the date in the "ls -l" format.
    QString _listDate(const QDateTime &datetime);

    LightBird::IApi   &api;
    QMap<QString, Commands::ControlMethod> controlCommands; ///< The list of the control commands.
    QMap<QString, QPair<bool, Commands::TransferMethod> > transferCommands; ///< The bool of the pair is true if the command sends data on transfer connection, false if it receives some.
    QStringList       anonymousCommands; ///< The list of the commands that can be used without being identified.
    static const char *months[]; ///< Ensures that the months are not localized when using QDate::toString.
};

#endif // COMMANDS_H
