#ifndef CLIENTS_H
# define CLIENTS_H

# include <QHostAddress>
# include <QMap>
# include <QPair>
# include <QString>
# include <QStringList>
# include <QThread>

# include "INetwork.h"
# include "IReadWrite.h"

# include "Client.h"
# include "Future.hpp"

/// @brief Manages the clients to which the server has connected. The clients are
/// manages in a dedicated thread that etablishes the connections and read/write
/// on the network. The connections TCP and UDP are supported.
class Clients : public QThread,
                public IReadWrite
{
    Q_OBJECT

public:
    Clients();
    ~Clients();

    /// @brief Creates a new Client. The connection to the client is done
    /// via IReadWrite::connect in a ThreadPool thread.
    Future<QString> connect(const QHostAddress &address, quint16 port, const QStringList &protocols,
                            LightBird::INetwork::Transport transport, int wait);
    /// @brief Asks the engine of a client to generate a new request.
    /// @see LightBird::INetwork::send
    bool            send(const QString &idClient, const QString &idPlugin, const QString &protocol, const QVariantMap &informations);
    /// @brief Asks the engine of a client to read a response.
    /// @see LightBird::INetwork::receive
    bool            receive(const QString &id, const QString &protocol, const QVariantMap &informations);
    /// @see LightBird::INetwork::getClient
    /// @param found : True if the client has been found.
    Future<bool>    getClient(const QString &id, LightBird::INetwork::Client &client, bool &found) const;
    /// @see LightBird::INetwork::getClients
    QStringList     getClients() const;
    /// @brief Disconnects the client.
    /// @param id : The id of the client to disconnect.
    bool            disconnect(const QString &id);
    /// @brief Cleans the client after the thread is finished.
    void            shutdown();

    // IReadWrite
    void            read(Client *client);
    bool            write(QByteArray *data, Client *client);
    /// @brief Emits the signal connectSignal that calls the slot _connect in the
    /// thread of the class Clients.
    bool            connect(Client *client);

signals:
    /// @brief Connects a TCP client to the server.
    void            connectSignal(QString id);
    /// @brief Emited when the Client is ready to read data on the network.
    /// They will be read from the thread via the _read method.
    void            readSignal(Client *client);
    /// @brief Emited when new data have to be written on the network, in order to
    /// write these data from the thread (where the sockets lives).
    void            writeSignal();

private:
    Clients(const Clients &);
    Clients &operator=(const Clients &);

    /// @brief The main method of the thread.
    void            run();
    /// @brief Returns true if the writeBuffer contains the client.
    bool            _containsClient(Client *client);

private slots:
    /// @brief Connects a TCP client to the server.
    void            _connect(QString id);
    /// @brief Reads the data on the network from the thread, then notifies the
    /// Client that they are ready to be processed.
    void            _read(Client *client);
    /// @brief Writes the data stored in writeBuffer on the network, from the thread.
    void            _write();
    /// @brief Called when a QTcpSocket is disconnected.
    void            _disconnected();
    /// @brief Called when the client is finished.
    void            _finished();

private:
    /// @brief Stores the data to write to a client.
    struct WriteBuffer
    {
        WriteBuffer(Client *c, QByteArray *d) : client(c), data(d), offset(0) {}
        Client      *client; ///< The client to which the data will be written.
        QByteArray  *data;   ///< The data to write.
        int         offset;  ///< The amount of data already written.
    };

    QList<Client *>    clients;       ///< The list of the clients managed.
    mutable QMutex     mutex;         ///< Makes the class thread safe.
    QList<WriteBuffer> writeBuffer;   ///< The list of the data that are going to be send from the thread.
    Future<bool>       threadStarted; ///< This future is unlocked when the thread is started.
    /// The list of the futures waiting for the connection of the client
    /// in order to set their results.
    QMap<QString, QPair<Future<QString> *, int> > connections;
};

#endif // CLIENTS_H
