#ifndef CLIENT_H
# define CLIENT_H

# include <QMutex>
# include <QThread>

# include "IClient.h"
# include "INetwork.h"
# include "IReadWrite.h"

# include "Future.hpp"
# include "TableAccounts.h"

class Engine;

/// @brief Represents a connected client.
/// All its requests are processed from a thread based on this class.
class Client : public QThread,
               public LightBird::IClient
{
    Q_OBJECT

public:
    /// @brief Stores the socket informations, and creates the client's thread.
    /// @param socket : The socket from which the client is connected.
    /// @param protocols : The protocols used to communicate with the client.
    /// @param transport : The transport protocol used by this client.
    /// @param port : The local port from which the client is connected.
    /// @param socketDescriptor : The descriptor of the socket used by the client.
    /// @param peerAddress : The address of the client.
    /// @param peerPort : The port from which the client is connected in his host.
    /// @param peerName : The name of the client's host. May be empty.
    /// @param mode : The connection mode of the client.
    Client(QAbstractSocket *socket, LightBird::INetwork::Transport transport, const QStringList &protocols,
           unsigned short port, int socketDescriptor, const QHostAddress &peerAddress,
           unsigned short peerPort, const QString &peerName, LightBird::IClient::Mode mode,
           IReadWrite *readWriteInterface);
    ~Client();

    /// @brief The main method of the Client's thread.
    /// Contains the thread event loop.
    void                    run();
    /// @brief Write the given data to the client.
    void                    write(QByteArray &data);
    /// @brief Calls the IDoRead interface of the plugins.
    bool                    doRead(QByteArray &data);
    /// @brief Calls the IDoWrite interface of the plugins.
    bool                    doWrite(QByteArray &data);
    /// @brief Asks the engine to call IDoSend in order to generate a request
    /// that will be send to a client.
    /// @param id : The id of the plugin that will be called by IDoSend.
    void                    send(const QString &id, const QString &protocol);

    const QString           &getId() const;
    unsigned short          getPort() const;
    const QStringList       &getProtocols() const;
    LightBird::INetwork::Transport getTransport() const;
    int                     getSocketDescriptor() const;
    const QHostAddress      &getPeerAddress() const;
    unsigned short          getPeerPort() const;
    const QString           &getPeerName() const;
    const QDateTime         &getConnectionDate() const;
    LightBird::IClient::Mode getMode() const;
    QAbstractSocket         &getSocket() const;
    QVariantMap             &getInformations();
    LightBird::ITableAccounts &getAccount();
    LightBird::IRequest     &getRequest();
    LightBird::IResponse    &getResponse();

    /// @brief This method is used to get the informations of a client in a thread
    /// safe way. It emits the signal getInformationsSignal which will calls the slot
    /// _getInformations in the client's thread. Then the informations of the client
    /// are filled, and the future is unlocked.
    /// @param client : The informations of the client are stored in this parameter.
    /// @param thread : The address of the thread that require the informations. Used
    /// to avoid dead locks.
    /// @param future : Once the informations are filled, the future is set in order
    /// to unlock the thread that is waiting for the informations.
    void    getInformations(LightBird::INetwork::Client *client, void *thread, Future<bool> *future);

private:
    Client();
    Client(const Client &);
    Client  *operator=(const Client &);

    QString                  id;                    ///< The id of the client.
    LightBird::INetwork::Transport transport;       ///< The transport protocol used by the underlaying socket.
    QStringList              protocols;             ///< The names of the protocols used to communicate with the client.
    unsigned short           port;                  ///< The local port through which the client is connected.
    int                      socketDescriptor;      ///< The descriptor of the socket.
    QHostAddress             peerAddress;           ///< The address of the client.
    unsigned short           peerPort;              ///< The peer port through which the client id connected.
    QString                  peerName;              ///< The name of the client's host (usually empty).
    LightBird::IClient::Mode mode;                  ///< The connection mode of the client.
    IReadWrite               *readWriteInterface;   ///< This interface is used to read and write data on network.
    QDateTime                connectionDate;        ///< The date of the creation of this object.
    QVariantMap              informations;          ///< Contains information on the client.
    QAbstractSocket          *socket;               ///< An abstract representation of the socket of the client.
    TableAccounts            account;               ///< Allows the client to be identified as a know account.
    QByteArray               *data;                 ///< May contains data read from the network.
    QMutex                   mutex;                 ///< Secure the access to the data member.
    Engine                   *engine;               ///< Used to process the requests, and generates the responses.

    bool            _connectToHost();
    bool            _onConnect();
    void            _onDisconnect();
    QByteArray      _simplified(QByteArray data);


public slots:
    /// @brief Calling this method tells the Client that new data are available to read.
    /// After being read, these data will ultimately feed the engine.
    void            read(QByteArray *data = NULL);

private slots:
    void            _send(const QString &id, const QString &protocol);
    void            _getInformations(LightBird::INetwork::Client *client, Future<bool> *future);

signals:
    /// @brief This signal ensure that data are read in the client thread.
    void            readSignal();
    void            sendSignal(const QString &id, const QString &protocol);
    void            getInformationsSignal(LightBird::INetwork::Client *client, Future<bool> *future);
};

#endif // CLIENT_H
