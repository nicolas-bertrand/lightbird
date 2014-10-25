#ifndef CLIENTS_H
# define CLIENTS_H

# include <QHostAddress>
# include <QMap>
# include <QPair>
# include <QSharedPointer>
# include <QStringList>
# include <QThread>

# include "INetwork.h"
# include "IReadWrite.h"

# include "Client.h"
# include "ClientsNetwork.h"
# include "Future.hpp"
# include "WriteBuffer.h"

/// @brief Manages the clients to which the server has connected. The clients are
/// managed in a dedicated thread that etablishes the connections and tells when
/// to read/write on the network. The connections TCP and UDP are supported.
class Clients
    : public QThread
    , public IReadWrite
    , public LightBird::Initialize
{
    Q_OBJECT

public:
    Clients();
    ~Clients();

    /// @brief Creates a new Client.
    Future<QString> connect(const QHostAddress &address, quint16 port, const QStringList &protocols, LightBird::INetwork::Transport transport, const QVariantMap &informations, const QStringList &contexts, int wait);
    /// @brief Asks the engine of a client to generate a new request.
    /// @see LightBird::INetwork::send
    bool            send(const QString &id, const QString &idPlugin, const QString &protocol, const QVariantMap &informations);
    /// @brief Asks the engine of a client to read a response.
    /// @see LightBird::INetwork::receive
    bool            receive(const QString &id, const QString &protocol, const QVariantMap &informations);
    /// @see LightBird::INetwork::pause
    bool            pause(const QString &id, int msec = -1);
    /// @see LightBird::INetwork::resume
    bool            resume(const QString &id);
    /// @see LightBird::INetwork::getClient
    /// @param found : True if the client has been found.
    Future<bool>    getClient(const QString &id, LightBird::INetwork::Client &client, bool &found) const;
    /// @see LightBird::INetwork::getClients
    QStringList     getClients() const;
    /// @brief Disconnects the client.
    /// @see LightBird::INetwork::disconnect
    bool            disconnect(const QString &id, bool fatal = false);
    /// @see LightBird::INetwork::setDisconnectIdle
    bool            setDisconnectIdle(const QString &id, qint64 msec = -1, bool fatal = false);
    /// @see LightBird::INetwork::getDisconnectIdle
    bool            getDisconnectIdle(const QString &id, bool *fatal, qint64 &result);
    /// @see LightBird::INetwork::setDisconnectTime
    bool            setDisconnectTime(const QString &id, const QDateTime &time = QDateTime(), bool fatal = false);
    /// @see LightBird::INetwork::getDisconnectTime
    bool            getDisconnectTime(const QString &id, bool *fatal, QDateTime &result);
    /// @brief Closes the clients and ends the thread.
    void            close();

    // IReadWrite
    void            read(Client *client);
    void            write(Client *client, const QByteArray &data);

private slots:
    /// @brief The connection to a socket is finished.
    void            _connected(Socket *socket, bool success);
    /// @brief Writes the data that could not be wrote in write().
    void            _write();
    /// @brief Called when a socket is disconnected.
    void            _disconnected(Socket *socket);
    /// @brief Called when the client is finished.
    void            _finished(Client *client);

private:
    Clients(const Clients &);
    Clients &operator=(const Clients &);

    /// @brief The main method of the thread.
    void            run();

    /// @brief Returns the shared pointer of a client.
    QSharedPointer<Client> _getClient(Client *client);

    ClientsNetwork *_network; ///< Manages the clients to which the server is connected.
    QList<QSharedPointer<Client> > _clients; ///< The list of the clients managed.
    mutable QReadWriteLock _mutex; ///< Makes the class thread safe.
    Future<bool> _threadStarted; ///< This future is unlocked when the thread is started.
    QWaitCondition _threadFinished; ///< Allows to wait until all the client are finished before quitting the thread.
    QMap<QString, Future<QString> *> _connections; ///< The list of the futures waiting for the connection of the client in order to set their results.
    QHash<QSharedPointer<Client>, QSharedPointer<WriteBuffer> > _writeBuffers; ///< Stores the data that could not be written in write().
};

#endif // CLIENTS_H
