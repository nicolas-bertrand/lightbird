#ifndef INetwork_H
# define INetwork_H

# include <QDateTime>
# include <QHostAddress>
# include <QList>
# include <QMap>
# include <QSharedPointer>
# include <QString>
# include <QStringList>
# include <QVariant>

# include "IFuture.h"

namespace LightBird
{
    class IClient;

    /// @brief This class allows to do several actions related to the network.
    class INetwork
    {
    public:
        virtual ~INetwork() {}

        /// List the available transports protocols.
        enum Transports
        {
            TCP,
            UDP
        };

        /// Stores the informations on a client.
        struct Client
        {
            Transports              transport;          ///< The transport protocol used by the underlaying socket.
            QStringList             protocols;          ///< The names of the protocols used to communicate with the client.
            unsigned short          port;               ///< The local port through which the client is connected.
            int                     socketDescriptor;   ///< The descriptor of the socket.
            QHostAddress            peerAddress;        ///< The address of the client.
            unsigned short          peerPort;           ///< The peer port through which the client id connected.
            QString                 peerName;           ///< The name of the client's host (usually empty).
            QDateTime               connectionDate;     ///< The date of the creation of this object.
            QString                 idAccount;          ///< The id of the account identified.
            QMap<QString, QVariant> informations;       ///< Information on the client.
        };

        /// @brief Open a new port on the server. Since the port is opened in
        /// a separate thread, users should use the IFuture returned to be aware
        /// when the port is actualy listening.
        /// @param port : The port to open.
        /// @param protocol : The name of the protocol used to communicate with
        /// the clients that connect to this port.
        /// @param transport : The transport protocol used by this port.
        /// @param maxClients : The maximum number of clients simultaneously
        /// connected to this port. When the number of client reach this limit, new
        /// connections are waiting that a connected client disconnect.
        /// @return The future result of the action, e.g true if the port has been created.
        virtual QSharedPointer<LightBird::IFuture<bool> >   addPort(unsigned short port, const QStringList &protocols = QStringList(),
                                                            LightBird::INetwork::Transports transport = LightBird::INetwork::TCP,
                                                            unsigned int maxClients = ~0) = 0;
        /// @brief Removes a port. This may take some time since all the operations
        /// made on the removed port have to be finished.
        /// @param port : The port to remove.
        /// @return The future result of the action, e.g false if the port is not valid.
        virtual QSharedPointer<LightBird::IFuture<bool> >   removePort(unsigned short port) = 0;
        /// @brief Allows to get informations on an opened port.
        /// @param port : The port to get.
        /// @param protocols : The name of the protocols used by the port.
        /// @param transport : The transport protocol of the port.
        /// @param maxClients : The maximum number of clients simultaneously connected,
        /// allowed by the port.
        /// @return True if the port exists.
        virtual bool    getPort(unsigned short port, QStringList &protocols, LightBird::INetwork::Transports &transport,
                                unsigned int &maxClients) = 0;
        /// @brief Returns the list of the open ports. Users can use getPort() to get
        /// more detailed informations about a specific port.
        /// @return The list of the opened ports on the server.
        virtual QList<unsigned short>   getPorts() = 0;
        /// @brief Returns the informations of a client.
        /// @param id : The id of the client.
        /// @param client : The informations of the client are stored in this structure.
        /// @return True if the client exists.
        ///
        virtual bool    getClient(const QString &id, LightBird::INetwork::Client &client) = 0;
        /// @brief Allows to get the id of the connected clients for a particular port.
        /// @param port : The port of the clients.
        /// @return The list of the id of the clients connected to the port.
        virtual QStringList getClients(unsigned short port) = 0;
        /// @brief Disconnects the client identified by the id in parameter.
        /// If a request is executing for this client, it is interrupted.
        /// The interface IOnDisconnect is called when the client has been
        /// disconnected.
        /// @param id : The id of the client to disconnect. Nothing appends
        /// if it is already disconnected.
        /// @return True if the client exists.
        virtual QSharedPointer<LightBird::IFuture<bool> >   disconnect(const QString &id) = 0;
    };
}

#endif // INetwork_H
