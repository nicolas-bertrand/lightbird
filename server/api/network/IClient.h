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
# include "IRequest.h"
# include "IResponse.h"
# include "ISessions.h"

# include "TableAccounts.h"

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
        virtual QAbstractSocket      &getSocket() = 0;
        /// @brief The local port from which the client is connected.
        virtual unsigned short       getPort() const = 0;
        /// @brief The name of the protocols used to communicate with the client.
        virtual const QStringList    &getProtocols() const = 0;
        /// @brief The transport protocol used by the connection.
        virtual LightBird::INetwork::Transport getTransport() const = 0;
        /// @brief The address of the client.
        virtual const QHostAddress   &getPeerAddress() const = 0;
        /// @brief The port of the host of the client.
        virtual unsigned short       getPeerPort() const = 0;
        /// @brief The name of the host of the client (usually empty).
        virtual const QString        &getPeerName() const = 0;
        /// @brief The date of the connection of the client, in local time.
        virtual const QDateTime      &getConnectionDate() const = 0;
        /// @brief The connection mode of the client.
        virtual LightBird::IClient::Mode getMode() const = 0;
        /// @brief Returns the amount of data that is waiting to be deserialized.
        /// It is the addition of the data already read from the network but not
        /// yet deserialized, and the data waiting to be read on the socket.
        virtual quint64              getBufferSize() const = 0;
        /// @brief Allows to store informations on the client.
        virtual QVariantMap          &getInformations() = 0;
        /// @brief This account is used to identified the client.
        virtual LightBird::TableAccounts &getAccount() = 0;
        /// @brief The request sent by the client.
        virtual LightBird::IRequest  &getRequest() = 0;
        /// @brief The response of the request made by the client.
        virtual LightBird::IResponse &getResponse() = 0;
        /// @brief The sessions to which the client is associated.
        /// @param id_account : The id of the account of the session to get.
        /// Ignored by default.
        virtual QStringList          getSessions(const QString &id_account = QString()) const = 0;
        /// @brief Returns the session of the client. A convenient method when
        /// there is only one session per client. A null session can be returned.
        /// @param id_account : The id of the account of the session to get.
        /// Ignored by default.
        virtual LightBird::Session   getSession(const QString &id_account = QString()) const = 0;
        /// @brief Returns true if the client is disconnecting. This occurs when
        /// a client is disconnected but false has been returned on a call to
        /// LightBird::IOnDisconnect. As a result the client is not destroyed
        /// immediatly, and it is marked as disconnecting in order to allow its
        /// remaining data to be processed normally.
        /// @see LightBird::IOnDisconnect
        virtual bool                 isDisconnecting() const = 0;
    };
}

#endif // ICLIENT_H
