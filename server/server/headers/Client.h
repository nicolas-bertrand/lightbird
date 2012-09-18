#ifndef CLIENT_H
# define CLIENT_H

# include <QList>
# include <QMap>
# include <QMutex>

# include "IClient.h"
# include "INetwork.h"
# include "IReadWrite.h"

# include "Future.hpp"
# include "ThreadPool.h"

class Engine;

/// @brief Manages a client connected to the server. The actions of the client
/// are executed in a thread of the ThreadPool to allow any number of client
/// to run simultaneously.
class Client : public QObject,
               public ThreadPool::ITask,
               public LightBird::IClient
{
    Q_OBJECT

public:
    /// @param socket : The socket from which the client is connected.
    /// @param protocols : The protocols used to communicate with the client.
    /// @param transport : The transport protocol used by this client.
    /// @param port : The local port from which the client is connected.
    /// @param socketDescriptor : The descriptor of the socket used by the client.
    /// @param peerAddress : The address of the client.
    /// @param peerPort : The port from which the client is connected in his host.
    /// @param peerName : The name of the client's host. May be empty.
    /// @param mode : The connection mode of the client.
    /// @param readWriteInterface : Allows the client to read and write on the network.
    Client(QAbstractSocket *socket, LightBird::INetwork::Transport transport, const QStringList &protocols,
           unsigned short port, int socketDescriptor, const QHostAddress &peerAddress,
           unsigned short peerPort, const QString &peerName, LightBird::IClient::Mode mode,
           IReadWrite *readWriteInterface);
    ~Client();

    /// @brief Performs the actions of the client in a thread of the ThreadPool.
    void                    run();
    /// @brief Write the data to the client. This method takes ownership of the data.
    /// The processing of the Client is paused until written() is called.
    void                    write(QByteArray *data);
    /// @brief Asks the engine to generate a new request.
    /// @see LightBird::INetwork::send
    bool                    send(const QString &protocol, const QVariantMap &informations, const QString &id = "");
    /// @brief Asks the engine to read a response without sending a request.
    /// @see LightBird::INetwork::receive
    bool                    receive(const QString &protocol, const QVariantMap &informations);
    /// @brief Disconnect the client
    void                    disconnect();
    /// @brief Calls the IDoRead interface of the plugins.
    bool                    doRead(QByteArray &data);
    /// @brief Calls the IDoWrite interface of the plugins.
    bool                    doWrite(QByteArray &data);
    /// @brief Returns true if the client is finished and can be safely destroyed.
    bool                    isFinished() const;
    /// @brief Changes the port member of the client (doesn't affect the real port).
    void                    setPort(unsigned short port);
    /// @brief Checks if the client accepts the protocol in parameter. If it is
    /// empty, the first protocol in the protocols list is returned. If nothing
    /// is returned, the protocol is invalid.
    QString                 getProtocol(QString protocol = "");
    /// @brief This method is used to get the informations of a client in a thread
    /// safe way. It stores the information request in a map (this->informationsRequests), which
    /// will be used in the client's ThreadPool to fill the informations and unlock the future.
    /// @param client : The informations of the client are stored in this parameter.
    /// @param future : Once the informations are filled, the future is set in order
    /// to unlock the thread that is waiting for the informations.
    void    getInformations(LightBird::INetwork::Client &client, Future<bool> *future);

    // LightBird::IClient
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
    QAbstractSocket         &getSocket();
    QVariantMap             &getInformations();
    LightBird::TableAccounts &getAccount();
    LightBird::IRequest     &getRequest();
    LightBird::IResponse    &getResponse();
    QStringList             getSessions(const QString &id_account = QString()) const;
    LightBird::Session      getSession(const QString &id_account = QString()) const;
    bool                    isDisconnecting() const;

public slots:
    /// @brief Calling this method tells the Client that new data are available to read.
    /// After being read, these data will ultimately feed the engine. The client takes
    /// ownership of the data.
    void                    read(QByteArray *data = NULL);
    /// @brief Tells the Client that the data are being written on the network.
    /// At this point the Client can be safely disconnected. This event is
    /// followed by bytesWritten.
    void                    bytesWriting();
    /// @brief Tells the Client that the data have been written on the network,
    /// and that it can resume its processing.
    void                    bytesWritten();

signals:
    /// @brief The client has been disconnected and can be safely destroyed.
    void                    finished();

private:
    Client();
    Client(const Client &);
    Client &operator=(const Client &);

    /// @brief The possible states of the client.
    enum State
    {
        CONNECT,    ///< The client is still connecting.
        READ,       ///< Data should be available to read on the network.
        SEND,       ///< Data have to be sent to the client.
        RECEIVE,    ///< Data have to be received without sending a request.
        RUN,        ///< The engine is running.
        RESUME,     ///< Allows to resume the processing of the Client after the data have been written on the network.
        DISCONNECT, ///< The client is going to be disconnected.
        NONE        ///< The client is idle.
    };

    /// @brief Run a new task in the threadpool and modify the state of the client
    /// if the parameter is different from NONE. This method returns immediatly, and
    /// the task will be executed in the ThreaPool.
    void    _newTask(Client::State state = Client::NONE);
    /// @brief Get the data read by the method read() and feed the Engine.
    /// @return True if there is some data waiting to be processed.
    bool    _read();
    /// @brief Asks the Engine to generate a new request.
    /// @return True if the Engine is generating the request.
    bool    _send();
    /// @brief Asks the Engine to read a response without sending a request.
    /// @return True if the Engine is ready to read the response.
    bool    _receive();
    /// @brief Calls IOnConnect.
    bool    _onConnect();
    /// @brief Calls IOnDisconnect.
    bool    _onDisconnect();
    /// @brief Calls IOnDestroy.
    void    _onDestroy();
    /// @brief Calls _getInformations for each elements in this->informationsRequests.
    void    _getInformations();
    /// @brief Fills the informations of the client in the client struct, and unlock the future.
    void    _getInformations(LightBird::INetwork::Client &client, Future<bool> *future);
    /// @brief Emit the finished signal and set disconnected to true.
    void    _finish();

    QString                  id;                  ///< The id of the client.
    LightBird::INetwork::Transport transport;     ///< The transport protocol used by the underlaying socket.
    QStringList              protocols;           ///< The names of the protocols used to communicate with the client.
    unsigned short           port;                ///< The local port through which the client is connected.
    int                      socketDescriptor;    ///< The descriptor of the socket.
    QHostAddress             peerAddress;         ///< The address of the client.
    unsigned short           peerPort;            ///< The peer port through which the client is connected.
    QString                  peerName;            ///< The name of the client's host (usually empty).
    LightBird::IClient::Mode mode;                ///< The connection mode of the client.
    IReadWrite               *readWriteInterface; ///< This interface is used to read and write data on network.
    QDateTime                connectionDate;      ///< The date of the connection.
    QVariantMap              informations;        ///< Contains information on the client.
    QAbstractSocket          *socket;             ///< An abstract representation of the socket of the client.
    LightBird::TableAccounts account;             ///< Allows the client to be identified as a know account.
    QList<QByteArray *>      data;                ///< The list of all the data read from the network.
    Engine                   *engine;             ///< Used to process the requests and the responses.
    State                    state;               ///< The state of the client.
    State                    oldTask;             ///< Used to restore the old state of the client in order to complete its tasks before disconnecting.
    State                    resume;              ///< The state to resume after the date have been written on the network.
    bool                     running;             ///< A task is running in a thread of the threadpool.
    bool                     readyRead;           ///< Data are available on the network.
    bool                     writing;             ///< The client's task is paused while the data are being written on the network.
    bool                     written;             ///< The data have been written on the socket but not sent on the network yet (bytesWriting).
    bool                     finish;              ///< If true, the client is going to be disconnected.
    bool                     disconnecting;       ///< The client is disconnected from the server, but there is still data to process.
    bool                     disconnected;        ///< The client has been disconnected and and can be safely destroyed.
    QMutex                   mutex;               ///< Makes this class thread safe.
    QList<QVariantMap>       sendRequests;        ///< Stores the idPlugin, the informations and the protocol of the requests that are going to be sent.
    QList<QVariantMap>       receiveResponses;    ///< Stores the protocol and the informations of the responses that are going to be received.
    QMap<Future<bool> *, LightBird::INetwork::Client *> informationsRequests; ///< Used by getInformations to keep track of the informations requests.
};

inline void Client::_newTask(Client::State state)
{
    this->state = state;
    this->running = true;
    ThreadPool::instance()->addTask(this);
}

#endif // CLIENT_H
