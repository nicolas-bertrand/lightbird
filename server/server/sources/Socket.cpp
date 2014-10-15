#include "Socket.h"

Socket::Socket(LightBird::INetwork::Transport transport, const QHostAddress &peerAddress, quint16 peerPort, quint16 localPort, qintptr socket)
    : _socket(socket)
    , _descriptor(socket)
    , _connected(true)
    , _peerAddress(peerAddress)
    , _peerPort(peerPort)
    , _localPort(localPort)
    , _transport(transport)
{
}

Socket::~Socket()
{
}
