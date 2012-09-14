#include <QDir>
#include <QSqlQuery>

#include "IIdentify.h"
#include "IIdentifier.h"

#include "Dir.h"
#include "Commands.h"
#include "File.h"
#include "LightBird.h"
#include "Properties.h"
#include "TableAccounts.h"
#include "TableDirectories.h"
#include "TableFiles.h"

Commands::Commands(LightBird::IApi *api) : api(api)
{
    this->controlCommands["USER"] = &Commands::_user;
    this->controlCommands["PASS"] = &Commands::_pass;
    this->controlCommands["ACCT"] = &Commands::_acct;
    this->controlCommands["HELP"] = &Commands::_help;
    this->controlCommands["PWD"] =  &Commands::_pwd;
    this->controlCommands["CWD"] = &Commands::_cwd;
    this->controlCommands["MKD"] = &Commands::_mkd;
    this->controlCommands["RMD"] = &Commands::_rmd;
    this->controlCommands["CDUP"] = &Commands::_cdup;
    this->controlCommands["SYST"] = &Commands::_syst;
    this->controlCommands["TYPE"] = &Commands::_type;
    this->controlCommands["STRU"] = &Commands::_stru;
    this->controlCommands["MODE"] = &Commands::_mode;
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

bool    Commands::isTransfert(const QString &command)
{
    return (this->transferCommands.contains(command));
}

bool    Commands::isSender(const QString &command)
{
    if (this->transferCommands.contains(command))
        return (this->transferCommands.value(command).first);
    return (true);
}

Commands::Result Commands::executeControl(const QString &command, const QString parameter, LightBird::Session &session)
{
    Result  result;

    if (!session->getAccount().isEmpty() || this->anonymousCommands.contains(command))
        result = (this->*(this->controlCommands.value(command)))(parameter, session);
    else
        result = Result(530, "Please login with USER and PASS.");
    session->setInformation("last-command", command);
    return (result);
}

Commands::Result Commands::executeTransfert(const QString &command, const QString &parameter, LightBird::Session &session, LightBird::IClient &client)
{
    Result  result;

    if (!session->getAccount().isEmpty() || this->anonymousCommands.contains(command))
        result = (this->*(this->transferCommands.value(command).second))(parameter, session, client);
    else
        result = Result(530, "Please login with USER and PASS.");
    session->setInformation("last-command", command);
    return (result);
}

Commands::Result Commands::_user(const QString &user, LightBird::Session &session)
{
    Result  result;

    if(!user.trimmed().isEmpty())
    {
        session->setInformation("user", user);
        result = Result(331, QString("User %1 OK. Password required.").arg(user));
    }
    else
        result = Result(530, "Anonymous login not alowed.");
    return (result);
}

Commands::Result Commands::_pass(const QString &pass, LightBird::Session &session)
{
    LightBird::TableAccounts account;
    QString      user;
    Result       result;

    if (session->getInformation("last-command") == "USER" && session->hasInformation("user"))
    {
        user = session->getInformation("user").toString();
        if (account.setIdFromNameAndPassword(user, pass))
        {
            result = Result(230, QString("Welcome %1.\r\nMake yourself at home!").arg(user));
            session->setAccount(account.getId());
        }
        else
            result = Result(530, "Login authentification failed.");
        session->removeInformation("user");
    }
    else
        result = Result(503, "Bad sequence of commands.");
    return (result);
}

Commands::Result Commands::_acct(const QString &, LightBird::Session &)
{
    return (Result(202, "Command superfluous."));
}

Commands::Result Commands::_help(const QString &, LightBird::Session &)
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

Commands::Result Commands::_pwd(const QString &, LightBird::Session &session)
{
    LightBird::TableDirectories directory(session->getInformation("working-dir").toString());
    
    return (Result(257, QString("\"%1\" is your current location").arg(this->_escapePath(directory.getVirtualPath(true)))));
}

Commands::Result Commands::_cwd(const QString &path, LightBird::Session &session)
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

Commands::Result Commands::_cdup(const QString &, LightBird::Session &session)
{
    return (this->_cwd("..", session));
}

Commands::Result Commands::_mkd(const QString &pathname, LightBird::Session &session)
{
    LightBird::TableDirectories   directory(session->getInformation("working-dir").toString());

    // The path is absolute
    if (pathname.startsWith('/'))
        directory.clear();
    // Creates the directories in the path
    directory.setId(directory.createVirtualPath(pathname, session->getAccount()));
    return (Result(257, QString("\%1\" directory created.").arg(this->_escapePath(directory.getVirtualPath(true)))));
}

Commands::Result Commands::_rmd(const QString &pathname, LightBird::Session &session)
{
    LightBird::TableDirectories   directory(session->getInformation("working-dir").toString());

    if (!directory.cd(pathname))
        return (Result(550, QString("Directory not found \"%1\".").arg(this->_escapePath(pathname))));
    directory.remove();
    return (Result(250, QString("\%1\" directory removed.").arg(this->_escapePath(pathname))));
}

Commands::Result Commands::_syst(const QString &, LightBird::Session &)
{
    // As far as the clients are concerned, we are in a UNIX environment.
    return (Result(215, "UNIX Type: L8"));
}

Commands::Result Commands::_type(const QString &parameter, LightBird::Session &session)
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

Commands::Result Commands::_stru(const QString &structure, LightBird::Session &)
{
    if (QString(structure).toUpper() != "F")
        return (Result(504, "Bad STRU command."));
    return (Result(200, QString("Structure set to %1.").arg(structure)));
}

Commands::Result Commands::_mode(const QString &mode, LightBird::Session &)
{
    if (QString(mode).toUpper() != "S")
        return (Result(504, "Bad MODE command."));
    return (Result(200, QString("Mode set to %1.").arg(mode)));
}

Commands::Result Commands::_port(const QString &hostPort, LightBird::Session &session)
{
    Result  result;
    QRegExp reg("(\\d{1,3}),(\\d{1,3}),(\\d{1,3}),(\\d{1,3}),(\\d{1,3}),(\\d{1,3})"); // Regex matching the port parameter

    if (reg.indexIn(hostPort) >= 0)
    {
        QString address = QString("%1.%2.%3.%4").arg(reg.cap(1)).arg(reg.cap(2)).arg(reg.cap(3)).arg(reg.cap(4));
        unsigned short port = reg.cap(5).toInt() << 8 | reg.cap(6).toInt();
        session->setInformation("transfer-mode", TransferModePort);
        session->setInformation("transfer-ip", address);
        session->setInformation("transfer-port", port);
        result = Result(200, "PORT command successful.");
    }
    else
        result = Result(501, "Syntax error in IP address.");
    return (result);
}

Commands::Result  Commands::_noop(const QString &, LightBird::Session &)
{
    return (Result(200, ":)"));
}

Commands::Result Commands::_abor(const QString &, LightBird::Session &session)
{
    QString dataId = session->getInformation("data-id").toString();

    if (dataId.isEmpty())
        return (Result(225, "No transfer in progress."));
    this->api->network().disconnect(dataId);
    session->removeInformation("data-id");
    return (Result(226, "Transfert aborted. Data connection closed."));
}

Commands::Result Commands::_quit(const QString &, LightBird::Session &session)
{
    session->destroy();
    this->api->network().disconnect(session->getInformation("control-id").toString());
    return (Result(221, "Goodbye."));
}

Commands::Result Commands::_list(const QString &, LightBird::Session &session, LightBird::IClient &client)
{
    LightBird::Dir wd = LightBird::Dir::byId(session->getInformation("working-dir").toString());
    LightBird::DirIterator it(wd);
    int count = 0;

    while(it.hasNext())
    {
        LightBird::Node *node = it.next();

        if (!node)
            continue;
        client.getResponse().getContent().setContent(
                QString(
                    "%1rwx------ 1 %2 nogroup %3 %4 %5\r\n"
                    ).arg(
                        ((node->getNodeType() == LightBird::Node::DirNode)?"d":"-"),
                        "user",
                        "0", //QString::number(table->getInformation("size").toInt()),
                        "Jan 20 15:36", //row["modified"].toDateTime().toString("MMM dd hh:mm"),
                        node->getName()
                    ).toUtf8(), true);
        count ++;
        delete node;
    }
    return Result(226,
            QString(
                "Options: -a -l\r\n"
                "%1 matches total\r\n"
            ).arg(QString::number(count))
            );
}

Commands::Result Commands::_retr(const QString &parameter, LightBird::Session &session, LightBird::IClient &client)
{
    Result result;
    LightBird::Dir wd = LightBird::Dir::byId(session->getInformation("working-dir").toString());
    //LightBird::Node *node = LightBird::Node::byPath(parameter, wd);
    LightBird::TableFiles file;

    file.setIdFromVirtualPath(parameter);
    if (file)
    {
        client.getResponse().getContent().setStorage(LightBird::IContent::FILE, file.getFullPath());
        result = Result(226, "File successfully transferred.");
    }
    else
        result = Result(550, QString("Can't open %1: No such file or directory.").arg(parameter));
    return (result);
}

Commands::Result Commands::_stor(const QString &parameter, LightBird::Session &session, LightBird::IClient &client)
{
    Upload upload;
    upload.file = client.getRequest().getContent().getTemporaryFile();
    upload.name = parameter;
    upload.parent = session->getInformation("working-dir").toString();
    
    
    // TODO: do this in another thread
    
    QString path;

    if (upload.name.contains("."))
        path = upload.name.left(upload.name.indexOf('.'));
    else
        path = upload.name;
    path += '.' + LightBird::createUuid();
    if (upload.name.contains("."))
        path += upload.name.right(upload.name.size() - upload.name.indexOf('.'));
    path = QDir().cleanPath(api->configuration().get("filesPath")) + "/" + path;
    QList<void *>   extensions;
    LightBird::IIdentify::Information information;

    if (!(extensions = api->extensions().get("IIdentifier")).isEmpty())
        information = static_cast<LightBird::IIdentifier *>(extensions.first())->identify(upload.file->fileName());
    api->extensions().release(extensions);
    if (information.data.value("mime").toString() == "application/octet-stream")
        information.data.remove("mime"); 
    
    LightBird::IDatabase &database = this->api->database();
    LightBird::TableFiles file;
    QSqlQuery query;
    QVector<QVariantMap> result;
    query.prepare(database.getQuery("FtpExecute", "select_file"));
    query.bindValue(":name", upload.name);
    query.bindValue(":parent", upload.parent);


    // Are we reuploading a file ?
    if (database.query(query, result) && result.size() >= 1)
    {
        file.setId(result[0]["id"].toString());
        file.setType(information.type_string);
        file.setInformations(information.data);
    }
    else // Create a new one
    {

        file.add(upload.name, path, information.data, information.type_string, upload.parent, session->getAccount());
    }

    LightBird::copy(upload.file->fileName(), file.getPath());

    return (Result(226, "File successfully transferred."));
}

QString Commands::_escapePath(const QString &path)
{
    return (QString(path).replace('"', "\"\""));
}
