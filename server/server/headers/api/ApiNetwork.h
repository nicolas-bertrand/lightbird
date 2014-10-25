#ifndef	APINETWORK_H
# define APINETWORK_H

# include <QObject>

# include "INetwork.h"

/// @brief Implements the interface INetwork that allows to access to the
/// server network features.
class ApiNetwork : public QObject,
                   public LightBird::INetwork
{
    Q_OBJECT

public:
    ApiNetwork(const QString &id);
    ~ApiNetwork();

    bool        openPort(unsigned short port, const QStringList &protocols = QStringList(), LightBird::INetwork::Transport transport = LightBird::INetwork::TCP, unsigned int maxClients = ~0);
    bool        closePort(unsigned short port, LightBird::INetwork::Transport transport = LightBird::INetwork::TCP);
    bool        getPort( unsigned short port, QStringList &protocols, unsigned int &maxClients, LightBird::INetwork::Transport transport = LightBird::INetwork::TCP) const;
    QList<unsigned short> getPorts(LightBird::INetwork::Transport transport = LightBird::INetwork::TCP) const;
    bool        getClient(const QString &id, LightBird::INetwork::Client &client) const;
    QStringList getClients() const;
    QStringList getClients(unsigned short port, LightBird::INetwork::Transport transport = LightBird::INetwork::TCP) const;
    QSharedPointer<LightBird::IFuture<QString> > connect(const QHostAddress &address, quint16 port, const QStringList &protocols = QStringList(), LightBird::INetwork::Transport transport = LightBird::INetwork::TCP, const QVariantMap &informations = QVariantMap(), const QStringList &contexts = QStringList(QString()), int wait = -1);
    bool        disconnect(const QString &id, bool fatal = false);
    bool        send(const QString &id, const QString &protocol = "", const QVariantMap &informations = QVariantMap());
    bool        receive(const QString &id, const QString &protocol = "", const QVariantMap &informations = QVariantMap());
    bool        pause(const QString &idClient, int msec = -1);
    bool        resume(const QString &idClient);
    void        setDisconnectIdle(const QString &id, qint64 msec = -1, bool fatal = false);
    qint64      getDisconnectIdle(const QString &id, bool *fatal = NULL);
    void        setDisconnectTime(const QString &id, const QDateTime &time = QDateTime(), bool fatal = false);
    QDateTime   getDisconnectTime(const QString &id, bool *fatal = NULL);

private:
    ApiNetwork();
    ApiNetwork(const ApiNetwork &);
    ApiNetwork &operator=(const ApiNetwork &);

    QString id; ///< The id of the plugin for which the object has been created.
};

#endif // APINETWORK_H
