#ifndef CLIENTSNETWORKWINDOWS_H
# define CLIENTSNETWORKWINDOWS_H

#include "ClientsNetwork.h"
#include "SocketTcpWindows.h"
#include "SocketUdpWindows.h"

#ifdef Q_OS_WIN
#include <winsock2.h>

/// @brief Windows implementation of ClientsNetwork.
/// This class is thread safe as long as execute() is called once.
class ClientsNetworkWindows : public ClientsNetwork
{
public:
    ClientsNetworkWindows();
    ~ClientsNetworkWindows();

    /// @brief This method only returns when the Clients is closed.
    void execute();
    /// @brief Adds a socket to manage.
    void addSocket(QSharedPointer<Socket> socket, int wait = -1);
    /// @brief Closes all the clients, making execute() return.
    void close();

private:
    /// @brief Adds a socket to manage.
    void _addSockets();
    /// @brief The connection to the socket is done (success or not).
    void _connected(SocketTcpWindows *socketTcp, int i);
    /// @brief The connection to the socket failed.
    void _connectionFailed(int i);
    /// @brief A client has been disconnected.
    void _disconnect(SocketTcpWindows *socketTcp, int &i);
    void _disconnect(SocketUdpWindows *socketUdp, int &i);
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
    QMap<qint64, int> _connections; ///< The list of the connecting sockets, sorted by their timeout.
    QList<QPair<QSharedPointer<Socket>, int> > _socketsToAdd; ///< The list of the sockets so add. This allows addSocket() to be thread safe.
    QMutex _socketsToAddMutex; ///< Allows _socketsToAdd to be thread safe.

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
