#include <QDir>
#include <QSqlQuery>

#include "Dir.h"
#include "Commands.h"
#include "File.h"
#include "LightBird.h"
#include "Plugin.h"
#include "Properties.h"
#include "TableAccounts.h"
#include "TableDirectories.h"

const char *Commands::months[] = { "", "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };

Commands::Commands(LightBird::IApi &api)
    : api(api)
{
    this->controlCommands["USER"] = &Commands::_user;
    this->controlCommands["PASS"] = &Commands::_pass;
    this->controlCommands["ACCT"] = &Commands::_acct;
    this->controlCommands["HELP"] = &Commands::_help;
    this->controlCommands["PWD"] =  &Commands::_pwd;
    this->controlCommands["CWD"] = &Commands::_cwd;
    this->controlCommands["CDUP"] = &Commands::_cdup;
    this->controlCommands["MKD"] = &Commands::_mkd;
    this->controlCommands["RMD"] = &Commands::_rmd;
    this->controlCommands["RNFR"] = &Commands::_rnfr;
    this->controlCommands["RNTO"] = &Commands::_rnto;
    this->controlCommands["DELE"] = &Commands::_dele;
    this->controlCommands["SIZE"] = &Commands::_size;
    this->controlCommands["MDTM"] = &Commands::_mdtm;
    this->controlCommands["SYST"] = &Commands::_syst;
    this->controlCommands["STAT"] = &Commands::_stat;
    this->controlCommands["FEAT"] = &Commands::_feat;
    this->controlCommands["OPTS"] = &Commands::_opts;
    this->controlCommands["TYPE"] = &Commands::_type;
    this->controlCommands["STRU"] = &Commands::_stru;
    this->controlCommands["MODE"] = &Commands::_mode;
    this->controlCommands["REST"] = &Commands::_rest;
    this->controlCommands["ALLO"] = &Commands::_allo;
    this->controlCommands["PASV"] = &Commands::_pasv;
    this->controlCommands["EPSV"] = &Commands::_epsv;
    this->controlCommands["PORT"] = &Commands::_port;
    this->controlCommands["EPRT"] = &Commands::_eprt;
    this->controlCommands["NOOP"] = &Commands::_noop;
    this->controlCommands["ABOR"] = &Commands::_abor;
    this->controlCommands["QUIT"] = &Commands::_quit;
    this->transferCommands["LIST"] = qMakePair(true, &Commands::_list);
    this->transferCommands["NLST"] = qMakePair(true, &Commands::_nlst);
    this->transferCommands["RETR"] = qMakePair(true, &Commands::_retr);
    this->transferCommands["STOR"] = qMakePair(false, &Commands::_stor);
    this->transferCommands["STOU"] = qMakePair(false, &Commands::_stou);
    this->transferCommands["APPE"] = qMakePair(false, &Commands::_appe);
    this->anonymousCommands << "USER" << "PASS" << "HELP" << "ACCT" << "FEAT" << "OPTS";
}

Commands::~Commands()
{
}

bool    Commands::isControl(const QString &command)
{
    return (this->controlCommands.contains(command));
}

bool    Commands::isTransfer(const QString &command)
{
    return (this->transferCommands.contains(command));
}

bool    Commands::isSender(const QString &command)
{
    if (this->transferCommands.contains(command))
        return (this->transferCommands.value(command).first);
    return (true);
}

Commands::Result Commands::executeControl(const QString &command, const QString parameter, LightBird::Session &session, LightBird::IClient &client)
{
    Result  result;

    if (!session->getAccount().isEmpty() || this->anonymousCommands.contains(command))
        result = (this->*(this->controlCommands.value(command)))(parameter, session, client);
    else
        result = Result(530, "Please login with USER and PASS.");
    session->setInformation(SESSION_LAST_COMMAND, command);
    return (result);
}

Commands::Result Commands::executeTransfer(const QString &command, const QString &parameter, LightBird::Session &session, LightBird::IClient &client)
{
    Result result = (this->*(this->transferCommands.value(command).second))(parameter, session, client);
    session->setInformation(SESSION_LAST_COMMAND, command);
    return (result);
}

Commands::Result Commands::_user(const QString &user, LightBird::Session &session, LightBird::IClient &client)
{
    Result  result;

    if (!session->getAccount().isEmpty())
    {
        LightBird::TableAccounts account(session->getAccount());
        result = Result(530, QString("Already logged in as %1.").arg(account.getName()));
    }
    else if(!user.trimmed().isEmpty())
    {
        client.getInformations().insert(CONTROL_USER, user);
        result = Result(331, QString("User %1 OK. Password required.").arg(user));
    }
    else
        result = Result(530, "Anonymous login not alowed.");
    return (result);
}

Commands::Result Commands::_pass(const QString &pass, LightBird::Session &session, LightBird::IClient &client)
{
    LightBird::TableAccounts account;
    QString      user;
    Result       result;

    if (session->getInformation(SESSION_LAST_COMMAND) == "USER" && client.getInformations().contains(CONTROL_USER))
    {
        user = client.getInformations().value(CONTROL_USER).toString();
        if (account.setIdFromNameAndPassword(user, pass))
        {
            if (account.isActive())
            {
                result = Result(230, QString("Welcome %1.\r\nMake yourself at home!").arg(user));
                session->setAccount(account.getId());
            }
            else
                result = Result(530, "This account is disabled.");
        }
        else
            result = Result(530, "Login authentification failed.");
        client.getInformations().remove(CONTROL_USER);
    }
    else
        result = Result(503, "Bad sequence of commands.");
    return (result);
}

Commands::Result Commands::_acct(const QString &, LightBird::Session &, LightBird::IClient &)
{
    return (Result(202, "Command superfluous."));
}

Commands::Result Commands::_help(const QString &, LightBird::Session &, LightBird::IClient &)
{
    QStringList commands(this->controlCommands.keys() + this->transferCommands.keys());
    QString result;

    commands.sort();
    result = commands.join(" ");
    // The line is 80 columns maximum
    for (int i = 1; i <= result.size() / 75; ++i)
        result.replace(result.indexOf(' ', i * 75 - 5), 1, "\r\n");
    return (Result(214, "The following commands are recognized:\r\n" + result));
}

Commands::Result Commands::_pwd(const QString &, LightBird::Session &session, LightBird::IClient &)
{
    LightBird::TableDirectories directory(session->getInformation(SESSION_WORKING_DIR).toString());
    
    return (Result(257, QString("\"%1\" is your current location.").arg(this->_escapePath(directory.getVirtualPath(true)))));
}

Commands::Result Commands::_cwd(const QString &path, LightBird::Session &session, LightBird::IClient &)
{
    LightBird::TableDirectories directory(session->getInformation(SESSION_WORKING_DIR).toString());

    if (directory.cd(path))
    {
        session->setInformation(SESSION_WORKING_DIR, directory.getId());
        return (Result(250, QString("Directory changed to \"%1\".").arg(this->_escapePath(directory.getVirtualPath(true)))));
    }
    else
        return (Result(550, QString("Can't change directory to \"%1\".").arg(this->_escapePath(path))));
}

Commands::Result Commands::_cdup(const QString &, LightBird::Session &session, LightBird::IClient &client)
{
    return (this->_cwd("..", session, client));
}

Commands::Result Commands::_mkd(const QString &pathName, LightBird::Session &session, LightBird::IClient &)
{
    LightBird::TableDirectories   directory(session->getInformation(SESSION_WORKING_DIR).toString());

    // The path is absolute
    if (pathName.startsWith('/'))
        directory.clear();
    // Creates the directories in the path
    directory.setId(directory.createVirtualPath(pathName, session->getAccount()));
    return (Result(257, QString("\"%1\" directory created.").arg(this->_escapePath(directory.getVirtualPath(true)))));
}

Commands::Result Commands::_rmd(const QString &pathName, LightBird::Session &session, LightBird::IClient &)
{
    LightBird::TableDirectories   directory(session->getInformation(SESSION_WORKING_DIR).toString());

    if (!directory.cd(pathName))
        return (Result(550, QString("Directory not found \"%1\".").arg(this->_escapePath(pathName))));
    directory.remove(true);
    return (Result(250, QString("\"%1\" directory removed.").arg(this->_escapePath(pathName))));
}

Commands::Result Commands::_rnfr(const QString &oldName, LightBird::Session &session, LightBird::IClient &client)
{
    bool    isFile;

    if (!(isFile = this->_getFile(oldName, session)) && !this->_getDirectory(oldName, session))
        return (Result(550, QString("File or directory not found.")));
    client.getInformations().insert(CONTROL_OLDNAME, oldName);
    return (Result(350, QString("Waiting for the new name of the %1 \"%2\".").arg(isFile ? "file" : "directory", this->_escapePath(oldName))));
}

Commands::Result Commands::_rnto(const QString &path, LightBird::Session &session, LightBird::IClient &client)
{
    LightBird::TableFiles       file;
    LightBird::TableDirectories directory;
    LightBird::TableDirectories newDirectory;
    QString                     newName = path;
    QString                     oldName;
    bool                        isFile;
    Result                      error;

    if (session->getInformation(SESSION_LAST_COMMAND) != "RNFR" || !client.getInformations().contains(CONTROL_OLDNAME))
        return (Result(503, "Bad sequence of commands."));
    oldName = client.getInformations().value(CONTROL_OLDNAME).toString();
    client.getInformations().remove(CONTROL_OLDNAME);
    if (!(isFile = (file = this->_getFile(oldName, session))) && !(directory = this->_getDirectory(oldName, session)))
        return (Result(550, QString("File or directory not found \"%1\".").arg(this->_escapePath(oldName))));
    error = Result(553, QString("Unable to rename the %1.").arg(isFile ? "file" : "directory"));
    // The object is just renamed
    if (!newName.contains('/') && ((isFile && !file.setName(newName)) || (!isFile && !directory.setName(newName))))
        return (error);
    // The object is moved and renamed
    else if (newName.contains('/'))
    {
        newName = newName.right(newName.size() - newName.lastIndexOf('/') - 1);
        // Gets the new directory (or the root)
        if (!(newDirectory = this->_getDirectory(path.left(path.size() - newName.size()), session)) && path != "/" + newName)
            return (error);
        // Ensures that the new directory is not a child of the old directory
        if (!isFile && newDirectory.getParents().contains(directory.getId()))
            return (error);
        // Ensures that an object doesn't have the new name in the destination directory
        if (((isFile && !newDirectory.getFile(newName).isEmpty()) || (!isFile && !newDirectory.getDirectory(newName).isEmpty())))
            return (error);
        // Moves the object
        if (((isFile && !file.setIdDirectory(newDirectory.getId())) || (!isFile && !directory.setIdDirectory(newDirectory.getId()))))
            return (error);
        // And renames it
        if (((isFile && !file.setName(newName)) || (!isFile && !directory.setName(newName))))
            return (error);
    }
    return (Result(250, QString("%1 renamed from \"%2\" to \"%3\".").arg(isFile ? "File" : "Directory", this->_escapePath(oldName), this->_escapePath(path))));
}

Commands::Result Commands::_dele(const QString &path, LightBird::Session &session, LightBird::IClient &)
{
    LightBird::TableFiles   file;

    if (!(file = this->_getFile(path, session)))
        return (Result(550, QString("File not found.")));
    file.remove(true);
    return (Result(250, QString("\"%1\" file deleted.").arg(path)));
}

Commands::Result Commands::_size(const QString &path, LightBird::Session &session, LightBird::IClient &)
{
    LightBird::TableFiles   file;

    if (!(file = this->_getFile(path, session)))
        return (Result(550, QString("File not found.")));
    return (Result(213, QString::number(QFileInfo(file.getFullPath()).size())));
}

Commands::Result Commands::_mdtm(const QString &path, LightBird::Session &session, LightBird::IClient &)
{
    LightBird::TableFiles   file;

    if (!(file = this->_getFile(path, session)))
        return (Result(550, QString("File not found.")));
    return (Result(213, QFileInfo(file.getFullPath()).lastModified().toUTC().toString("yyyyMMddhhmmss")));
}

Commands::Result Commands::_syst(const QString &, LightBird::Session &, LightBird::IClient &)
{
    // As far as the clients are concerned, we are in a UNIX environment.
    return (Result(215, "UNIX Type: L8"));
}

Commands::Result Commands::_stat(const QString &path, LightBird::Session &session, LightBird::IClient &)
{
    bool         binary = session->getInformation(SESSION_BINARY_FLAG).toBool();
    QString      result;

    if (path.isEmpty())
    {
        result = "211-FTP server status:\r\n";
        result += " Logged in as " + LightBird::TableAccounts(session->getAccount()).getName() + "\r\n";
        result += " TYPE: " + QString(binary ? "BINARY" : "ASCII") + "\r\n";
        result += " LightBird " + this->api.getServerVersion() + "\r\n";
        result += "211 End of status\r\n";
    }
    else
    {
        result = "213-Status follows:\r\n";
        result += this->_getList(path, session).join("");
        result += "213 End of status.\r\n";
    }
    return (Result(0, result));
}

Commands::Result Commands::_feat(const QString &, LightBird::Session &, LightBird::IClient &)
{
    QStringList  feat;

    feat << "EPRT" << "EPSV" << "MDTM" << "PASV" << "REST STREAM" << "SIZE" << "UTF8";
    return (Result(0, "211-Features:\r\n " + feat.join("\r\n ") + "\r\n211 End\r\n"));
}

Commands::Result Commands::_opts(const QString &parameter, LightBird::Session &, LightBird::IClient &)
{
    if (parameter.trimmed() == "UTF8 ON")
        return (Result(200, "Always in UTF8 mode."));
    return (Result(501, "Option not understood."));
}

Commands::Result Commands::_type(const QString &parameter, LightBird::Session &session, LightBird::IClient &)
{
    bool    binary = session->getInformation(SESSION_BINARY_FLAG).toBool();
    QString type = parameter.toUpper();

    if (!type.isEmpty())
    {
        QString first = type.at(0);
        if (QString::compare(first, "A", Qt::CaseInsensitive) == 0)
        {
            if (type.size() > 2 && type.at(2) != 'N')
                return (Result(504, QString("Only N(on-print) format is supported.")));
            binary = false;
        }
        else if (QString::compare(first, "I", Qt::CaseInsensitive) == 0)
            binary = true;
        else if (QString::compare(first, "L", Qt::CaseInsensitive) == 0)
        {
            if (type.size() > 1 && type.at(1).isDigit())
            {
                if (type.at(1) != '8')
                    return (Result(504, "Only 8-bit bytes are supported."));
            }
            else
                return (Result(501, "Missing argument."));
            binary = true;
        }
        else
            return (Result(504, QString("Unknown Type : %1.").arg(type)));
    }
    else
        return (Result(501, "Missing argument.\r\nA(scii) I(mage) L(ocal)"));
    session->setInformation(SESSION_BINARY_FLAG, binary);
    return (Result(200, QString("TYPE is now %1.\r\n").arg(binary ? "8-bit binary" : "ASCII")));
}

Commands::Result Commands::_stru(const QString &structure, LightBird::Session &, LightBird::IClient &)
{
    if (QString(structure).toUpper() != "F")
        return (Result(504, "Bad STRU command."));
    return (Result(200, QString("Structure set to %1.").arg(structure)));
}

Commands::Result Commands::_mode(const QString &mode, LightBird::Session &, LightBird::IClient &)
{
    if (QString(mode).toUpper() != "S")
        return (Result(504, "Bad MODE command."));
    return (Result(200, QString("Mode set to %1.").arg(mode)));
}

Commands::Result Commands::_rest(const QString &position, LightBird::Session &session, LightBird::IClient &)
{
    if (!position.contains(QRegExp("^\\d+$")))
        return (Result(501, "Bad argument."));
    if (position.toULongLong())
        session->setInformation(SESSION_RESTART, position.toULongLong());
    else
        session->removeInformation(SESSION_RESTART);
    return (Result(350, QString("Restart position accepted (%1).").arg(position)));
}

Commands::Result Commands::_allo(const QString &, LightBird::Session &, LightBird::IClient &)
{
    return (Result(202, QString("ALLO command ignored.")));
}

Commands::Result Commands::_pasv(const QString &, LightBird::Session &session, LightBird::IClient &client)
{
    QString         protocol = Plugin::getConfiguration().dataProtocolName;
    QStringList     protocols;
    unsigned int    maxClients;
    unsigned short  port = Plugin::getConfiguration().passivePort;

    // Ensures that the passive port is opened
    if (this->api.network().getPort(port, protocols, maxClients) && protocols.contains(protocol))
    {
        session->setInformation(SESSION_TRANSFER_MODE, Commands::PASSIVE);
        session->setInformation(SESSION_TRANSFER_IP, client.getPeerAddress().toString());
        session->setInformation(SESSION_TRANSFER_PORT, client.getPeerPort());
        return (Result(227, QString("Entered Passive Mode (127,0,0,1,%1,%2)").arg(QString::number(port >> 8), QString::number(port & 0xFF))));
    }
    return (Result(502, "Data port not opened.\r\nUse active connection instead."));
}

Commands::Result Commands::_epsv(const QString &network, LightBird::Session &session, LightBird::IClient &client)
{
    QString         protocol = Plugin::getConfiguration().dataProtocolName;
    QStringList     protocols;
    unsigned int    maxClients;
    unsigned short  port = Plugin::getConfiguration().passivePort;

    // Ensures that the passive port is opened
    if (this->api.network().getPort(port, protocols, maxClients) && protocols.contains(protocol))
    {
        if (!network.isEmpty() && network != "1" && network != "2")
            return (Result(522, "Network protocol not supported, use (1,2)"));
        session->setInformation(SESSION_TRANSFER_MODE, Commands::PASSIVE);
        session->setInformation(SESSION_TRANSFER_IP, client.getPeerAddress().toString());
        session->setInformation(SESSION_TRANSFER_PORT, client.getPeerPort());
        return (Result(229, QString("Extended Passive Mode Entered (|||%1|)").arg(QString::number(port))));
    }
    return (Result(502, "Data port not opened.\r\nUse active connection instead."));
}

Commands::Result Commands::_port(const QString &hostPort, LightBird::Session &session, LightBird::IClient &)
{
    QRegExp reg("(\\d{1,3}),(\\d{1,3}),(\\d{1,3}),(\\d{1,3}),(\\d{1,3}),(\\d{1,3})"); // Regex matching the port parameter

    if (reg.indexIn(hostPort) < 0)
        return (Result(501, "Syntax error in IP address."));
    QString address = QString("%1.%2.%3.%4").arg(reg.cap(1)).arg(reg.cap(2)).arg(reg.cap(3)).arg(reg.cap(4));
    unsigned short port = reg.cap(5).toInt() << 8 | reg.cap(6).toInt();
    session->setInformation(SESSION_TRANSFER_MODE, Commands::ACTIVE);
    session->setInformation(SESSION_TRANSFER_IP, address);
    session->setInformation(SESSION_TRANSFER_PORT, port);
    return (Result(200, QString("Entered Active Mode (%1:%2)").arg(address, QString::number(port))));
}

Commands::Result Commands::_eprt(const QString &hostPort, LightBird::Session &session, LightBird::IClient &)
{
    QRegExp        reg("^\\|(\\d+)\\|(.+)\\|(\\d{1,5})\\|$"); // Regex matching the eprt parameter
    QHostAddress   host;

    if (reg.indexIn(hostPort) != 0)
        return (Result(501, "Syntax error in the parameter."));
    if (reg.cap(1) != "1" && reg.cap(1) != "2")
        return (Result(522, "Network protocol not supported, use (1,2)"));
    if (!host.setAddress(reg.cap(2)) || (reg.cap(1) == "1" && host.protocol() != QAbstractSocket::IPv4Protocol)
        || (reg.cap(1) == "2" && host.protocol() != QAbstractSocket::IPv6Protocol))
        return (Result(501, "Syntax error in the host address."));
    session->setInformation(SESSION_TRANSFER_MODE, Commands::ACTIVE);
    session->setInformation(SESSION_TRANSFER_IP, host.toString());
    session->setInformation(SESSION_TRANSFER_PORT, reg.cap(3).toUShort());
    return (Result(200, QString("Extended Active Mode Entered (%1|%2)").arg(host.toString(), reg.cap(3))));
}

Commands::Result  Commands::_noop(const QString &, LightBird::Session &, LightBird::IClient &)
{
    return (Result(200, ":)"));
}

Commands::Result Commands::_abor(const QString &, LightBird::Session &session, LightBird::IClient &)
{
    QString dataId = session->getInformation(SESSION_DATA_ID).toString();

    if (dataId.isEmpty())
        return (Result(225, "No transfer in progress."));
    this->api.network().disconnect(dataId);
    session->getClients();
    session->setInformation(SESSION_DISCONNECT_DATA, true);
    return (Result(0, "426 Data connection closed.\r\n226 Transfer aborted.\r\n"));
}

Commands::Result Commands::_quit(const QString &, LightBird::Session &session, LightBird::IClient &)
{
    session->destroy();
    this->api.network().disconnect(session->getInformation(SESSION_CONTROL_ID).toString());
    return (Result(221, "Goodbye."));
}

Commands::Result Commands::_list(const QString &path, LightBird::Session &session, LightBird::IClient &client)
{
    LightBird::IContent &content = client.getResponse().getContent();
    QString             controlId = session->getInformation(SESSION_CONTROL_ID).toString();
    QStringList         result;

    Plugin::sendControlMessage(controlId, Result(150, "Here comes the directory listing."));
    content.setData((result = this->_getList(path, session)).join("").toUtf8());
    client.getInformations().insert(DATA_CODE, 226);
    client.getInformations().insert(DATA_MESSAGE, QString("Directory sent.\r\n%1 objects\r\n").arg(QString::number(result.size())));
    return (Result());
}

QStringList Commands::_getList(const QString &path, LightBird::Session &session)
{
    LightBird::TableDirectories directory(session->getInformation(SESSION_WORKING_DIR).toString());
    QStringList                 files;
    QStringList                 directories;
    QString                     pattern("%1rwx------ %2 %3 nogroup %4 %5 %6\r\n");
    QString                     name;
    QString                     subDirectories;
    QString                     date;
    QString                     size;
    QStringList                 result;

    // Changes the directory to the argument if possible
    if (!path.isEmpty() && !directory.cd(path))
    {
        // The argument might by a path to a file
        if (path.contains('/'))
        {
            directory.cd(name = path.left(path.lastIndexOf('/') + 1));
            if (!(name = directory.getFile(path.right(path.size() - name.size()))).isEmpty())
                files.append(name);
        }
        // Otherwise it is a file name
        else if (!(name = directory.getFile(path)).isEmpty())
            files.append(name);
    }
    // List the content of the directory
    else
    {
        directories = directory.getDirectories();
        files = directory.getFiles();
        QStringListIterator d(directories);
        while(d.hasNext())
        {
            LightBird::TableDirectories directory(d.next());
            name = LightBird::TableAccounts(directory.getIdAccount()).getName();
            subDirectories = QString::number(directory.getDirectories().size() + 2);
            date = this->_listDate(directory.getModified().toLocalTime());
            result << pattern.arg("d", subDirectories, (name.isEmpty() ? "nouser" : name), QString::number(4096), date, directory.getName());
        }
    }
    // List the files
    QStringListIterator f(files);
    while(f.hasNext())
    {
        LightBird::TableFiles file(f.next());
        QFileInfo fileInfo(file.getFullPath());
        name = LightBird::TableAccounts(file.getIdAccount()).getName();
        size = QString::number(fileInfo.size());
        if (fileInfo.isFile())
            date = this->_listDate(fileInfo.lastModified());
        else
            date = this->_listDate(file.getModified());
        result << pattern.arg("-", "1", (name.isEmpty() ? "nouser" : name), size, date, file.getName());
    }
    return (result);
}

QString     Commands::_listDate(const QDateTime &datetime)
{
    QDate   date(datetime.date());

    if (date.year() == QDate::currentDate().year())
        return (Commands::months[date.month()] + datetime.toString(" dd hh:mm"));
    return (Commands::months[date.month()] + datetime.toString(" dd yyyy"));
}

Commands::Result Commands::_nlst(const QString &path, LightBird::Session &session, LightBird::IClient &client)
{
    QString                     controlId = session->getInformation(SESSION_CONTROL_ID).toString();
    LightBird::TableDirectories directory(session->getInformation(SESSION_WORKING_DIR).toString());
    QStringList                 list;
    QString                     name;
    QString                     result;

    Plugin::sendControlMessage(controlId, Result(150, "Here comes the directory listing."));
    // Changes the directory to the argument if possible
    if (!path.isEmpty() && !directory.cd(path))
    {
        // The argument might by a path to a file
        if (path.contains('/'))
        {
            directory.cd(name = path.left(path.lastIndexOf('/') + 1));
            result = name;
            if (!(name = directory.getFile(path.right(path.size() - name.size()))).isEmpty())
                list.append(LightBird::TableFiles(name).getName());
        }
        // Otherwise it is a file name
        else if (!(name = directory.getFile(path)).isEmpty())
            list.append(LightBird::TableFiles(name).getName());
    }
    // Fills the list with the content of the directory
    else
    {
        QStringListIterator directories(directory.getDirectories());
        while (directories.hasNext())
            list << LightBird::TableDirectories(directories.next()).getName();
        QStringListIterator files(directory.getFiles());
        while (files.hasNext())
            list << LightBird::TableFiles(files.next()).getName();
        result = path;
    }
    // Joins the elements of the list with the relative path
    if (result.endsWith('/'))
        result.chop(1);
    if (!result.isEmpty())
        result += "/" + list.join("\r\n" + result + "/").append("\r\n");
    // And without it
    else
        result = list.join("\r\n").append("\r\n");
    if (list.isEmpty())
        result.clear();
    client.getResponse().getContent().setData(result.toUtf8());
    client.getInformations().insert(DATA_CODE, 226);
    client.getInformations().insert(DATA_MESSAGE, QString("Directory sent.\r\n%1 objects\r\n").arg(QString::number(list.size())));
    return (Result());
}

Commands::Result Commands::_retr(const QString &path, LightBird::Session &session, LightBird::IClient &client)
{
    QString                     controlId = session->getInformation(SESSION_CONTROL_ID).toString();
    bool                        binary = session->getInformation(SESSION_BINARY_FLAG).toBool();
    quint64                     restart = session->getInformation(SESSION_RESTART).toULongLong();
    LightBird::TableDirectories directory(session->getInformation(SESSION_WORKING_DIR).toString());
    LightBird::TableFiles       file;
    Result                      result;

    if (restart)
        session->removeInformation(SESSION_RESTART);
    if (!path.contains('/'))
        file.setIdFromVirtualPath(path);
    else if (directory.cd(path.left(path.lastIndexOf('/') + 1)))
        file.setId(directory.getFile(path.right(path.size() - path.lastIndexOf('/') - 1)));
    if (file)
    {
        client.getResponse().getContent().setStorage(LightBird::IContent::FILE, file.getFullPath());
        client.getResponse().getContent().setSeek(restart);
        // Intermediate response
        Plugin::sendControlMessage(controlId, Result(150, QString("Opening %1 mode data connection for %2 (%3 bytes).").arg(binary ? "BINARY" : "ASCII", path, QString::number(QFileInfo(file.getFullPath()).size()))));
        // Final response
        client.getInformations().insert(DATA_CODE, 226);
        client.getInformations().insert(DATA_MESSAGE, "Transfer complete.");
    }
    else
        result = Result(550, QString("Can't open %1: No such file or directory.").arg(path));
    return (result);
}

Commands::Result Commands::_stor(const QString &pathName, LightBird::Session &session, LightBird::IClient &client)
{
    QString                     controlId = session->getInformation(SESSION_CONTROL_ID).toString();
    bool                        binary = session->getInformation(SESSION_BINARY_FLAG).toBool();
    quint64                     restart = session->getInformation(SESSION_RESTART).toULongLong();
    LightBird::TableDirectories directory(session->getInformation(SESSION_WORKING_DIR).toString());
    LightBird::TableFiles       file;
    QString                     filesPath = LightBird::getFilesPath();
    QString                     fileName = pathName;
    QString                     realPath;
    QString                     path;

    if (restart)
        session->removeInformation(SESSION_RESTART);
    // Checks if the file exists
    if (fileName.contains('/'))
        fileName = fileName.right(fileName.size() - fileName.lastIndexOf('/') - 1);
    if (!pathName.contains('/') || directory.cd(pathName.left(pathName.lastIndexOf('/') + 1)))
        file.setId(directory.getFile(fileName));
    else
        return (Result(501, QString("Directory \"%1\" not found.").arg(pathName.left(pathName.lastIndexOf('/')))));
    // Creates a new file
    if (!file)
    {
        // Defines the new name
        path = fileName;
        if (path.contains('.'))
            path = path.left(fileName.lastIndexOf('.'));
        path += '.' + LightBird::createUuid();
        if (fileName.contains('.'))
            path += fileName.right(fileName.size() - fileName.lastIndexOf('.'));
        realPath = filesPath + path;
        // Creates the file
        if (!QFile(realPath).open(QIODevice::WriteOnly | QIODevice::Truncate)
            || !file.add(fileName, path, "other", directory.getId(), session->getAccount()))
        {
            QFile::remove(realPath);
            return (Result(550, "Unable to create the file."));
        }
    }
    // Removes the old file or restart the transfer
    else
    {
        path = file.getPath();
        if ((realPath = file.getFullPath()).isEmpty())
            realPath = filesPath + path;
        if (restart)
        {
            QFile f(realPath);
            if (!f.open(QIODevice::ReadWrite) || (qint64)restart > f.size() || !f.resize(restart))
                return (Result(550, "Unable to RESTart the transfer."));
        }
        else if (!QFile(realPath).open(QIODevice::WriteOnly | QIODevice::Truncate))
            return (Result(550, "Unable to replace the file."));
        file.setIdAccount(session->getAccount());
    }
    // The file will be filled in the parser via the content
    client.getRequest().getContent().setStorage(LightBird::IContent::FILE, realPath);
    // The id is stored here in order to identify the file at the end of the upload
    client.getInformations().insert(DATA_UPLOAD_ID, file.getId());
    // Intermediate response
    Plugin::sendControlMessage(controlId, Result(150, QString("Opening %1 mode data connection for %2.").arg(binary ? "BINARY" : "ASCII", pathName)));
    // Final response
    client.getInformations().insert(DATA_CODE, 250);
    client.getInformations().insert(DATA_MESSAGE, QString("Transfer complete."));
    return (Result());
}

Commands::Result Commands::_stou(const QString &pathName, LightBird::Session &session, LightBird::IClient &client)
{
    QString                     controlId = session->getInformation(SESSION_CONTROL_ID).toString();
    LightBird::TableDirectories directory(session->getInformation(SESSION_WORKING_DIR).toString());
    LightBird::TableFiles       file;
    QString                     filesPath = LightBird::getFilesPath();
    QString                     fileName = pathName;
    QString                     realPath;
    QString                     path;
    QString                     extension;
    unsigned int                i = 0;

    // RESTart is not allowed here
    if (session->hasInformation(SESSION_RESTART))
    {
        session->removeInformation(SESSION_RESTART);
        return (Result(503, "RESTart is not allowed for STOU and APPE."));
    }
    // Checks if the file exists
    if (fileName.contains('/'))
        fileName = fileName.right(fileName.size() - fileName.lastIndexOf('/') - 1);
    if (!pathName.contains('/') || directory.cd(pathName.left(pathName.lastIndexOf('/') + 1)))
        file.setId(directory.getFile(fileName));
    else
        return (Result(501, QString("Directory \"%1\" not found.").arg(pathName.left(pathName.lastIndexOf('/')))));
    // Separates the extension from the file name
    if (fileName.contains('.'))
        extension = fileName.right(fileName.size() - fileName.lastIndexOf('.'));
    fileName = fileName.left(fileName.size() - extension.size());
    // A file has already this name, so we search a unique name in the directory
    if (file)
        do
            path = fileName + " - " + QString::number(++i);
        while ((directory.getId().isEmpty() && file.setIdFromVirtualPath(path + extension))
               || (!directory.getId().isEmpty() && !directory.getFile(path + extension).isEmpty()));
    // Creates the file
    if (!path.isEmpty())
        fileName = path;
    path = fileName + '.' + LightBird::createUuid() + extension;
    realPath = filesPath + path;
    if (!QFile(realPath).open(QIODevice::WriteOnly | QIODevice::Truncate)
        || !file.add(fileName + extension, path, "other", directory.getId(), session->getAccount()))
    {
        QFile::remove(realPath);
        return (Result(550, "Unable to create the file."));
    }
    // The file will be filled in the parser via the content
    client.getRequest().getContent().setStorage(LightBird::IContent::FILE, realPath);
    // The id is stored here in order to identify the file at the end of the upload
    client.getInformations().insert(DATA_UPLOAD_ID, file.getId());
    // Intermediate response
    Plugin::sendControlMessage(controlId, Result(150, QString("FILE: %1").arg(file.getVirtualPath(true, true))));
    // Final response
    client.getInformations().insert(DATA_CODE, 250);
    client.getInformations().insert(DATA_MESSAGE, QString("Transfer complete."));
    return (Result());
}

Commands::Result Commands::_appe(const QString &pathName, LightBird::Session &session, LightBird::IClient &client)
{
    QString                     controlId = session->getInformation(SESSION_CONTROL_ID).toString();
    LightBird::IContent         &content = client.getRequest().getContent();
    bool                        binary = session->getInformation(SESSION_BINARY_FLAG).toBool();
    LightBird::TableDirectories directory(session->getInformation(SESSION_WORKING_DIR).toString());
    LightBird::TableFiles       file;
    QString                     filesPath = LightBird::getFilesPath();
    QString                     fileName = pathName;
    QString                     realPath;
    QString                     path;

    // RESTart is not allowed here
    if (session->hasInformation(SESSION_RESTART))
    {
        session->removeInformation(SESSION_RESTART);
        return (Result(503, "RESTart is not allowed for APPE and STOU."));
    }
    // Checks if the file exists
    if (fileName.contains('/'))
        fileName = fileName.right(fileName.size() - fileName.lastIndexOf('/') - 1);
    if (!pathName.contains('/') || directory.cd(pathName.left(pathName.lastIndexOf('/') + 1)))
        file.setId(directory.getFile(fileName));
    else
        return (Result(501, QString("Directory \"%1\" not found.").arg(pathName.left(pathName.lastIndexOf('/')))));
    // Creates a new file
    if (!file)
    {
        // Defines the new name
        path = fileName;
        if (path.contains('.'))
            path = path.left(fileName.lastIndexOf('.'));
        path += '.' + LightBird::createUuid();
        if (fileName.contains('.'))
            path += fileName.right(fileName.size() - fileName.lastIndexOf('.'));
        realPath = filesPath + path;
        // Creates the file
        if (!QFile(realPath).open(QIODevice::WriteOnly | QIODevice::Truncate)
            || !file.add(fileName, path, "other", directory.getId(), session->getAccount()))
        {
            QFile::remove(realPath);
            return (Result(550, "Unable to create the file."));
        }
    }
    // If the file already exists the uploaded data will be append to it
    else
    {
        path = file.getPath();
        if ((realPath = file.getFullPath()).isEmpty())
            realPath = filesPath + path;
        if (!QFile(realPath).open(QIODevice::ReadWrite))
        {
            QFile::remove(realPath);
            return (Result(550, "Unable to append data to the file."));
        }
        file.setIdAccount(session->getAccount());
    }
    // The file will be filled from the end in the parser via the content
    client.getRequest().getContent().setStorage(LightBird::IContent::FILE, realPath);
    content.setSeek(content.size());
    // The id is stored here in order to identify the file at the end of the upload
    client.getInformations().insert(DATA_UPLOAD_ID, file.getId());
    // Intermediate response
    Plugin::sendControlMessage(controlId, Result(150, QString("Opening %1 mode data connection for %2.").arg(binary ? "BINARY" : "ASCII", pathName)));
    // Final response
    client.getInformations().insert(DATA_CODE, 250);
    client.getInformations().insert(DATA_MESSAGE, QString("Transfer complete."));
    return (Result());
}

LightBird::TableFiles Commands::_getFile(const QString &path, LightBird::Session &session)
{
    LightBird::TableDirectories directory(session->getInformation(SESSION_WORKING_DIR).toString());
    LightBird::TableFiles       file;
    QString                     name;

    if (!path.contains('/'))
        file.setId(directory.getFile(path));
    else
    {
        if (directory.cd(name = path.left(path.lastIndexOf('/') + 1)))
            file.setId(directory.getFile(path.right(path.size() - name.size())));
    }
    return (file);
}

LightBird::TableDirectories Commands::_getDirectory(const QString &path, LightBird::Session &session)
{
    LightBird::TableDirectories directory(session->getInformation(SESSION_WORKING_DIR).toString());

    directory.cd(path);
    return (directory);
}

QString Commands::_escapePath(const QString &path)
{
    return (QString(path).replace('"', "\"\""));
}
