#ifndef PORTUDP_H
# define PORTUDP_H

# include <QUdpSocket>

# include "Port.h"

/// @brief Manage a UDP port of the network. It is a specialization
/// of the abstract class Port, which don't care if the port is UDP or TCP.
/// In the UDP implementation, a connection is defined as a peerAddress and
/// peerPort pair. Each time a new pair is discovered, a new client is created.
/// The clients created by UDP are never disconnected alone. It is the role of
/// the plugins to determine when a UDP connection must be closed.
/// @see Port
class PortUdp : public Port
{
    Q_OBJECT

public:
    PortUdp(unsigned short port, const QStringList &protocols, unsigned int maxClients = ~0, QObject *parent = 0);
    ~PortUdp();

    /// @brief Returns true if the socket is currently bound on the port of the network.
    bool    isListening();
    /// @brief Close the UDP socket. No new connections will be accepted after this call.
    void    stopListening();
    bool    read(QByteArray &data, QObject *caller);
    bool    write(QByteArray &data, QObject *caller);

private:
    PortUdp(const PortUdp &);
    PortUdp *operator=(const PortUdp &);

    QUdpSocket  socket; ///< This UDP socket is bound on the port to receive all datagrams sent to it.

private slots:
    /// @brief This slot is called when datagrams are ready to be read.
    void    _readPendingDatagrams();
};

#endif // PORTUDP_H
