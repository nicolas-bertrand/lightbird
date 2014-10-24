#include "ServerTcp.h"
#ifdef Q_OS_WIN
# include "ServerTcpWindows.h"
#elif defined Q_OS_LINUX
# include "ServerTcpLinux.h"
#endif // Q_OS_WIN

ServerTcp *ServerTcp::create(quint16 port, const QHostAddress &address)
{
#ifdef Q_OS_WIN
    return new ServerTcpWindows(port, address);
#elif defined Q_OS_LINUX
    return new ServerTcpLinux(port, address);
#endif // Q_OS_WIN
    return NULL;
}

ServerTcp::ServerTcp(quint16 port, const QHostAddress &address)
    : _port(port)
    , _address(address)
    , _listening(false)
{
}

ServerTcp::~ServerTcp()
{
}

QSharedPointer<Socket> ServerTcp::nextPendingConnection()
{
    QSharedPointer<Socket> socket;

    if (!_pendingConnections.isEmpty())
    {
        socket = _pendingConnections.first();
        _pendingConnections.pop_front();
    }
    return socket;
}
