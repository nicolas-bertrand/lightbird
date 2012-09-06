#include "Execute.h"
#include "LightBird.h"
#include "Properties.h"
#include "IIdentify.h"
#include "IIdentifier.h"

#include "File.h"
#include "Dir.h"

#include "TableFiles.h"

#include <QDir>
#include <QSqlQuery>

Execute::Execute(LightBird::IApi *api) : api(api)
{
}

Execute::~Execute()
{
}

Execute::MethodResult Execute::doGreeting(QString, LightBird::Session)
{
    return MethodResult(
            220,
            "Welcome to Lightbird's FTP server\r\n"
            "Please authenticate\r\n"
            "And of course, have fun !\r\n"
            );
}

Execute::MethodResult Execute::doUser(QString parameter, LightBird::Session session)
{
    MethodResult ret;
    QString username = parameter;
    if(!username.trimmed().isEmpty()) // If the string is not blank
    {
        session->setInformation("username", username);
        ret.first = 331;
        ret.second = QString("User %1 OK. Password required.\r\n").arg(username);
    }
    else
    {
        ret.first = 530;
        ret.second = "Anonymous login not alowed.\r\n";
    }
    return ret;
}

Execute::MethodResult Execute::doPass(QString parameter, LightBird::Session session)
{
    MethodResult ret;
    if (session->hasInformation("username"))
    {
        QString username = session->getInformation("username").toString();
        QString password = parameter;
        QSqlQuery query;
        QVector<QVariantMap>    result;

        LightBird::IDatabase &database = this->api->database();
        query.prepare(database.getQuery("FtpExecute", "select_account"));
        query.bindValue(":name", username);

        api->log().trace("FTP AUTH", Properties("user", username).add("pass", password).toMap(), "Execute", "doPass");
        
        if (database.query(query, result) && result.size() >= 1 && result[0]["password"].toByteArray() == LightBird::sha256(password.toAscii() + result[0]["id"].toByteArray()))
        {
            ret.first = 230;
            ret.second = "Login authentification OK.\r\n";

            session->setAccount(result[0]["id"].toString());
        }
        else
        {
            ret.first = 530;
            ret.second = "Login authentification failed.\r\n";
            api->log().trace("FTP AUTH FAILED", Properties("count", result.size()).toMap(), "Execute", "doPass");
            if (result.size() >= 1)
            {
                api->log().trace("Ftp password mismatch", Properties("computed", LightBird::sha256(password.toAscii() + result[0]["id"].toByteArray())).add("stored", result[0]["password"]).toMap(), "Execute", "doPass");
            }
        }
    }
    else
    {
        ret.first = 530;
        ret.second = "Please tell me who you are first.\r\n";
    }

    return ret;
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

    return MethodResult(215, syst + " Type: L8\r\n");
}

Execute::MethodResult Execute::doPwd(QString, LightBird::Session session)
{
    LightBird::Dir wd = LightBird::Dir::byId(session->getInformation("working-dir").toString());
    
    return MethodResult(257, QString("\"%1\" is your current location\r\n").arg(wd.getPath()));
}

Execute::MethodResult Execute::doType(QString parameter, LightBird::Session session)
{
    QString type = parameter;
    int code = 200;
    QString message;
    bool binary = session->getInformation("binary-flag").toBool();
    if (type.size() > 0) // We have a type appended
    {
        QString first = type.at(0);
        if (QString::compare(first, "A", Qt::CaseInsensitive) == 0)
        {
            binary = false;
        }
        else if (QString::compare(first, "I", Qt::CaseInsensitive) == 0)
        {
            binary = true;
        }
        else if (QString::compare(first, "L", Qt::CaseInsensitive) == 0)
        {
            if (type.size() > 1 && type.at(1).isDigit())
            {
                if (type.at(1) != '8')
                {
                    message += "Only 8-bit bytes are supported\r\n";
                }
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

    message += QString("TYPE is now %1\r\n").arg(binary?"8-bit binary":"ASCII");
    session->setInformation("binary-flag", binary);
    return MethodResult(code, message);
}

Execute::MethodResult Execute::doPort(QString parameter, LightBird::Session session)
{
    MethodResult ret;
    QRegExp reg("(\\d{1,3}),(\\d{1,3}),(\\d{1,3}),(\\d{1,3}),(\\d+),(\\d+)"); // Regex matching a port parameter

    if (reg.indexIn(parameter) >= 0)
    {
        QString address = QString("%1.%2.%3.%4").arg(reg.cap(1)).arg(reg.cap(2)).arg(reg.cap(3)).arg(reg.cap(4));
        int port = reg.cap(5).toInt() << 8 | reg.cap(6).toInt();

        //this->api->log().trace("Port command " + address + " " + QString::number(port));

        session->setInformation("transfer-mode", TransferModePort);
        session->setInformation("transfer-ip", address);
        session->setInformation("transfer-port", port);

        ret.first = 200;
        ret.second = "PORT command successful\r\n";


    }
    else
    {
        ret.first = 501;
        ret.second = "Syntax error in IP address\r\n";
    }
    return ret;
}

Execute::MethodResult Execute::doCwd(QString parameter, LightBird::Session session)
{
    LightBird::Dir wd = LightBird::Dir::byId(session->getInformation("working-dir").toString());
    LightBird::Dir cwd = LightBird::Dir::byPath(parameter, wd);

    if (cwd)
    {
        session->setInformation("working-dir", cwd.getId());
        return MethodResult(250, QString("OK. Current directory is \"%1\"\r\n").arg(cwd.getPath()));
    }
    else
        return MethodResult(550, QString("Can't change directory to %1\r\n").arg(parameter));

    
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
    MethodResult ret;
    LightBird::Dir wd = LightBird::Dir::byId(session->getInformation("working-dir").toString());
    LightBird::Node *node = LightBird::Node::byPath(parameter, wd);

    if (node)
    {
        // TODO: set storage
        //response->getContent().setStorage(LightBird::IContent::FILE, table->getFullPath());
        ret = MethodResult(226, 
            "File successfully transferred\r\n"
            );
    }
    else
        ret = MethodResult(550, QString("Can't open %1: No such file or directory").arg(parameter));

    return ret;
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


