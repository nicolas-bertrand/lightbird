#include "SocketUdp.h"
#ifdef Q_OS_WIN
# include "SocketUdpWindows.h"
#elif defined Q_OS_LINUX
# include "SocketUdpLinux.h"
#endif // Q_OS_WIN

SocketUdp *SocketUdp::create(const QHostAddress &peerAddress, quint16 peerPort)
{
#ifdef Q_OS_WIN
    return new SocketUdpWindows(peerAddress, peerPort);
#elif defined Q_OS_LINUX
    return new SocketUdpLinux(peerAddress, peerPort);
#endif // Q_OS_WIN
    return NULL;
}

SocketUdp::SocketUdp(const QHostAddress &peerAddress, quint16 peerPort, quint16 localPort, qintptr socket)
    : Socket(LightBird::INetwork::UDP, peerAddress, peerPort, localPort, socket)
{
}

SocketUdp::~SocketUdp()
{
}
