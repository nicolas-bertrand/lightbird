#ifndef CLIENTSNETWORKWINDOWS_H
# define CLIENTSNETWORKWINDOWS_H

#include "ClientsNetwork.h"
#include "SocketTcpWindows.h"

#ifdef Q_OS_WIN
#include <winsock2.h>

/// @brief Windows implementation of ClientsNetwork.
class ClientsNetworkWindows : public ClientsNetwork
{
public:
    ClientsNetworkWindows();
    ~ClientsNetworkWindows();

    /// @brief This method only returns when the Clients is closed.
    void execute();
    /// @brief Returns a socket to manage.
    void addSocket(QSharedPointer<Socket> socket, int wait = -1);
    /// @brief Closes all the clients, making execute() return.
    void close();

private:
    /// @brief A client has been disconnected.
    void _disconnect(SocketTcpWindows *socketTcp, int &i);
    /// @brief Disconnects all the sockets.
    void _disconnectAll();
    /// @brief Creates the client and server sockets that allow to interrupt WSAPoll.
    bool _createInterruptSockets();
    /// @brief Interrupts WSAPoll.
    void _interruptPoll();

    bool _disableWriteBuffer; ///< True to disable the write buffer (less CPU overhead, but more protocol overhead).
    int _wsaPollTimeout; ///< The timeout of the WSAPoll.
    QVector<WSAPOLLFD> _fds; ///< The FD used by WSAPoll.
    QList<QSharedPointer<Socket> > _sockets; ///< The list of the sockets connected to the clients.
    QList<int> _connections; ///< The list of the connecting sockets.

    /// @brief Allows to interrupt WSAPoll by writing a UDP datagram.
    /// @see _interruptPoll
    struct Interrupt
    {
        SOCKET serverSocket;
        SOCKET clientSocket;
        sockaddr_in6 serverAddr;
    };
    Interrupt _interrupt;
};

#endif // Q_OS_WIN
#endif // CLIENTSNETWORKWINDOWS_H
