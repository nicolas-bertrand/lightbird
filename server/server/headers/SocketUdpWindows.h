#ifndef SOCKETUDPWINDOWS_H
# define SOCKETUDPWINDOWS_H

#include "SocketUdp.h"

#ifdef Q_OS_WIN
#include <WinSock2.h>
#include <Ws2tcpip.h>

/// @brief Windows implementation of SocketUdp.
class SocketUdpWindows : public SocketUdp
{
public:
    /// @brief Creates a new socket (client mode).
    SocketUdpWindows(const QHostAddress &peerAddress, quint16 peerPort);
    /// @brief Builds the UDP socket around an existing socket (server mode).
    SocketUdpWindows(quint16 localPort, qintptr socket);
    ~SocketUdpWindows();

    // SocketUdp
    qint64 size() const;
    qint64 read(char *data, qint64 size);
    qint64 write(const char *data, qint64 size);
    void writeAgain();
    void close();

    // Emits signals
    void readyRead();
    void readyWrite();
    void disconnected();

    /// @brief Sets the WSAPoll events used by this socket.
    void setFdEvents(short *events = NULL);

private:
    /// @brief Closes the socket.
    inline void _close();

    sockaddr *_peerAddress; ///< The address of the peer the socket is connected to. The actual format can be sockaddr_in or sockaddr_in6.
    sockaddr *_peerAddressRecv; ///< Stores the address of the peer of the last datagram received. The actual format can be sockaddr_in or sockaddr_in6.
    int _peerAddressSize; ///< The size of the peer address (depends on IPv4 or IPv6).
    short *_events; ///< The WSAPoll events of this socket. Must never point to an invalid address.
    short _noEvents; ///< The address pointed by _events when it is not set.
    bool _disableWriteBuffer; ///< True to disable the write buffer (less CPU overhead, but more protocol overhead).
};

#endif // Q_OS_WIN
#endif // SOCKETUDPWINDOWS_H
