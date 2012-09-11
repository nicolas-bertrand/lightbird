#include <QDir>
#include <QSqlQuery>

#include "IIdentify.h"
#include "IIdentifier.h"

#include "Dir.h"
#include "Execute.h"
#include "File.h"
#include "LightBird.h"
#include "Properties.h"
#include "TableAccounts.h"
#include "TableFiles.h"

Execute::Execute(LightBird::IApi *api) : api(api)
{
}

Execute::~Execute()
{
}

Execute::MethodResult Execute::doGreeting(QString, LightBird::Session)
{
    return (MethodResult(220, "Welcome to Lightbird's FTP server\r\n"
                              "Please authenticate\r\n"
                              "And of course, have fun !\r\n"));
}

Execute::MethodResult Execute::doUser(QString user, LightBird::Session session)
{
    MethodResult      result;

    if(!user.trimmed().isEmpty())
    {
        session->setInformation("user", user);
        result = MethodResult(331, QString("User %1 OK. Password required.\r\n").arg(user));
    }
    else
        result = MethodResult(530, "Anonymous login not alowed.\r\n");
    return (result);
}

Execute::MethodResult Execute::doPass(QString pass, LightBird::Session session)
{
    LightBird::TableAccounts account;
    QString                  user;
    MethodResult             result;

    if (session->hasInformation("user"))
    {
        user = session->getInformation("user").toString();
        if (account.setIdFromNameAndPassword(user, pass))
        {
            result = MethodResult(230, "Login authentification OK.\r\n");
            session->setAccount(account.getId());
        }
        else
            result = MethodResult(530, "Login authentification failed.\r\n");
    }
    else
        result = MethodResult(530, "Please tell me who you are first.\r\n");
    return (result);
}

Execute::MethodResult Execute::doSyst(QString, LightBird::Session)
{
#if   defined(Q_OS_UNIX)
    QString syst = "UNIX";
#elif defined(Q_OS_WIN32)
    QString syst = "WIN32";
#else
# warning Please add your system type here
    QString syst = "UNKNOWN";
#endif
    return (MethodResult(215, syst + " Type: L8\r\n"));
}

Execute::MethodResult Execute::doPwd(QString, LightBird::Session session)
{
    LightBird::Dir wd = LightBird::Dir::byId(session->getInformation("working-dir").toString());
    
    return (MethodResult(257, QString("\"%1\" is your current location\r\n").arg(wd.getPath())));
}

Execute::MethodResult Execute::doType(QString type, LightBird::Session session)
{
    int     code = 200;
    QString message;
    bool    binary = session->getInformation("binary-flag").toBool();

    if (!type.isEmpty()) // We have a type appended
    {
        QString first = type.at(0);
        if (QString::compare(first, "A", Qt::CaseInsensitive) == 0)
            binary = false;
        else if (QString::compare(first, "I", Qt::CaseInsensitive) == 0)
            binary = true;
        else if (QString::compare(first, "L", Qt::CaseInsensitive) == 0)
        {
            if (type.size() > 1 && type.at(1).isDigit())
            {
                if (type.at(1) != '8')
                    message += "Only 8-bit bytes are supported\r\n";
            }
            else
                message += "Missing argument\r\n";
            binary = true;
        }
        else
        {
            code = 504;
            message += QString("Unknown Type : %1\r\n").arg(type);
        }
    }
    else
    {
        code  = 501;
        message += "Missing argument\r\n"
                   "A(scii) I(mage) L(ocal)\r\n";
    }
    message += QString("TYPE is now %1\r\n").arg(binary ? "8-bit binary" : "ASCII");
    session->setInformation("binary-flag", binary);
    return (MethodResult(code, message));
}

Execute::MethodResult Execute::doPort(QString parameter, LightBird::Session session)
{
    MethodResult result;
    QRegExp      reg("(\\d{1,3}),(\\d{1,3}),(\\d{1,3}),(\\d{1,3}),(\\d+),(\\d+)"); // Regex matching a port parameter

    if (reg.indexIn(parameter) >= 0)
    {
        QString address = QString("%1.%2.%3.%4").arg(reg.cap(1)).arg(reg.cap(2)).arg(reg.cap(3)).arg(reg.cap(4));
        int port = reg.cap(5).toInt() << 8 | reg.cap(6).toInt();
        session->setInformation("transfer-mode", TransferModePort);
        session->setInformation("transfer-ip", address);
        session->setInformation("transfer-port", port);
        result = MethodResult(200, "PORT command successful\r\n");
    }
    else
        result = MethodResult(501, "Syntax error in IP address\r\n");
    return (result);
}

Execute::MethodResult Execute::doCwd(QString parameter, LightBird::Session session)
{
    LightBird::Dir wd = LightBird::Dir::byId(session->getInformation("working-dir").toString());
    LightBird::Dir cwd = LightBird::Dir::byPath(parameter, wd);

    if (cwd)
    {
        session->setInformation("working-dir", cwd.getId());
        return (MethodResult(250, QString("OK. Current directory is \"%1\"\r\n").arg(cwd.getPath())));
    }
    else
        return (MethodResult(550, QString("Can't change directory to %1\r\n").arg(parameter)));
}

Execute::MethodResult Execute::doCdup(QString, LightBird::Session session)
{
    return doCwd("..", session);
}

Execute::MethodResult Execute::doList(QString parameter, LightBird::Session session, LightBird::IRequest *request, LightBird::IResponse *response)
{
    LightBird::Dir wd = LightBird::Dir::byId(session->getInformation("working-dir").toString());
    LightBird::DirIterator it(wd);
    int count = 0;

    while(it.hasNext())
    {
        LightBird::Node *node = it.next();

        if (!node)
            continue;

        response->getContent().setContent(
                QString(
                    "%1rwx------ 1 %2 nogroup %3 %4 %5\r\n"
                    ).arg(
                        ((node->getNodeType() == LightBird::Node::DirNode)?"d":"-"),
                        node->getOwner(),
                        "0", //QString::number(table->getInformation("size").toInt()),
                        "Jan 20 15:36", //row["modified"].toDateTime().toString("MMM dd hh:mm"),
                        node->getName()
                    ).toUtf8(), true);
        count ++;
        delete node;
    }
    return MethodResult(226,
            QString(
                "Options: -a -l\r\n"
                "%1 matches total\r\n"
            ).arg(QString::number(count))
            );
}

Execute::MethodResult Execute::doRetr(QString parameter, LightBird::Session session, LightBird::IRequest *request, LightBird::IResponse *response)
{
    MethodResult result;
    LightBird::Dir wd = LightBird::Dir::byId(session->getInformation("working-dir").toString());
    //LightBird::Node *node = LightBird::Node::byPath(parameter, wd);
    LightBird::TableFiles file;

    file.setIdFromVirtualPath(parameter);
    if (file)
    {
        response->getContent().setStorage(LightBird::IContent::FILE, file.getFullPath());
        result = MethodResult(226, "File successfully transferred\r\n");
    }
    else
        result = MethodResult(550, QString("Can't open %1: No such file or directory").arg(parameter));
    return (result);
}

Execute::MethodResult Execute::doStor(QString parameter, LightBird::Session session, LightBird::IRequest *request, LightBird::IResponse *response)
{
    Upload upload;
    upload.file = request->getContent().getTemporaryFile();
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

    return MethodResult(226, "File successfully transferred\r\n");
}


