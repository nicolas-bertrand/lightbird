#ifndef CLIENTS_H
# define CLIENTS_H

# include <QHostAddress>
# include <QObject>
# include <QMap>
# include <QPair>
# include <QString>
# include <QStringList>

# include "INetwork.h"
# include "IReadWrite.h"

# include "Client.h"
# include "Future.hpp"

/// @brief Manages the clients to which the server has connected.
class Clients : public QObject,
                public IReadWrite
{
    Q_OBJECT

public:
    Clients();
    ~Clients();

    /// @brief Creates a new Client and tries to connect to it.
    void        connect(const QHostAddress &address, quint16 port, const QStringList &protocols,
                        LightBird::INetwork::Transports transport, int wait, Future<QString> *future);
    /// @brief Asks the engine of a client to generate a new request.
    bool        send(const QString &idClient, const QString &idPlugin, const QString &protocol);
    /// @brief Allows to get the informations of a client.
    /// @param id : The id of the client to find.
    /// @param client : The information of the client to fill.
    /// @param thread : The address of the thread that requires the informations.
    /// Used to avoid dead locks.
    /// @param future : Used to unlock the thread that is waiting the informations.
    /// @return True if the searched client exists.
    bool        getClient(const QString &id, LightBird::INetwork::Client *client, void *thread, Future<bool> *future);
    /// @brief Returns the id of the clients to which the server is connected.
    QStringList getClients();
    /// @brief Disconnects the client.
    /// @param id : The id of the client to disconnect.
    bool        disconnect(const QString &id);

    // IReadWrite
    bool        read(QByteArray &data, Client *client);
    bool        write(QByteArray &data, Client *client);
    bool        connect(Client *client);

private:
    Clients(const Clients &);
    Clients &operator=(const Clients &);

    QList<Client *> clients; ///< The list of the clients managed.
    QMutex          mutex;   ///< Makes the class thread safe.
    /// The list of the futures waiting for the connection of the client
    /// in order to set there results.
    QMap<QString, QPair<Future<QString> *, int> > futures;

private slots:
    /// @brief Called when a QTcpSocket is disconnected.
    void        _disconnected();
    /// @brief Called when the client thread is finished.
    void        _finished();
};

#endif // CLIENTS_H
