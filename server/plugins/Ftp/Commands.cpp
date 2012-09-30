#include <QDir>
#include <QSqlQuery>

#include "IIdentify.h"
#include "IIdentifier.h"

#include "Dir.h"
#include "Commands.h"
#include "File.h"
#include "LightBird.h"
#include "Plugin.h"
#include "Properties.h"
#include "TableAccounts.h"
#include "TableDirectories.h"

const char *Commands::months[] = { "", "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };

Commands::Commands(LightBird::IApi *api) : api(api)
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
    this->controlCommands["SYST"] = &Commands::_syst;
    this->controlCommands["TYPE"] = &Commands::_type;
    this->controlCommands["STRU"] = &Commands::_stru;
    this->controlCommands["MODE"] = &Commands::_mode;
    this->controlCommands["PASV"] = &Commands::_pasv;
    this->controlCommands["PORT"] = &Commands::_port;
    this->controlCommands["NOOP"] = &Commands::_noop;
    this->controlCommands["ABOR"] = &Commands::_abor;
    this->controlCommands["QUIT"] = &Commands::_quit;
    this->transferCommands["LIST"] = qMakePair(true, &Commands::_list);
    this->transferCommands["RETR"] = qMakePair(true, &Commands::_retr);
    this->transferCommands["STOR"] = qMakePair(false, &Commands::_stor);
    this->anonymousCommands << "USER" << "PASS" << "HELP" << "ACCT";
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
    session->setInformation("last-command", command);
    return (result);
}

Commands::Result Commands::executeTransfer(const QString &command, const QString &parameter, LightBird::Session &session, LightBird::IClient &client)
{
    Result result = (this->*(this->transferCommands.value(command).second))(parameter, session, client);
    session->setInformation("last-command", command);
    return (result);
}

Commands::Result Commands::_user(const QString &user, LightBird::Session &, LightBird::IClient &client)
{
    Result  result;

    if(!user.trimmed().isEmpty())
    {
        client.getInformations().insert("user", user);
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

    if (session->getInformation("last-command") == "USER" && client.getInformations().contains("user"))
    {
        user = client.getInformations().value("user").toString();
        if (account.setIdFromNameAndPassword(user, pass))
        {
            result = Result(230, QString("Welcome %1.\r\nMake yourself at home!").arg(user));
            session->setAccount(account.getId());
        }
        else
            result = Result(530, "Login authentification failed.");
        client.getInformations().remove("user");
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
    LightBird::TableDirectories directory(session->getInformation("working-dir").toString());
    
    return (Result(257, QString("\"%1\" is your current location").arg(this->_escapePath(directory.getVirtualPath(true)))));
}

Commands::Result Commands::_cwd(const QString &path, LightBird::Session &session, LightBird::IClient &)
{
    LightBird::TableDirectories directory(session->getInformation("working-dir").toString());

    if (directory.cd(path))
    {
        session->setInformation("working-dir", directory.getId());
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
    LightBird::TableDirectories   directory(session->getInformation("working-dir").toString());

    // The path is absolute
    if (pathName.startsWith('/'))
        directory.clear();
    // Creates the directories in the path
    directory.setId(directory.createVirtualPath(pathName, session->getAccount()));
    return (Result(257, QString("\"%1\" directory created.").arg(this->_escapePath(directory.getVirtualPath(true)))));
}

Commands::Result Commands::_rmd(const QString &pathName, LightBird::Session &session, LightBird::IClient &)
{
    LightBird::TableDirectories   directory(session->getInformation("working-dir").toString());

    if (!directory.cd(pathName))
        return (Result(550, QString("Directory not found \"%1\".").arg(this->_escapePath(pathName))));
    directory.remove();
    return (Result(250, QString("\"%1\" directory removed.").arg(this->_escapePath(pathName))));
}

Commands::Result Commands::_rnfr(const QString &oldName, LightBird::Session &session, LightBird::IClient &client)
{
    if (!this->_getFile(oldName, session))
        return (Result(550, QString("File not found.")));
    client.getInformations().insert("oldName", oldName);
    return (Result(350, QString("Waiting for the new name of the file \"%1\".").arg(this->_escapePath(oldName))));
}

Commands::Result Commands::_rnto(const QString &newName, LightBird::Session &session, LightBird::IClient &client)
{
    LightBird::TableFiles   file;
    QString                 oldName;

    if (session->getInformation("last-command") != "RNFR" || !client.getInformations().contains("oldName"))
        return (Result(503, "Bad sequence of commands."));
    oldName = client.getInformations().value("oldName").toString();
    client.getInformations().remove("oldName");
    if (!(file = this->_getFile(oldName, session)))
        return (Result(550, QString("File not found \"%1\".").arg(this->_escapePath(oldName))));
    if (!file.setName(newName))
        return (Result(553, QString("Unable to rename the file.")));
    return (Result(250, QString("File renamed from \"%1\" to \"%2\".").arg(this->_escapePath(oldName), this->_escapePath(newName))));
}

Commands::Result Commands::_dele(const QString &path, LightBird::Session &session, LightBird::IClient &)
{
    LightBird::TableFiles   file;
    QString                 realPath;

    if (!(file = this->_getFile(path, session)))
        return (Result(550, QString("File not found.")));
    realPath = file.getFullPath();
    if (QFileInfo(realPath).isFile() && !QFile(realPath).remove())
        return (Result(450, QString("Unable to delete this file now. Try again later.")));
    if (!file.remove())
        return (Result(550, QString("Unable to delete the file.")));
    return (Result(250, QString("\"%1\" file deleted.").arg(path)));
}

Commands::Result Commands::_syst(const QString &, LightBird::Session &, LightBird::IClient &)
{
    // As far as the clients are concerned, we are in a UNIX environment.
    return (Result(215, "UNIX Type: L8"));
}

Commands::Result Commands::_type(const QString &parameter, LightBird::Session &session, LightBird::IClient &)
{
    bool    binary = session->getInformation("binary-flag").toBool();
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
    session->setInformation("binary-flag", binary);
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

Commands::Result Commands::_pasv(const QString &, LightBird::Session &session, LightBird::IClient &client)
{
    QString         protocol = Plugin::getConfiguration().dataProtocolName;
    QStringList     protocols;
    unsigned int    maxClients;
    unsigned short  port = Plugin::getConfiguration().passivePort;

    // Ensures that the passive port is opened
    if (this->api->network().getPort(port, protocols, maxClients) && protocols.contains(protocol))
    {
        session->setInformation("transfer-mode", Commands::PASSIVE);
        session->setInformation("transfer-ip", client.getPeerAddress().toString());
        session->setInformation("transfer-port", client.getPeerPort());
        return (Result(0, QString("227 Entered Passive Mode (127,0,0,1,%1,%2)\r\n").arg(QString::number(port >> 8), QString::number(port & 0xFF))));
    }
    return (Result(502, "Data port not opened.\r\nUse active connection instead."));
}

Commands::Result Commands::_port(const QString &hostPort, LightBird::Session &session, LightBird::IClient &)
{
    Result  result;
    QRegExp reg("(\\d{1,3}),(\\d{1,3}),(\\d{1,3}),(\\d{1,3}),(\\d{1,3}),(\\d{1,3})"); // Regex matching the port parameter

    if (reg.indexIn(hostPort) >= 0)
    {
        QString address = QString("%1.%2.%3.%4").arg(reg.cap(1)).arg(reg.cap(2)).arg(reg.cap(3)).arg(reg.cap(4));
        unsigned short port = reg.cap(5).toInt() << 8 | reg.cap(6).toInt();
        session->setInformation("transfer-mode", Commands::ACTIVE);
        session->setInformation("transfer-ip", address);
        session->setInformation("transfer-port", port);
        result = Result(200, "PORT command successful.");
    }
    else
        result = Result(501, "Syntax error in IP address.");
    return (result);
}

Commands::Result  Commands::_noop(const QString &, LightBird::Session &, LightBird::IClient &)
{
    return (Result(200, ":)"));
}

Commands::Result Commands::_abor(const QString &, LightBird::Session &session, LightBird::IClient &)
{
    QString dataId = session->getInformation("data-id").toString();

    if (dataId.isEmpty())
        return (Result(225, "No transfer in progress."));
    this->api->network().disconnect(dataId);
    session->getClients();
    session->setInformation("disconnect-data", true);
    return (Result(0, "426 Data connection closed.\r\n226 Transfer aborted.\r\n"));
}

Commands::Result Commands::_quit(const QString &, LightBird::Session &session, LightBird::IClient &)
{
    session->destroy();
    this->api->network().disconnect(session->getInformation("control-id").toString());
    return (Result(221, "Goodbye."));
}

Commands::Result Commands::_list(const QString &path, LightBird::Session &session, LightBird::IClient &client)
{
    LightBird::TableDirectories directory(session->getInformation("working-dir").toString());
    LightBird::IContent         &content = client.getResponse().getContent();
    QStringList                 files;
    QStringList                 directories;
    QString                     pattern("%1rwx------ %2 %3 nogroup %4 %5 %6\r\n");
    QString                     line;
    QString                     name;
    QString                     subDirectories;
    QString                     date;
    QString                     size;

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
            subDirectories = QString::number(directory.getDirectories().size() + 1);
            date = this->_listDate(directory.getModified());
            line = pattern.arg("d", subDirectories, (name.isEmpty() ? "nouser" : name), QString::number(4096), date, directory.getName());
            content.setContent(line.toUtf8());
        }
    }
    // List the files
    QStringListIterator f(files);
    while(f.hasNext())
    {
        LightBird::TableFiles file(f.next());
        name = LightBird::TableAccounts(file.getIdAccount()).getName();
        size = QString::number(QFileInfo(file.getFullPath()).size());
        date = this->_listDate(file.getModified());
        line = pattern.arg("-", "1", (name.isEmpty() ? "nouser" : name), size, date, file.getName());
        content.setContent(line.toUtf8());
    }
    return (Result(226, QString("Directory sent.\r\n%1 objects\r\n").arg(QString::number(files.size() + directories.size()))));
}

QString     Commands::_listDate(const QDateTime &datetime)
{
    QDate   date(datetime.date());

    if (date.year() == QDate::currentDate().year())
        return (Commands::months[date.month()] + datetime.toString(" dd hh:mm"));
    return (Commands::months[date.month()] + datetime.toString(" dd yyyy"));
}

Commands::Result Commands::_retr(const QString &path, LightBird::Session &session, LightBird::IClient &client)
{
    LightBird::TableDirectories directory(session->getInformation("working-dir").toString());
    LightBird::TableFiles       file;
    Result                      result;

    if (!path.contains('/'))
        file.setIdFromVirtualPath(path);
    else if (directory.cd(path.left(path.lastIndexOf('/') + 1)))
        file.setId(directory.getFile(path.right(path.size() - path.lastIndexOf('/') - 1)));
    if (file)
    {
        client.getResponse().getContent().setStorage(LightBird::IContent::FILE, file.getFullPath());
        result = Result(226, "File successfully transferred.");
    }
    else
        result = Result(550, QString("Can't open %1: No such file or directory.").arg(path));
    return (result);
}

Commands::Result Commands::_stor(const QString &pathName, LightBird::Session &session, LightBird::IClient &client)
{
    LightBird::IContent         &content = client.getRequest().getContent();
    LightBird::TableDirectories directory(session->getInformation("working-dir").toString());
    LightBird::TableFiles       file;
    QString                     filesPath = LightBird::getFilesPath();
    QString                     fileName = pathName;
    QString                     path;

    // Checks if the file exists
    if (fileName.contains('/'))
        fileName = fileName.right(fileName.size() - fileName.lastIndexOf('/') - 1);
    if (!pathName.contains('/'))
        file.setIdFromVirtualPath(pathName);
    else if (directory.cd(pathName.left(pathName.lastIndexOf('/') + 1)))
        file.setId(directory.getFile(fileName));
    else
        return (Result(501, QString("Directory \"%1\" not found.").arg(pathName.left(pathName.lastIndexOf('/')))));
    // Removes the file if it already exists
    if (file)
    {
        path = file.getFullPath();
        if (QFileInfo(path).isFile() && (!QFile::remove(path) || !file.remove()))
            return (Result(550, "Unable to replace the file."));
    }
    // Defines the real name of the file
    path = filesPath + fileName;
    if (path.contains('.'))
        path = path.left(fileName.lastIndexOf('.'));
    path += '.' + LightBird::createUuid();
    if (fileName.contains('.'))
        path += fileName.right(fileName.size() - fileName.lastIndexOf('.'));
    // Creates the file
    if (!QFile(fileName).open(QIODevice::WriteOnly)
        || !file.add(fileName, path, "other", directory.getId(), session->getAccount()))
        return (Result(550, "Unable to create the file."));
    content.setStorage(LightBird::IContent::FILE, path);
    return (Result(250, "File successfully transferred."));
}

LightBird::TableFiles Commands::_getFile(const QString &path, LightBird::Session &session)
{
    LightBird::TableDirectories directory(session->getInformation("working-dir").toString());
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

QString Commands::_escapePath(const QString &path)
{
    return (QString(path).replace('"', "\"\""));
}
