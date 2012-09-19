#ifndef FTP_H
# define FTP_H

# include <QMap>
# include <QString>
# include <QTcpSocket>

# include "Plugin.h"

# define MSEC 5000
# define COMMAND(WRITE, READ) ASSERT(_command(s, WRITE, READ));

/// @brief Tests the Ftp plugin.
class Ftp
{
public:
    Ftp(LightBird::IApi &api);
    ~Ftp();

private:
    Ftp(const Ftp &);
    Ftp &operator=(const Ftp &);

    /// @brief Runs the tests.
    bool    _tests();
    bool    _data();
    /// @brief Sends a FTP command to the server and returns true if the response
    /// is correct.
    bool    _command(QTcpSocket &s, const QString &write, const QString &read);
    QString _print(QTcpSocket &s, const QString &write);

    LightBird::IApi      &api;
    LightBird::IDatabase &database;
    LightBird::ILogs     &log;
    unsigned short       port;
};

#endif // FTP_H
