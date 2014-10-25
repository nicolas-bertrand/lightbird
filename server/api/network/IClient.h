#ifndef LIGHTBIRD_ICLIENT_H
# define LIGHTBIRD_ICLIENT_H

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
# include "ISocket.h"

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
        virtual LightBird::ISocket   &getSocket() = 0;
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
        /// @brief Returns the number of bytes that are waiting to be deserialized.
        /// It is the addition of the data already read from the network but not
        /// yet deserialized, and the data waiting to be read on the socket.
        virtual quint64              getBufferSize() const = 0;
        /// @brief The connection mode of the client.
        virtual LightBird::IClient::Mode getMode() const = 0;
        /// @brief Returns the names of the contexts currently set for this
        /// client. All the network interfaces of the contexts in this list
        /// can be potentially called.
        /// The default context (the class that inherits LightBird::IPlugin) is
        /// called if this list is empty, or there is an empty string in it.
        /// By default the contexts list contains an empty string, so that the
        /// default context of the plugins is always called.
        /// Notice that changing the contexts does not affect the current
        /// interface context since it has already been evaluated. The changes
        /// will apply to the next interface.
        /// @see LightBird::IContexts
        virtual QStringList          &getContexts() = 0;
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

        /// @brief Allows to pause the network workflow of the client. No more
        /// network interface is called for the client until the resume method
        /// is called, the time has elapsed or the client is disconnected.
        /// In the latter case IOnDisconnect is called, but the workflow will
        /// still be paused until one of the other conditions is met, except if
        /// onDisconnect returned true, in which case IOnResume is called
        /// followed by IOnDestroy.
        /// In any cases the IOnResume interface is called when the pause ends.
        /// The IOnPause interface is called just after this method.
        /// @param msec : The maximum duration of the pause in milliseconds.
        /// If the value is negative or zero, the pause will never timeout
        /// (resume must be called).
        /// @see resume
        /// @see LightBird::IOnPause
        /// @see LightBird::IOnResume
        /// @see LightBird::IClient::isPaused
        /// @see LightBird::IOnDisconnect
        /// @return False if the client does not exists or is already paused.
        virtual bool                 pause(int msec = -1) = 0;
        /// @brief Resumes the network workflow of the paused client.
        /// The IOnResume interface is called just after this method.
        /// @see pause
        /// @see LightBird::IOnPause
        /// @see LightBird::IOnResume
        /// @return False if the client does not exists or is not paused.
        virtual bool                 resume() = 0;
        /// @brief Returns true if the client has been paused using INetwork::pause.
        /// @see LightBird::INetwork::pause
        virtual bool                 isPaused() const = 0;

        /// @brief Disconnects the client.
        /// If data are being processed for this client, it may be interrupted.
        /// The interface IOnDisconnect is called when the client is being
        /// disconnected.
        /// @param fatal : If false, the disconnection behavior is based on
        /// the value returned by IOnDisconnect. However if fatal is true,
        /// the client is immediatly disconnected regardless the return value
        /// of IOnDisconnect, and IOnDestroy is called just after it.
        /// @see LightBird::IOnDisconnect
        virtual void                 disconnect(bool fatal = false) = 0;
        /// @brief Returns true if the client is disconnecting. This occurs when
        /// a client is disconnected but false has been returned on a call to
        /// LightBird::IOnDisconnect. As a result the client is not destroyed
        /// immediatly, and it is marked as disconnecting in order to allow its
        /// remaining data to be processed normally.
        /// @see LightBird::IOnDisconnect
        virtual bool                 isDisconnecting() const = 0;
        /// @brief Sets the number of milliseconds of inactivity after which the
        /// client will be automatically disconnected.
        /// @param msec : If msec is negative, disconnect idle is cleared and the
        /// client will never be disconnected (which is the default behavior).
        /// @param fatal : See the fatal behavior of IClient::disconnect.
        virtual void                 setDisconnectIdle(qint64 msec = -1, bool fatal = false) = 0;
        /// @brief Returns the number of milliseconds of inactivity after which
        /// the client will be disconnected. A negative value means that the
        /// client will never be disconnected, which is the default behavior.
        /// @param fatal : If used, allows to know if the disconnection will be fatal.
        virtual qint64               getDisconnectIdle(bool *fatal = NULL) const = 0;
        /// @brief Sets the time at which the client will be automatically
        /// disconnected no matter what.
        /// @param time : The date at which the client will be disconnected.
        /// If the QDateTime is not valid, the disconnect time is cleared
        /// and the client will never be disconnected (which is the default behavior).
        /// @param fatal : See the fatal behavior of IClient::disconnect.
        virtual void                 setDisconnectTime(const QDateTime &time = QDateTime(), bool fatal = false) = 0;
        /// @brief Sets the time at which the client will be automatically
        /// disconnected no matter what.
        /// @param msec : The number of milliseconds from now after which the
        /// client will be disconnected.
        /// If the msec is negative, the disconnect time is cleared and the
        /// client will never be disconnected (which is the default behavior).
        /// @param fatal : See the fatal behavior of IClient::disconnect.
        inline void                  setDisconnectTime(qint64 msec = -1, bool fatal = false) { setDisconnectTime((msec >= 0 ? QDateTime::currentDateTime().addMSecs(msec) : QDateTime()), fatal); }
        /// @brief Returns the time at which the client will be disconnected.
        /// A null QDateTime means that the client will never be disconnected.
        /// @param fatal : If used, allows to know if the disconnection will be fatal.
        virtual const QDateTime      &getDisconnectTime(bool *fatal = NULL) const = 0;
    };
}

#endif // LIGHTBIRD_ICLIENT_H
