#include "SocketTcp.h"
#ifdef Q_OS_WIN
# include "SocketTcpWindows.h"
#elif defined Q_OS_LINUX
# include "SocketTcpLinux.h"
#endif // Q_OS_WIN

SocketTcp *SocketTcp::create(const QHostAddress &peerAddress, quint16 peerPort)
{
#ifdef Q_OS_WIN
    return new SocketTcpWindows(peerAddress, peerPort);
#elif defined Q_OS_LINUX
    return new SocketTcpLinux(peerAddress, peerPort);
#endif // Q_OS_WIN
    return NULL;
}

SocketTcp::SocketTcp(const QHostAddress &peerAddress, quint16 peerPort, quint16 localPort, qintptr socket)
    : Socket(LightBird::INetwork::TCP, peerAddress, peerPort, localPort, socket)
    , _connecting(false)
{
}

SocketTcp::~SocketTcp()
{
}
