#ifndef PORTUDP_H
# define PORTUDP_H

# include <QMultiHash>
# include <QSharedPointer>

# include "ServerUdp.h"
# include "Future.hpp"
# include "WriteBuffer.h"
# include "Port.h"

/// @brief Manages a UDP port of the network. Each port have its own thread.
class PortUdp : public Port
{
    Q_OBJECT

public:
    PortUdp(unsigned short port, const QStringList &protocols, const QStringList &contexts = QStringList(QString()), unsigned int maxClients = ~0);
    ~PortUdp();

    /// @brief Returns true if the port is listening the network.
    inline bool isListening() const { return _serverUdp->isListening(); }
    /// @brief Closes the UDP socket. No new connections will be accepted.
    void    close();

    // IReadWrite
    void    read(Client *client);
    void    write(Client *client, const QByteArray &data);

private slots:
    /// @brief This slot is called when datagrams are ready to be read.
    void    _readPendingDatagrams();
    /// @brief Writes the data that could not be wrote in write().
    void    _write();
    /// @brief Called when a client is finished and should be destroyed.
    void    _finished(Client *client);

private:
    PortUdp(const PortUdp &);
    PortUdp &operator=(const PortUdp &);

    /// @brief The main method of the thread.
    void    run();

    ServerUdp *_serverUdp; ///< The UDP server that listens on the network and waits new datagrams.
    Future<bool> _threadStarted; ///< This future is unlocked when the thread is started.
    QWaitCondition _threadFinished; ///< Allows to wait until all the client are finished before quitting the thread.
    QMultiHash<Client *, QByteArray *> _readBuffer; ///< The list of the datagrams read, waiting to be processed by the Client.
    uint _maxReadBufferSize; ///< The maximum number of datagrams in the read buffer.
    QHash<QSharedPointer<Client>, QSharedPointer<WriteBuffer> > _writeBuffers; ///< Stores the data that could not be written in write().
};

#endif // PORTUDP_H
