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
