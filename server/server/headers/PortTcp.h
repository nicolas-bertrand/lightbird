#ifndef PORTTCP_H
# define PORTTCP_H

# include <QSharedPointer>

# include "ServerTcp.h"
# include "Future.hpp"
# include "Port.h"
# include "WriteBuffer.h"

/// @brief Manages a TCP port of the network. Each port have its own thread.
class PortTcp : public Port
{
    Q_OBJECT

public:
    PortTcp(unsigned short port, const QStringList &protocols, unsigned int maxClients = ~0);
    ~PortTcp();

    /// @brief Returns true if the port is listening the network.
    inline bool isListening() const { return _serverTcp->isListening(); }
    /// @brief Closes the TCP server. No new connections will be accepted.
    void    close();

    // IReadWrite
    void    read(Client *client);
    void    write(Client *client, const QByteArray &data);

private slots:
    /// @brief This slot is called when a new connection is pending on the port of the serverTcp.
    void    _newConnection();
    /// @brief Writes the data that could not be wrote in write().
    void    _write();
    /// @brief Called when a socket is disconnected.
    void    _disconnected(Socket *socket);
    /// @brief Called when a client is finished and should be destroyed.
    void    _finished(Client *client);

private:
    PortTcp(const PortTcp &);
    PortTcp &operator=(const PortTcp &);

    /// @brief The main method of the thread.
    void    run();

    ServerTcp *_serverTcp; ///< The TCP server that listens on the network and waits new connections.
    Future<bool> _threadStarted; ///< This future is unlocked when the thread is started.
    QWaitCondition _threadFinished; ///< Allows to wait until all the client are finished before quitting the thread.
    QHash<QSharedPointer<Client>, QSharedPointer<WriteBuffer> > _writeBuffers; ///< Stores the data that could not be written in write().
};

#endif // PORTTCP_H
