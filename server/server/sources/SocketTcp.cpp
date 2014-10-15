#include "SocketTcp.h"
#ifdef Q_OS_WIN
# include "SocketTcpWindows.h"
#else
# include "SocketTcpLinux.h"
#endif // Q_OS_WIN

SocketTcp *SocketTcp::create(const QHostAddress &peerAddress, quint16 peerPort)
{
#ifdef Q_OS_WIN
    return new SocketTcpWindows(peerAddress, peerPort);
#else
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
