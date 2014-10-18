#include "ServerUdp.h"
#ifdef Q_OS_WIN
# include "ServerUdpWindows.h"
#else
# include "ServerUdpLinux.h"
#endif // Q_OS_WIN

ServerUdp *ServerUdp::create(quint16 port, const QHostAddress &address)
{
#ifdef Q_OS_WIN
    return new ServerUdpWindows(port, address);
#else
    return new ServerUdpLinux(port, address);
#endif // Q_OS_WIN
    return NULL;
}

ServerUdp::ServerUdp(quint16 port, const QHostAddress &address)
    : _port(port)
    , _address(address)
    , _listening(false)
{
}

ServerUdp::~ServerUdp()
{
}

