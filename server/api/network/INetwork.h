#ifndef INETWORK_H
# define INETWORK_H

# include <QDateTime>
# include <QHostAddress>
# include <QList>
# include <QMap>
# include <QSharedPointer>
# include <QString>
# include <QStringList>
# include <QVariant>
# include <QVariantMap>

# include "IFuture.h"

namespace LightBird
{
    class IClient;

    /// @brief This class manages the network.
    class INetwork
    {
    public:
        virtual ~INetwork() {}

        /// The available transports protocols.
        enum Transport
        {
            TCP,
            UDP
        };

        /// Stores the informations on a client.
        struct Client
        {
            Transport       transport;        ///< The transport protocol used by the underlaying socket.
            QStringList     protocols;        ///< The names of the protocols used to communicate with the client.
            unsigned short  port;             ///< The local port through which the client is connected.
            int             socketDescriptor; ///< The descriptor of the socket.
            QHostAddress    peerAddress;      ///< The address of the client.
            unsigned short  peerPort;         ///< The peer port through which the client is connected.
            QString         peerName;         ///< The name of the client's host (usually empty).
            QDateTime       connectionDate;   ///< The date of the connection, in local time.
            QString         idAccount;        ///< The id of the account identified.
            QVariantMap     informations;     ///< Information on the client.
        };

        /// @brief Opens a port on the server.
        /// @param port : The number of the port to open.
        /// @param protocols : The names of the protocols used to communicate with
        /// the clients that connect to this port.
        /// @param transport : The transport protocol used by this port.
        /// @param maxClients : The maximum number of clients simultaneously
        /// connected to this port. When the number of client reach this limit, new
        /// connections are waiting that a connected client disconnect.
        /// @return True if the port is listening.
        virtual bool    openPort(unsigned short port, const QStringList &protocols = QStringList(),
                                 LightBird::INetwork::Transport transport = LightBird::INetwork::TCP,
                                 unsigned int maxClients = ~0) = 0;
        /// @brief Closes a port. This may take some time since all the operations
        /// made on the closed port have to be finished.
        /// @param port : The number of the port to close.
        /// @param transport : The transport protocol of the port to close.
        /// @return False if the port is not valid. This method may return before
        /// the port is actually closed.
        virtual bool    closePort(unsigned short port, LightBird::INetwork::Transport transport = LightBird::INetwork::TCP) = 0;
        /// @brief Allows to get informations on an opened port.
        /// @param port : The number of the port to get.
        /// @param protocols : The names of the protocols used by the port.
        /// @param maxClients : The maximum number of clients simultaneously connected,
        /// allowed by the port.
        /// @param transport : The transport protocol of the port to get.
        /// @return True if the port exists.
        virtual bool    getPort(unsigned short port, QStringList &protocols, unsigned int &maxClients,
                                LightBird::INetwork::Transport transport = LightBird::INetwork::TCP) const = 0;
        /// @brief Returns the list of the open ports in TCP or UDP. One can use
        /// getPort() to get more detailed informations about a specific port.
        /// @param transport : The transport protocol of the ports to get.
        /// @return The list of the opened ports on the server.
        virtual QList<unsigned short>   getPorts(LightBird::INetwork::Transport transport = LightBird::INetwork::TCP) const = 0;
        /// @brief Returns the informations of a client.
        /// @param id : The id of the client.
        /// @param client : The informations of the client are stored in this structure.
        /// @return True if the client exists.
        virtual bool    getClient(const QString &id, LightBird::INetwork::Client &client) const = 0;
        /// @brief Returns the list of the clients to which the server is connected.
        /// These clients are not connected to an opened port since the server
        /// initiated the connection.
        /// @return The list of the id of the clients in CLIENT mode.
        virtual QStringList getClients() const = 0;
        /// @brief Returns the list of the clients connected to a particular port.
        /// @param port : The port of the clients.
        /// @param transport : The transport protocol of the port.
        /// @return The list of the id of the clients connected to the port.
        virtual QStringList getClients(unsigned short port, LightBird::INetwork::Transport transport = LightBird::INetwork::TCP) const = 0;
        /// @brief Connects the server to an other server.
        /// IOnConnection is called if the connection was successful.
        /// The client is created using the CLIENT mode which uses a different
        /// Engine than the SERVER mode.
        /// @param address : The address of the peer.
        /// @param port : The port of the peer.
        /// @param protocols : The protocols used to communicate with the client.
        /// If empty the client accepts all the protocols.
        /// @param transport : The transport protocol used to connect to the client.
        /// @param wait : The maximum time allowed for the server to connect to
        /// the host. After this delay, the connection is interrupted, and this
        /// method returns an empty string. This parameter is in milliseconds
        /// and is only used with a TCP connection. If wait is negative, this
        /// function will not time out.
        /// @return The id of the client if the connection successed. Otherwise
        /// it returns an empty string. In UDP this method doesn't check that the
        /// peer actually exists, since there is no connection in UDP.
        /// @see LightBird::IClient::Mode
        virtual QSharedPointer<LightBird::IFuture<QString> > connect(const QHostAddress &address,
                                                                     quint16 port,
                                                                     const QStringList &protocols = QStringList(),
                                                                     LightBird::INetwork::Transport transport = LightBird::INetwork::TCP,
                                                                     int wait = 30000) = 0;
        /// @brief Disconnects the client identified by the id in parameter.
        /// If a request is executing for this client, it is interrupted.
        /// The interface IOnDisconnect is called when the client has been
        /// disconnected.
        /// @param id : The id of the client to disconnect. Nothing appends
        /// if it is already disconnected.
        /// @return True if the client exists.
        virtual bool    disconnect(const QString &id) = 0;
        /// @brief The behaviour of this method depends on the mode of the client.
        /// * In SERVER mode it allows to bypass the deserialization of the requests
        /// and to call directly LightBird::IOnDeserialize followed by IDoExecution,
        /// in order to send a response to the client without waiting for a request.
        /// If is a request is deserializing, false is returned and the server
        /// will wait that the request has been entirely processed before calling
        /// LightBird::IOnDeserialize. If this method is called multiple times,
        /// it will generate as much responses.
        /// * In CLIENT mode it asks the server to call the LightBird::IDoSend
        /// interface in order to generate a request to send. This interface is
        /// called for the plugin that called this method. If other plugins have
        /// called it before, the request is queued until the other requests have
        /// been processed. If a plugin calls this method several times, only one
        /// will be taken into account for the same client.
        /// @param id : The id of the targeted client.
        /// @param protocol : The protocol used to communicate with the client.
        /// If empty the first protocol in the client protocols list (defined in
        /// connect()) is used. Therefore this list must not be empty in this case.
        /// @param informations : This parameter will be stored in the request.
        /// Use LightBird::IRequest::getInformations to access it.
        /// @return False if the client or the protocol is invalid.
        /// * In SERVER mode false is also returned if a request is deserializing.
        /// @see LightBird::IClient::Mode
        /// @see LightBird::IDoSend
        /// @see LightBird::INetwork::connect
        virtual bool    send(const QString &id, const QString &protocol = "", const QVariantMap &informations = QVariantMap()) = 0;
        /// @brief If the client is in CLIENT mode, the server will try to bypass
        /// the serialization of the request, and call directly LightBird::IOnSerialize
        /// followed by IDoDeserializeHeader in order to read the response of
        /// the client without having to send a request. If a request is being
        /// serialized, false is returned and the server will wait that the
        /// request has been entirely processed before calling IOnSerialize.
        /// The same thing happends when no data has been received yet. If this
        /// method is called multiple times, it will read as much responses.
        /// @param id : The id of the targeted client.
        /// @param protocol : The protocol used to communicate with the client.
        /// If empty the first protocol in the client protocols list (defined in
        /// connect()) is used. Therefore this list must not be empty in this case.
        /// @param informations : This parameter will be stored in the request.
        /// Use LightBird::IRequest::getInformations to access it.
        /// @return False if the client or the protocol is invalid, if a response
        /// is being serialized, or if no data have been received yet.
        /// @see LightBird::IClient::Mode
        /// @see LightBird::INetwork::connect
        virtual bool    receive(const QString &id, const QString &protocol = "", const QVariantMap &informations = QVariantMap()) = 0;
    };
}

#endif // INETWORK_H
