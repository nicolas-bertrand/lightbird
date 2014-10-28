#ifndef SOCKETTCPWINDOWS_H
# define SOCKETTCPWINDOWS_H

#include "SocketTcp.h"

#ifdef Q_OS_WIN
#include <WinSock2.h>
#include <Ws2tcpip.h>

/// @brief Windows implementation of SocketTcp.
class SocketTcpWindows : public SocketTcp
{
public:
    /// @brief Creates a new socket (client mode).
    SocketTcpWindows(const QHostAddress &peerAddress, quint16 peerPort);
    /// @brief Builds the TCP socket around an existing socket (server mode).
    SocketTcpWindows(const QHostAddress &peerAddress, quint16 peerPort, quint16 localPort, SOCKET socket);
    ~SocketTcpWindows();

    // SocketTcp
    qint64 size() const;
    qint64 read(char *data, qint64 size);
    qint64 write(const char *data, qint64 size);
    void writeAgain();
    void close();

    // Emits signals
    void connected(bool success);
    void readyRead();
    void readyWrite();
    void disconnected();

    /// @brief Sets the WSAPoll events used by this socket.
    void setFdEvents(short *events = NULL);

private:
    /// @brief Closes the socket.
    inline void _close();

    sockaddr_in6 _sockaddr;
    short *_events; ///< The WSAPoll events of this socket. Must never point to an invalid address.
    short _noEvents; ///< The address pointed by _events when it is not set.
    bool _disableWriteBuffer; ///< True to disable the write buffer (less CPU overhead, but more protocol overhead).
};

#endif // Q_OS_WIN
#endif // SOCKETTCPWINDOWS_H
