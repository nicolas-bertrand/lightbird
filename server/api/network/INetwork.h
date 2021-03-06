#ifndef LIGHTBIRD_INETWORK_H
# define LIGHTBIRD_INETWORK_H

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
            Transport       transport;      ///< The transport protocol used by the underlaying socket.
            QStringList     protocols;      ///< The names of the protocols used to communicate with the client.
            unsigned short  localPort;      ///< The local port through which the client is connected.
            int             descriptor;     ///< The descriptor of the socket.
            QHostAddress    peerAddress;    ///< The address of the client.
            unsigned short  peerPort;       ///< The peer port through which the client is connected.
            QString         peerName;       ///< The name of the client's host (usually empty).
            QDateTime       connectionDate; ///< The date of the connection, in local time.
            QString         idAccount;      ///< The id of the account identified.
            QVariantMap     informations;   ///< Information on the client.
        };

        /// @brief Opens a port on the server.
        /// @param port : The number of the port to open.
        /// @param protocols : The names of the protocols used to communicate with
        /// the clients that connect to this port.
        /// @param transport : The transport protocol used by this port.
        /// @param contexts : The default contexts of all the clients connected to this port.
        /// @param maxClients : The maximum number of clients simultaneously
        /// connected to this port. When the number of client reach this limit, new
        /// connections are waiting that a connected client disconnect.
        /// @return True if the port is listening.
        virtual bool    openPort(unsigned short port
            , const QStringList &protocols = QStringList()
            , LightBird::INetwork::Transport transport = LightBird::INetwork::TCP
            , const QStringList &contexts = QStringList(QString())
            , unsigned int maxClients = ~0) = 0;

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
        virtual bool    getPort(unsigned short port
            , QStringList &protocols
            , unsigned int &maxClients
            , LightBird::INetwork::Transport transport = LightBird::INetwork::TCP) const = 0;

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
        /// @param informations : This parameter will be stored in the client.
        /// Use LightBird::IClient::getInformations to access it.
        /// @param contexts : The contexts of the new client, as defined by
        /// LightBird::IClient::getContexts.
        /// @param wait : The maximum time allowed for the server to connect to
        /// the host. After this delay, the connection is interrupted, and this
        /// method returns an empty string. This parameter is in milliseconds
        /// and is only used with a TCP connection. If wait is negative, this
        /// function will not time out.
        /// @return The id of the client if the connection successed. Otherwise
        /// it returns an empty string. In UDP this method doesn't check that the
        /// peer actually exists, since there is no connection in UDP.
        /// @see LightBird::IClient::Mode
        /// @see LightBird::IClient::getContexts
        virtual QSharedPointer<LightBird::IFuture<QString> > connect(const QHostAddress &address
            , quint16 port
            , const QStringList &protocols = QStringList()
            , LightBird::INetwork::Transport transport = LightBird::INetwork::TCP
            , const QVariantMap &informations = QVariantMap()
            , const QStringList &contexts = QStringList(QString())
            , int wait = 30000) = 0;

        /// @brief Disconnects the client identified by the id in parameter.
        /// If data are being processed for this client, it may be interrupted.
        /// The interface IOnDisconnect is called when the client is being
        /// disconnected.
        /// @param id : The id of the client to disconnect. Nothing appends
        /// if it is already disconnected or disconnecting.
        /// @param fatal : If false, the disconnection behavior is based on
        /// the value returned by IOnDisconnect. However if fatal is true,
        /// the client is immediatly disconnected regardless the return value
        /// of IOnDisconnect, and IOnDestroy is called just after it.
        /// @return True if the client exists.
        /// @see LightBird::IOnDisconnect
        virtual bool    disconnect(const QString &id, bool fatal = false) = 0;

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

        /// @brief Allows to pause the network workflow of a client. No more
        /// network interface is called for the client until the resume method
        /// is called, the time has elapsed or the client is disconnected.
        /// In the latter case IOnDisconnect is called, but the workflow will
        /// still be paused until one of the other conditions is met, except if
        /// onDisconnect returned true, in which case IOnResume is called
        /// followed by IOnDestroy.
        /// In any cases the IOnResume interface is called when the pause ends.
        /// The IOnPause interface is called just after this method.
        /// @param id : The id of the client to pause.
        /// @param msec : The maximum duration of the pause in milliseconds.
        /// If the value is negative or zero, the pause will never timeout
        /// (resume must be called).
        /// @see resume
        /// @see LightBird::IOnPause
        /// @see LightBird::IOnResume
        /// @see LightBird::IClient::isPaused
        /// @see LightBird::IOnDisconnect
        /// @return False if the client does not exists or is already paused.
        virtual bool    pause(const QString &id, int msec = -1) = 0;

        /// @brief Resumes the network workflow of a client paused.
        /// The IOnResume interface is called just after this method.
        /// @param id : The id of the client to resume.
        /// @see pause
        /// @see LightBird::IOnPause
        /// @see LightBird::IOnResume
        /// @return False if the client does not exists or is not paused.
        virtual bool    resume(const QString &id) = 0;

        /// @brief Sets the number of milliseconds of inactivity after which the
        /// client will be automatically disconnected.
        /// @param id : The id of the client.
        /// @param msec : If msec is negative, disconnect idle is cleared and the
        /// client will never be disconnected (which is the default behavior).
        /// @param fatal : See the fatal behavior of IClient::disconnect.
        virtual void    setDisconnectIdle(const QString &id, qint64 msec = -1, bool fatal = false) = 0;
        /// @brief Returns the number of milliseconds of inactivity after which
        /// the client will be disconnected. A negative value means that the
        /// client will never be disconnected, which is the default behavior.
        /// @param fatal : If used, allows to know if the disconnection will be fatal.
        virtual qint64  getDisconnectIdle(const QString &id, bool *fatal = NULL) = 0;

        /// @brief Sets the time at which the client will be automatically
        /// disconnected no matter what.
        /// @param id : The id of the client.
        /// @param time : The date at which the client will be disconnected.
        /// If the QDateTime is not valid, the disconnect time is cleared and
        /// the client will never be disconnected (which is the default behavior).
        /// @param fatal : See the fatal behavior of IClient::disconnect.
        virtual void    setDisconnectTime(const QString &id, const QDateTime &time = QDateTime(), bool fatal = false) = 0;
        /// @brief Sets the time at which the client will be automatically
        /// disconnected no matter what.
        /// @param msec : The number of milliseconds from now after which the
        /// client will be disconnected.
        /// If msec is negative, the disconnect time is cleared and the
        /// client will never be disconnected (which is the default behavior).
        /// @param fatal : See the fatal behavior of IClient::disconnect.
        inline void     setDisconnectTime(const QString &id, qint64 msec = -1, bool fatal = false) { setDisconnectTime(id, (msec >= 0 ? QDateTime::currentDateTime().addMSecs(msec) : QDateTime()), fatal); }
        /// @brief Returns the time at which the client will be disconnected.
        /// A null QDateTime means that the client will never be disconnected.
        /// @param fatal : If used, allows to know if the disconnection will be fatal.
        virtual QDateTime getDisconnectTime(const QString &id, bool *fatal = NULL) = 0;
    };
}

#endif // LIGHTBIRD_INETWORK_H
