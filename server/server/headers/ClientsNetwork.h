#ifndef CLIENTSNETWORK_H
# define CLIENTSNETWORK_H

#include <QObject>
#include <QSharedPointer>

#include "SocketTcp.h"
#include "Initialize.h"

/// @brief Manages the client sockets.
class ClientsNetwork
    : public QObject
    , public LightBird::Initialize
{
    Q_OBJECT

public:
    /// @brief Creates an instance of ClientsNetwork for the current platform.
    /// NULL is returned if no implementation is available for the current platform.
    static ClientsNetwork *create();

    ClientsNetwork();
    virtual ~ClientsNetwork();

    /// @brief Blocks until the instance is closed.
    virtual void execute() = 0;
    /// @brief Adds a socket to manage.
    virtual void addSocket(QSharedPointer<Socket> socket, int wait = -1) = 0;
    /// @brief Closes the all the clients.
    virtual void close() = 0;

    inline bool isListening() { return _listening; }

protected:
    bool _listening; ///< True while the Clients is listenning the network in execute().
};

#endif // CLIENTSNETWORK_H
