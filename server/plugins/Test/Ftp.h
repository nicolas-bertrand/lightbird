#ifndef FTP_H
# define FTP_H

# include <QTcpSocket>

# include "ITest.h"

# define MSEC 5000
# define COMMAND(WRITE, READ) ASSERT(_command(s, WRITE, READ));

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
    /// @brief Sends a FTP command to the server and returns true if the response
    /// is correct.
    bool    _command(QTcpSocket &s, const QString &write, const QString &read);
    void    _data();
    QString _print(QTcpSocket &s, const QString &write);

    unsigned short  port;
};

#endif // FTP_H
