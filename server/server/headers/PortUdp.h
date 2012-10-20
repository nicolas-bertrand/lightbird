#ifndef PORTUDP_H
# define PORTUDP_H

# include <QMultiHash>
# include <QUdpSocket>

# include "Future.hpp"
# include "Port.h"

/// @brief Manages a UDP port of the network.
class PortUdp : public Port
{
    Q_OBJECT

public:
    PortUdp(unsigned short port, const QStringList &protocols, unsigned int maxClients = ~0);
    ~PortUdp();

    /// @brief Closes the UDP socket. No new connections will be accepted.
    void    close();

    // IReadWrite
    void    read(Client *client);
    bool    write(QByteArray *data, Client *client);

private slots:
    /// @brief This slot is called when datagrams are ready to be read.
    void    _readPendingDatagrams();
    /// @brief Called when a client is finished.
    Client  *_finished(Client *client = NULL);

private:
    PortUdp(const PortUdp &);
    PortUdp &operator=(const PortUdp &);

    /// @brief The main method of the thread.
    void    run();
    /// @brief Returns true if the socket is currently bound on the port of the network.
    bool    _isListening() const;

    QUdpSocket   socket;        ///< This UDP socket is bound on the port to receive all datagrams sent to it.
    Future<bool> threadStarted; ///< This future is unlocked when the thread is started.
    QMultiHash<Client *, QByteArray *> readBuffer; ///< The list of the datagrams read, waiting to be processed by the Client.
};

#endif // PORTUDP_H
