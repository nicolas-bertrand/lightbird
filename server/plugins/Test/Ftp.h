#ifndef FTP_H
# define FTP_H

# include <QTcpServer>
# include <QTcpSocket>

# include "ITest.h"

# define MSEC 5000
# define COMMAND(WRITE, READ) ASSERT(_command(control, WRITE, READ));
# define FTP_USER QString("ftpTestAccount")
# define FTP_PASS QString("pass")
# define ACTIVE_PORT 2020

/// @brief Tests the Ftp plugin.
class Ftp : public ITest
{
public:
    Ftp(LightBird::IApi &api);
    ~Ftp();

    unsigned int    run();

private:
    Ftp(const Ftp &);
    Ftp &operator=(const Ftp &);

    void    _tests();
    /// @brief Sends a command and return its result.
    QString _command(QTcpSocket &s, const QString &write);
    /// @brief Sends a FTP command to the server and returns true if the response
    /// is correct.
    bool    _command(QTcpSocket &s, const QString &write, const QString &read);
    /// @brief Executes a RETR command and compare its response with result.
    bool    _retr(const QString &file, const QString &result, QTcpServer &server, QTcpSocket &control);
    void    _data();
    QString _print(QTcpSocket &s, const QString &write);

    unsigned short _port;
};

#endif // FTP_H
