#ifndef ICLIENT_H
# define ICLIENT_H

# include <QAbstractSocket>
# include <QDateTime>
# include <QHostAddress>
# include <QMap>
# include <QString>
# include <QStringList>
# include <QVariant>

# include "INetwork.h"
# include "ITableAccounts.h"
# include "IRequest.h"
# include "IResponse.h"

namespace LightBird
{
    /// @brief Each instances of this interface represents a connected client.
    /// It stores all its informations.
    class IClient
    {
    public:
        virtual ~IClient() {}

        /// @brief The mode defines how a client connects to the server.
        /// In the SERVER mode, clients connect to the server via an opened port.
        /// This is the default mode since LightBird is a server before everything else.
        /// In the CLIENT mode, the server initiates the connection to the clients.
        /// It can be used to exchange data with other servers.
        /// These two modes use different Engines to communicate with the client.
        /// @see LightBird::INetwork::connect
        enum Mode
        {
            CLIENT,
            SERVER
        };

        /// @brief Returns the client id. Each clients has a unique id.
        virtual const QString        &getId() const = 0;
        /// @brief The socket through which the client is connected.
        virtual QAbstractSocket      &getSocket() const = 0;
        /// @brief The local port from which the client is connected.
        virtual unsigned short       getPort() const = 0;
        /// @brief The names of the protocols used to communicate with the client.
        virtual const QStringList    &getProtocols() const = 0;
        /// @brief The transport protocol used by the connection.
        virtual LightBird::INetwork::Transports getTransport() const = 0;
        /// @brief The address of the client.
        virtual const QHostAddress   &getPeerAddress() const = 0;
        /// @brief The port of the host of the client.
        virtual unsigned short       getPeerPort() const = 0;
        /// @brief The name of the host of the client (usually empty).
        virtual const QString        &getPeerName() const = 0;
        /// @brief The date of the connection of the client.
        virtual const QDateTime      &getConnectionDate() const = 0;
        /// @brief The connection mode of the client.
        virtual LightBird::IClient::Mode getMode() const = 0;
        /// @brief Allows to store informations on the client.
        virtual QMap<QString, QVariant> &getInformations() = 0;
        /// @brief This account is used to identified the client.
        virtual LightBird::ITableAccounts &getAccount() = 0;
        /// @brief The request sent by the client.
        virtual LightBird::IRequest  &getRequest() = 0;
        /// @brief The response of the request made by the client.
        virtual LightBird::IResponse &getResponse() = 0;
    };
}

#endif // ICLIENT_H