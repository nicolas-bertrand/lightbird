#ifndef CLIENT_H
# define CLIENT_H

# include <QList>
# include <QMap>
# include <QMutex>
# include <QTimer>

# include "IClient.h"
# include "INetwork.h"
# include "IReadWrite.h"

# include "Context.h"
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
    /// @param contexts : The names of the contexts of the client.
    Client(QAbstractSocket *socket, const QStringList &protocols, LightBird::INetwork::Transport transport, unsigned short port, int socketDescriptor, const QHostAddress &peerAddress, unsigned short peerPort, const QString &peerName, LightBird::IClient::Mode mode, IReadWrite *readWriteInterface, const QStringList &contexts = QStringList(QString()));
    ~Client();

    /// @brief Performs the actions of the client in a thread of the ThreadPool.
    void                    run();
    /// @brief Called by the readWriteInterface to notifity the Client that data
    /// have been read and are ready to be processed.
    void                    bytesRead();
    /// @brief Writes the data to the client. This method takes ownership of the data.
    /// The processing of the Client is paused until bytesWritten() is called. The data to
    /// send are stored in Client::writing and actually written in Client::_write.
    void                    write(QByteArray *data);
    /// @brief Asks the engine to generate a new request.
    /// @see LightBird::INetwork::send
    bool                    send(const QString &protocol, const QVariantMap &informations, const QString &id = "");
    /// @brief Asks the engine to read a response without sending a request.
    /// @see LightBird::INetwork::receive
    bool                    receive(const QString &protocol, const QVariantMap &informations);
    /// @see LightBird::INetwork::pause
    bool                    pause(int msec = -1);
    /// @brief Disconnects the client.
    void                    disconnect();
    /// @brief Calls the IDoRead interface of the plugins.
    bool                    doRead(QByteArray &data);
    /// @brief Calls the IDoWrite interface of the plugins.
    /// @param result : The return of the IDoWrite call.
    /// @return True if doWrite has been called.
    bool                    doWrite(const char *data, qint64 size, qint64 &result);
    /// @brief Returns true if the client is finished and can be safely destroyed.
    bool                    isFinished() const;
    /// @brief Returns the data buffer of the client. This method should only be
    /// used when the Client is in reading phase.
    QByteArray              &getData();
    /// @brief Changes the port member of the client (doesn't affect the real port).
    void                    setPort(unsigned short port);
    /// @brief Checks if the client accepts the protocol in parameter. If it is
    /// empty, the first protocol in the protocols list is returned. If nothing
    /// is returned, the protocol is invalid.
    QString                 getProtocol(QString protocol = "");
    /// @brief Returns the validator used to validate the context in order to
    /// call the network interfaces.
    /// @param useName : True if we have to validate the name of the context.
    /// @param requestProtocol : True if the protocol of the request have to be used.
    /// Otherwise the protocols of the client will be validated.
    /// @param useMethodType : True if we have to validate the method and the type
    /// of the request.
    Context::Validator      &getValidator(bool useName = true, bool requestProtocol = true, bool useMethodType = false);
    /// @brief This method is used to get the informations of a client in a
    /// thread safe way. It stores the information request in a map
    /// (this->informationsRequests), which will be used in the client's ThreadPool
    /// to fill the informations and unlock the future.
    /// @param client : The informations of the client are stored in this parameter.
    /// @param future : Once the informations are filled, the future is set in order
    /// to unlock the thread that is waiting for the informations.
    void    getInformations(LightBird::INetwork::Client &client, Future<bool> *future);

    // LightBird::IClient
    const QString           &getId() const;
    QAbstractSocket         &getSocket();
    unsigned short          getPort() const;
    const QStringList       &getProtocols() const;
    LightBird::INetwork::Transport getTransport() const;
    int                     getSocketDescriptor() const;
    const QHostAddress      &getPeerAddress() const;
    unsigned short          getPeerPort() const;
    const QString           &getPeerName() const;
    const QDateTime         &getConnectionDate() const;
    quint64                 getBufferSize() const;
    LightBird::IClient::Mode getMode() const;
    QStringList             &getContexts();
    QVariantMap             &getInformations();
    LightBird::TableAccounts &getAccount();
    LightBird::IRequest     &getRequest();
    LightBird::IResponse    &getResponse();
    QStringList             getSessions(const QString &id_account = QString()) const;
    LightBird::Session      getSession(const QString &id_account = QString()) const;
    bool                    isPaused() const;
    bool                    isDisconnecting() const;

public slots:
    /// @brief Calling this method tells the Client that new data are available,
    /// and starts the reading phase: the READING state is set, then the data are
    /// read in the appropriate thread via the readWriteInterface. Finally the
    /// READ state is set, and the engine is RUN if any data have been read.
    void                    readyRead();
    /// @brief Tells the Client that the data have been written on the network,
    /// and that it can resume its processing.
    void                    bytesWritten();
    /// @brief Resumes the paused network workflow. Can be called directly, or
    /// via the pause timer.
    /// @see LightBird::INetwork::resume
    bool                    resume();

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
        READING,    ///< Data should be available to read on the network.
        READ,       ///< Data have been read and are ready to be processed.
        WRITTEN,    ///< Data have been written on the socket.
        SEND,       ///< Data have to be sent to the client.
        RECEIVE,    ///< Data have to be received without sending a request.
        RUN,        ///< The engine is running.
        DISCONNECT, ///< The client is going to be disconnected.
        PAUSE,      ///< The network workflow has to be paused.
        RESUME,     ///< The network workflow has to be resumed.
        NONE        ///< The client is idle.
    };
    /// @brief The pause states of the network workflow.
    struct Pause
    {
        enum State
        {
            NONE,    ///< The network workflow is running normally.
            PAUSING, ///< Is being paused. The current task have to be finished.
            PAUSED,  ///< Has been paused. No more network interface is called, except IOnDisconnect.
            RESUMING ///< Is being resumed.
        };
    };
    /// @brief The disconnect states of the client.
    struct Disconnect
    {
        enum State
        {
            NONE,          ///< The client is connected.
            DISCONNECT,    ///< The client is going to be disconnected.
            DISCONNECTING, ///< The client is disconnected from the server, but there is still data to process.
            DISCONNECTED   ///< The client has been disconnected and can be safely destroyed.
        };
    };
    /// @brief Allows to save temporarily the state of the variables in the run method.
    struct RunState
    {
        RunState() : newTask(Client::NONE), send(true), receive(true) {}
        State   newTask;
        bool    send;
        bool    receive;
    };

    /// @brief Run a new task in the threadpool and modify the state of the client
    /// if the parameter is different from NONE. This method returns immediatly, and
    /// the task will be executed in the ThreaPool.
    void    _newTask(Client::State state = Client::NONE);
    /// @brief Ends the reading phase and calls LightBird::IOnRead via the Engine.
    /// @return True if there is data waiting to be processed.
    bool    _read();
    /// @brief Writes the data stored in Client::writing to the client.
    void    _write(Client::State newTask);
    /// @brief Asks the Engine to generate a new request.
    /// @return True if the Engine is generating the request.
    bool    _send();
    /// @brief Asks the Engine to read a response without sending a request.
    /// @return True if the Engine is ready to read the response.
    bool    _receive();
    /// @brief Pauses the network workflow.
    bool    _pause();
    /// @brief Calls IOnPause.
    void    _onPause();
    /// @brief Calls IOnResume.
    void    _onResume();
    /// @brief Calls IOnConnect.
    bool    _onConnect();
    /// @brief Disconnects the client.
    bool    _disconnect();
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
    QDateTime                connectionDate;      ///< The date of the connection, in local time.
    QStringList              contexts;            ///< The names of the contexts of the client.
    QVariantMap              informations;        ///< Contains information on the client.
    QAbstractSocket          *socket;             ///< An abstract representation of the socket of the client.
    LightBird::TableAccounts account;             ///< Allows the client to be identified as a know account.
    QByteArray               data;                ///< The data read on the network, waiting to be processed.
    Engine                   *engine;             ///< Used to process the requests and the responses.
    State                    state;               ///< The state of the client.
    RunState                 runStatePause;       ///< Used to restore the state of the client in order to complete its tasks after resuming.
    RunState                 runStateDisconnect;  ///< Used to restore the state of the client in order to complete its tasks before disconnecting.
    bool                     running;             ///< A task is running in a thread of the threadpool.
    bool                     reading;             ///< Data are available on the network.
    QByteArray               *writing;            ///< Stores the data to write. The client's task is paused while the data are being written on the network (i.e. while writing is not NULL).
    State                    written;             ///< The state to resume after the data have been written on the socket.
    Pause::State             pauseState;          ///< The pause state of the network workflow.
    QTimer                   pauseTimer;          ///< Resumes the network workflow when the pause duration elapsed.
    Disconnect::State        disconnectState;     ///< The disconnect state of the client.
    QMutex                   mutex;               ///< Makes this class thread safe.
    QList<QVariantMap>       sendRequests;        ///< Stores the idPlugin, the informations and the protocol of the requests that are going to be sent.
    QList<QVariantMap>       receiveResponses;    ///< Stores the protocol and the informations of the responses that are going to be received.
    Context::Validator       validator;           ///< Validates the current context in order to call the network interfaces.
    QMap<Future<bool> *, LightBird::INetwork::Client *> informationsRequests; ///< Used by getInformations to keep track of the informations requests.
};

inline void Client::_newTask(Client::State state)
{
    this->state = state;
    this->running = true;
    ThreadPool::instance()->addTask(this);
}

#endif // CLIENT_H
