#include <QTcpServer>

#include "LightBird.h"
#include "Ftp.h"

Ftp::Ftp(LightBird::IApi &a) : api(a),
                               database(api.database()),
                               log(api.log())
{
    bool    result = true;

    if (!(this->port = this->api.configuration(true).get("ftp/port").toUShort()))
        this->port = 21;
    this->log.info("Runing the tests of the FTP server", "Ftp", "Ftp");
    if (!this->_tests())
        result = false;
    //if (!this->_data())
    //    result = false;
    if (result)
        this->log.info("All the tests of the FTP server were successful!", "Ftp", "Ftp");
    else
        this->log.info("At least one test failed!", "Ftp", "Ftp");
}

Ftp::~Ftp()
{
}

bool            Ftp::_tests()
{
    QTcpSocket  s;
    QTcpServer  server;
    QTcpSocket  *c;
    QString     str;

    this->log.debug("Running the tests...", "Ftp", "_tests");
    try
    {
        s.connectToHost(QHostAddress::LocalHost, this->port);
        ASSERT(s.waitForConnected(MSEC));
        ASSERT(s.waitForReadyRead(MSEC));
        ASSERT(s.readAll().contains("220"));
        COMMAND("SYST", "530");
        COMMAND("PASS pass", "503");
        COMMAND("USER", "530");
        COMMAND("USER user", "331");
        COMMAND("ACCT user", "202");
        COMMAND("PASS pass", "503");
        COMMAND("USER user", "331");
        COMMAND("PASS false", "530");
        COMMAND("USER user", "331");
        COMMAND("PASS pass", "230");
        COMMAND("PASS pass", "503");
        // -----
        COMMAND("PWD", "257 \"/\"");
        COMMAND("RMD testFtp", "");
        COMMAND("CWD testFtp", "550");
        COMMAND("MKD testFtp/d1", "257");
        COMMAND("PWD", "257 \"/\"");
        COMMAND("CWD testFtp/d1", "250");
        COMMAND("PWD", "257 \"/testFtp/d1\"");
        COMMAND("CWD testFtp", "550");
        COMMAND("CWD /testFtp", "250");
        COMMAND("MKD d1", "257");
        COMMAND("CWD d1", "250");
        COMMAND("MKD /testFtp/d1/d2", "257");
        COMMAND("CWD /../d1/d2", "550");
        COMMAND("CWD ../d1/d2", "250");
        COMMAND("PWD", "257 \"/testFtp/d1/d2\"");
        // -----
        COMMAND("TYPE I", "200");
        COMMAND("TYPE A", "200");
        COMMAND("TYPE A N", "200");
        COMMAND("TYPE A T", "504");
        COMMAND("TYPE", "501");
        COMMAND("TYPE L", "501");
        COMMAND("TYPE L8", "200");
        COMMAND("STRU F", "200");
        COMMAND("STRU", "504");
        COMMAND("STRU R", "504");
        COMMAND("MODE S", "200");
        COMMAND("MODE", "504");
        COMMAND("MODE C", "504");
        // -----
        COMMAND("ABOR", "225");
        COMMAND(" PORT 127,0,0,1,0,142", "200");
        ASSERT(server.listen(QHostAddress::Any, 142));
        COMMAND("CWD /", "250");
        COMMAND("LIST", "150");
        ASSERT(server.waitForNewConnection(MSEC));
        ASSERT(c = server.nextPendingConnection());
        while (!str.contains("testFtp"))
            ASSERT(c->waitForReadyRead(MSEC) && !(str += c->readAll()).isEmpty());
        ASSERT(s.waitForReadyRead(MSEC) && s.readAll().contains("226"));
        ASSERT(c->waitForDisconnected(MSEC));
        delete c;
        // -----
        COMMAND("CWD /testFtp/d1/d2", "250");
        COMMAND("RMD /testFtp", "250");
        COMMAND("PWD", "257 \"/\"");
        COMMAND("QUIT", "221");
    }
    catch (QMap<QString, QString> properties)
    {
        this->log.error("Test failed", properties, "Ftp", "_tests");
        return (false);
    }
    this->log.debug("Test successful!", "Ftp", "_tests");
    return (true);
}

bool            Ftp::_data()
{

    this->log.debug("Running the tests...", "Ftp", "_data");
    try
    {
        QTcpSocket c;
        c.connectToHost("ftp.mozilla.org", this->port);
        ASSERT(c.waitForConnected(MSEC));
        ASSERT(c.waitForReadyRead(MSEC));
        ASSERT(c.readAll().contains("220"));
        this->_print(c, "USER anonymous");
        this->_print(c, "PASS anonymous");
        this->_print(c, "TYPE I");
        QString addr = this->_print(c, "PASV");
        QRegExp reg("(\\d{1,3}),(\\d{1,3}),(\\d{1,3}),(\\d{1,3}),(\\d{1,3}),(\\d{1,3})");
        reg.indexIn(addr);
        QTcpSocket s;
        s.connectToHost(reg.cap(1) + "." + reg.cap(2) + "." + reg.cap(3) + "." + reg.cap(4), reg.cap(5).toInt() << 8 | reg.cap(6).toInt());
        ASSERT(s.waitForConnected(MSEC));
        this->_print(c, "LIST index.html");
        this->_print(s, "");
    }
    catch (QMap<QString, QString> properties)
    {
        this->log.error("Test failed", properties, "Ftp", "_data");
        return (false);
    }
    this->log.debug("Test successful!", "Ftp", "_data");
    return (true);
}

bool        Ftp::_command(QTcpSocket &s, const QString &request, const QString &response)
{
    QString result;

    if (s.write(QString(request + "\r\n").toAscii()) == -1 || !s.waitForBytesWritten(MSEC))
        return (false);
    if (!s.waitForReadyRead(MSEC) || !(result = s.readAll()).contains(response.toAscii()))
    {
        this->log.error("Test failed", Properties("response", result.trimmed()).toMap(), "Ftp", "_command");
        return (false);
    }
    return (true);
}

#ifdef WIN32
# include "windows.h"
#endif
QString     Ftp::_print(QTcpSocket &s, const QString &request)
{
    QString result;

    if (!request.isEmpty())
        if (s.write(QString(request + "\r\n").toAscii()) == -1 || !s.waitForBytesWritten(MSEC))
            return (result);
#ifdef WIN32
    Sleep(1000);
#endif
    if (!s.waitForReadyRead(MSEC) || (result = s.readAll()).isEmpty())
        return (result);
    this->log.info(request + " =\n" + result.trimmed());
    return (result);
}
