#include "LightBird.h"
#include "Ftp.h"
#include "TableAccounts.h"

Ftp::Ftp(LightBird::IApi &api) : ITest(api)
{
    if (!(this->port = this->api.configuration(true).get("ftp/port").toUShort()))
        this->port = 21;
}

Ftp::~Ftp()
{
}

unsigned int    Ftp::run()
{
    log.debug("Running the tests of the FTP server...", "Ftp", "run");
    try
    {
        this->_tests();
    }
    catch (unsigned int line)
    {
        this->log.debug("Tests of the FTP server failed!", Properties("line", line).toMap(), "Ftp", "run");
        return (line);
    }
    this->log.debug("Tests of the FTP server successful!", "Ftp", "run");
    return (0);
}

void              Ftp::_tests()
{
    QTcpSocket control;
    QTcpServer server;
    QTcpSocket *data;
    QString    str;
    quint16    port;
    QRegExp    regex;
    LightBird::TableAccounts a;

    try
    {
        a.remove(a.getIdFromName(FTP_USER));
        ASSERT(a.add(FTP_USER, FTP_PASS, true, false));
        // -----
        control.connectToHost(QHostAddress::LocalHost, this->port);
        ASSERT(control.waitForConnected(MSEC));
        ASSERT(control.waitForReadyRead(MSEC));
        ASSERT(control.readAll().contains("220"));
        COMMAND("SYST", "530");
        COMMAND("PASS " + FTP_PASS, "503");
        COMMAND("USER", "530");
        COMMAND("USER " + FTP_USER, "331");
        COMMAND("ACCT " + FTP_USER, "202");
        COMMAND("PASS " + FTP_PASS, "503");
        COMMAND("USER " + FTP_USER, "331");
        COMMAND("PASS false", "530");
        COMMAND("USER " + FTP_USER, "331");
        COMMAND("PASS " + FTP_PASS, "530");
        ASSERT(a.isActive(true));
        COMMAND("USER " + FTP_USER, "331");
        COMMAND("PASS " + FTP_PASS, "230");
        COMMAND("PASS " + FTP_PASS, "503");
        COMMAND("USER " + FTP_USER, "530");
        // -----
        COMMAND("PWD", "257 \"/\"");
        COMMAND("RMD testFtp", "50");
        COMMAND("CWD testFtp", "550");
        COMMAND("MKD testFtp/d1", "257");
        COMMAND("PWD", "257 \"/\"");
        COMMAND("CWD testFtp/d1", "250");
        COMMAND("PWD", "257 \"/testFtp/d1\"");
        COMMAND("CWD testFtp", "550");
        COMMAND("CWD /testFtp", "250");
        COMMAND("MKD d1", "257");
        COMMAND("MKD d1", "257");
        COMMAND("CWD d1", "250");
        COMMAND("MKD /testFtp/d1/d2", "257");
        COMMAND("CWD /../d1/d2", "550");
        COMMAND("CWD ../d1/d2", "250");
        COMMAND("PWD", "257 \"/testFtp/d1/d2\"");
        COMMAND("CDUP", "250");
        COMMAND("PWD", "257 \"/testFtp/d1\"");
        COMMAND("CDUP", "250");
        COMMAND("CDUP", "250");
        COMMAND("PWD", "257 \"/\"");
        COMMAND("CDUP", "550");
        // -----
        COMMAND("TYPE A", "200");
        COMMAND("TYPE A N", "200");
        COMMAND("TYPE A T", "504");
        COMMAND("TYPE I", "200");
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
        COMMAND("HELP", "214");
        COMMAND("ALLO 42", "202");
        COMMAND("ABOR", "225");
        // -----
        data = new QTcpSocket();
        regex.setPattern("^227.+\\((\\d{1,3}),(\\d{1,3}),(\\d{1,3}),(\\d{1,3}),(\\d{1,3}),(\\d{1,3})\\)");
        ASSERT(regex.indexIn(this->_command(control, "PASV")) >= 0);
        port = regex.cap(5).toInt() << 8 | regex.cap(6).toInt();
        data->connectToHost(QHostAddress::LocalHost, port);
        ASSERT(data->waitForConnected(MSEC));
        COMMAND("LIST", "150");
        str.clear();
        while (!str.contains("testFtp"))
            ASSERT(data->waitForReadyRead(MSEC) && !(str += data->readAll()).isEmpty());
        ASSERT(control.waitForReadyRead(MSEC) && control.readAll().contains("226"));
        delete data;
        // -----
        COMMAND("EPSV 1", "229");
        COMMAND("EPSV 2", "229");
        COMMAND("EPSV 3", "522");
        data = new QTcpSocket();
        regex.setPattern("^229.+\\(\\|\\|\\|(\\d{1,5})\\|\\)");
        ASSERT(regex.indexIn(this->_command(control, "EPSV")) == 0);
        port = regex.cap(1).toInt();
        data->connectToHost(QHostAddress::LocalHost, port);
        ASSERT(data->waitForConnected(MSEC));
        COMMAND("LIST", "150");
        str.clear();
        while (!str.contains("testFtp"))
            ASSERT(data->waitForReadyRead(MSEC) && !(str += data->readAll()).isEmpty());
        ASSERT(control.waitForReadyRead(MSEC) && control.readAll().contains("226"));
        delete data;
        // -----
        COMMAND(" PORT 127,0,0,1," + QString::number(ACTIVE_PORT >> 8) + "," + QString::number(ACTIVE_PORT & 0xFF), "200");
        ASSERT(server.listen(QHostAddress::Any, ACTIVE_PORT));
        COMMAND("CWD /", "250");
        COMMAND("LIST", "150");
        ASSERT(server.waitForNewConnection(MSEC) && (data = server.nextPendingConnection()));
        str.clear();
        while (!str.contains("testFtp"))
            ASSERT(data->waitForReadyRead(MSEC) && !(str += data->readAll()).isEmpty());
        ASSERT(control.waitForReadyRead(MSEC) && control.readAll().contains("226"));
        ASSERT(data->waitForDisconnected(MSEC));
        server.close();
        delete data;
        // -----
        COMMAND(QString(" EPRT |1||127.0.0.1|%1|").arg(QString::number(ACTIVE_PORT)), "501");
        COMMAND(QString(" EPRT |3|127.0.0.1|%1|").arg(QString::number(ACTIVE_PORT)), "522");
        COMMAND(QString(" EPRT |2|127.0.0.1|%1|").arg(QString::number(ACTIVE_PORT)), "501");
        COMMAND(QString(" EPRT |1|127.0.0.1|%1|").arg(QString::number(ACTIVE_PORT)), "200");
        COMMAND(QString(" EPRT |1|%1|%2|").arg(QHostAddress(QHostAddress::LocalHostIPv6).toString()).arg(QString::number(ACTIVE_PORT)), "501");
        COMMAND(QString(" EPRT |2|%1|%2|").arg(QHostAddress(QHostAddress::LocalHostIPv6).toString()).arg(QString::number(ACTIVE_PORT)), "200");
        ASSERT(server.listen(QHostAddress::AnyIPv6, ACTIVE_PORT));
        COMMAND("CWD /", "250");
        COMMAND("LIST", "150");
        ASSERT(server.waitForNewConnection(MSEC) && (data = server.nextPendingConnection()));
        str.clear();
        while (!str.contains("testFtp"))
            ASSERT(data->waitForReadyRead(MSEC) && !(str += data->readAll()).isEmpty());
        ASSERT(control.waitForReadyRead(MSEC) && control.readAll().contains("226"));
        ASSERT(data->waitForDisconnected(MSEC));
        delete data;
        // -----
        COMMAND("STOR /testFtp/file1.txt", "");
        ASSERT(server.waitForNewConnection(MSEC) && (data = server.nextPendingConnection()));
        ASSERT(data->write("content1") && data->waitForBytesWritten(MSEC));
        delete data;
        str.clear();
        while (!str.contains("250"))
            ASSERT(control.waitForReadyRead(MSEC) && !(str = control.readAll()).isEmpty());
        ASSERT(this->_retr("RETR /testFtp/file1.txt", "content1", server, control));
        COMMAND("STOR /testFtp/file1.txt", "");
        ASSERT(server.waitForNewConnection(MSEC) && (data = server.nextPendingConnection()));
        ASSERT(data->write("content2") && data->waitForBytesWritten(MSEC));
        delete data;
        str.clear();
        while (!str.contains("250"))
            ASSERT(control.waitForReadyRead(MSEC) && !(str = control.readAll()).isEmpty());
        ASSERT(this->_retr("RETR /testFtp/file1.txt", "content2", server, control));
        COMMAND("STOR /testFtp/false/file1.txt", "");
        ASSERT(server.waitForNewConnection(MSEC) && (data = server.nextPendingConnection()));
        ASSERT(data->write("content3") && data->waitForBytesWritten(MSEC));
        delete data;
        str.clear();
        while (!str.contains("501"))
            ASSERT(control.waitForReadyRead(MSEC) && !(str = control.readAll()).isEmpty());
        COMMAND("APPE /testFtp/file1.txt", "");
        ASSERT(server.waitForNewConnection(MSEC) && (data = server.nextPendingConnection()));
        ASSERT(data->write("content3") && data->waitForBytesWritten(MSEC));
        delete data;
        str.clear();
        while (!str.contains("250"))
            ASSERT(control.waitForReadyRead(MSEC) && !(str = control.readAll()).isEmpty());
        ASSERT(this->_retr("RETR /testFtp/file1.txt", "content2content3", server, control));
        COMMAND("STOU /testFtp/file1.txt", "");
        ASSERT(server.waitForNewConnection(MSEC) && (data = server.nextPendingConnection()));
        ASSERT(data->write("content4") && data->waitForBytesWritten(MSEC));
        delete data;
        str.clear();
        while (!str.contains("250"))
            ASSERT(control.waitForReadyRead(MSEC) && !(str += control.readAll()).isEmpty());
        ASSERT(str.contains("150 FILE: /testFtp/file1 - 1.txt"));
        ASSERT(this->_retr("RETR /testFtp/file1 - 1.txt", "content4", server, control));
        // -----
        COMMAND("REST -42", "501");
        COMMAND("REST 4", "350");
        COMMAND("APPE /testFtp/file1.txt", "");
        ASSERT(server.waitForNewConnection(MSEC) && (data = server.nextPendingConnection()));
        ASSERT(data->write("content5") && data->waitForBytesWritten(MSEC));
        delete data;
        str.clear();
        while (!str.contains("503"))
            ASSERT(control.waitForReadyRead(MSEC) && !(str = control.readAll()).isEmpty());
        COMMAND("REST 4", "350");
        COMMAND("STOR /testFtp/file1.txt", "");
        ASSERT(server.waitForNewConnection(MSEC) && (data = server.nextPendingConnection()));
        ASSERT(data->write("content6") && data->waitForBytesWritten(MSEC));
        delete data;
        str.clear();
        while (!str.contains("250"))
            ASSERT(control.waitForReadyRead(MSEC) && !(str = control.readAll()).isEmpty());
        ASSERT(this->_retr("RETR /testFtp/file1.txt", "contcontent6", server, control));
        COMMAND("REST 13", "350");
        COMMAND("STOR /testFtp/file1.txt", "");
        ASSERT(server.waitForNewConnection(MSEC) && (data = server.nextPendingConnection()));
        ASSERT(data->write("content7") && data->waitForBytesWritten(MSEC));
        delete data;
        str.clear();
        while (!str.contains("550"))
            ASSERT(control.waitForReadyRead(MSEC) && !(str = control.readAll()).isEmpty());
        COMMAND("REST 12", "350");
        COMMAND("STOR /testFtp/file1.txt", "");
        ASSERT(server.waitForNewConnection(MSEC) && (data = server.nextPendingConnection()));
        ASSERT(data->write("8") && data->waitForBytesWritten(MSEC));
        delete data;
        str.clear();
        while (!str.contains("250"))
            ASSERT(control.waitForReadyRead(MSEC) && !(str = control.readAll()).isEmpty());
        ASSERT(this->_retr("RETR /testFtp/file1.txt", "contcontent68", server, control));
        COMMAND("APPE /testFtp/file3.txt", "");
        ASSERT(server.waitForNewConnection(MSEC) && (data = server.nextPendingConnection()));
        ASSERT(data->write("content9") && data->waitForBytesWritten(MSEC));
        delete data;
        str.clear();
        while (!str.contains("250"))
            ASSERT(control.waitForReadyRead(MSEC) && !(str = control.readAll()).isEmpty());
        ASSERT(this->_retr("RETR /testFtp/file3.txt", "content9", server, control));
        // -----
        COMMAND("RNFR testFtp/false", "550");
        COMMAND("RNTO testFtp/file1.txt", "503");
        COMMAND("RNFR testFtp/file1.txt", "350");
        COMMAND("RNTO file3.txt", "553");
        COMMAND("RNFR testFtp/d1////./../file1.txt", "350");
        COMMAND("RNTO file2.txt", "250");
        COMMAND("RNFR testFtp/file3.txt", "350");
        COMMAND("RNTO /__testFtpFile3.txt", "250");
        COMMAND("RNFR __testFtpFile3.txt", "350");
        COMMAND("RNTO testFtp/file2.txt", "553");
        COMMAND("RNFR __testFtpFile3.txt", "350");
        COMMAND("RNTO testFtp/d1/file3.txt", "250");
        COMMAND("RNFR testFtp/d1", "350");
        COMMAND("RNTO ..", "553");
        COMMAND("RNFR testFtp/d1", "350");
        COMMAND("RNTO d3", "250");
        COMMAND("RNFR /testFtp/d3", "350");
        COMMAND("RNTO /__testFtp2", "250");
        COMMAND("RNFR /__testFtp2", "350");
        COMMAND("RNTO testFtp/d1", "250");
        COMMAND("RNFR testFtp", "350");
        COMMAND("RNTO   testFtp  ", "250");
        COMMAND("RNFR  testFtp  ", "550");
        COMMAND("RNFR   testFtp ", "550");
        COMMAND("RNFR   testFtp  ", "350");
        COMMAND("RNTO testFtp", "250");
        COMMAND("RNFR testFtp/d1", "350");
        COMMAND("RNTO testFtp/d1/d2/d1", "553");
        COMMAND("RNFR testFtp/d1", "350");
        COMMAND("RNTO testFtp/d1/d1", "553");
        COMMAND("RNFR testFtp/d1", "350");
        COMMAND("RNTO testFtp/d1", "553");
        COMMAND("SIZE testFtp/file2.txt", "213 13");
        COMMAND("SIZE testFtp/false", "550");
        regex.setPattern(QString("213 %1\\d{4}").arg(QDateTime::currentDateTimeUtc().toString("yyyyMMddhh")));
        ASSERT(regex.indexIn(this->_command(control, "MDTM testFtp/file2.txt")) == 0);
        COMMAND("MDTM testFtp/false", "550");
        COMMAND("DELE testFtp/file2.txt", "250");
        COMMAND("DELE testFtp/file2.txt", "550");
        // -----
        COMMAND("CWD /testFtp/d1/d2", "250");
        COMMAND("RMD /testFtp", "250");
        COMMAND("PWD", "257 \"/\"");
        COMMAND("QUIT", "221");
    }
    catch (unsigned int line)
    {
        throw line;
    }
}

bool        Ftp::_command(QTcpSocket &s, const QString &request, const QString &response)
{
    QString result;

    if (s.write(QString(request + "\r\n").toAscii()) == -1 || !s.waitForBytesWritten(MSEC))
        return (false);
    if (!response.isEmpty() && (!s.waitForReadyRead(MSEC) || !(result = s.readAll()).contains(response.toAscii())))
    {
        this->log.error("Test failed", Properties("response", result.trimmed()).toMap(), "Ftp", "_command");
        return (false);
    }
    return (true);
}

QString     Ftp::_command(QTcpSocket &s, const QString &request)
{
    QString result;

    if (s.write(QString(request + "\r\n").toAscii()) > 0 && s.waitForBytesWritten(MSEC) && s.waitForReadyRead(MSEC))
        result = s.readAll();
    return (result);
}

bool           Ftp::_retr(const QString &file, const QString &result, QTcpServer &server, QTcpSocket &control)
{
    QTcpSocket *data;
    QString    str;

    COMMAND(file, "");
    if ((!server.hasPendingConnections() && !server.waitForNewConnection(MSEC)) || !(data = server.nextPendingConnection()))
        return (false);
    while (!str.contains(result))
        if (!data->waitForReadyRead(MSEC) || (str += data->readAll()).isEmpty())
            return (false);
    str.clear();
    while (!str.contains("226"))
        if (!control.waitForReadyRead(MSEC) || (str += control.readAll()).isEmpty())
            return (false);
    delete data;
    return (true);
}

void            Ftp::_data()
{

    this->log.debug("Running the tests...", "Ftp", "_data");
    try
    {
        QTcpSocket c;
        c.connectToHost("ftp.mozilla.org", 21);
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
    catch (unsigned int line)
    {
        this->log.error("Test failed", Properties("line", line).toMap(), "Ftp", "_data");
        throw line;
    }
    this->log.debug("Test successful!", "Ftp", "_data");
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
