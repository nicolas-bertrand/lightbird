#ifndef PORTTCP_H
# define PORTTCP_H

# include <QTcpServer>

# include "Port.h"

/**
 * @brief Manage a TCP port of the network. It is a specialization
 * of the abstract class Port, which don't care if the port is UDP or TCP.
 * The TCP implementation use the QTcpServer to detect new connection,
 * and accept them. One QTcpSocket is created by connection, and is attached
 * to a Client. The socket is destroyed with the Client.
 * @see Port
 */
class PortTcp : public Port
{
    Q_OBJECT

public:
    PortTcp(unsigned short port, const QStringList &protocols, unsigned int maxClients = ~0, QObject *parent = 0);
    ~PortTcp();

    /// @brief Returns true if the port is currently listening the network.
    bool    isListening();
    /// @brief Close the TCP server No new connections will be accepted after this call.
    void    stopListening();
    bool    read(QByteArray &data, QObject *caller);
    bool    write(QByteArray &data, QObject *caller);

private:
    PortTcp(const PortTcp &);
    PortTcp *operator=(const PortTcp &);

    QTcpServer                          tcpServer;  ///< The TCP server, that listen on the network, and waiting new connections.
    QMap<QAbstractSocket *, Client *>   sockets;    ///< Associates the socket with its client.

private slots:
    /// @brief This slot is called when a new connection is pending on the port of the tcpServer.
    void    _newConnection();
    /// @brief Called when a QTcpSocket is disconnected, to removes its client.
    void    _disconnected();
    /// @brief Called when a Client's thread is finished. Used to destroy its QTcpSocket.
    Client  *_finished();
};

#endif // PORTTCP_H
