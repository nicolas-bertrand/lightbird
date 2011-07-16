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

    bool                  openPort(unsigned short port, const QStringList &protocols = QStringList(),
                                  LightBird::INetwork::Transport transport = LightBird::INetwork::TCP,
                                  unsigned int maxClients = ~0);
    bool                  closePort(unsigned short port);
    bool                  getPort(unsigned short port, QStringList &protocols,
                                  LightBird::INetwork::Transport &transport, unsigned int &maxClients);
    QList<unsigned short> getPorts();
    bool                  getClient(const QString &id, LightBird::INetwork::Client &client);
    QStringList           getClients();
    QStringList           getClients(unsigned short port);
    QSharedPointer<LightBird::IFuture<QString> > connect(const QHostAddress &address,
                          quint16 port, const QStringList &protocols = QStringList(),
                          LightBird::INetwork::Transport transport = LightBird::INetwork::TCP,
                          int wait = -1);
    bool                  disconnect(const QString &id);
    bool                  send(const QString &id, const QString &protocol = "");

private:
    ApiNetwork();
    ApiNetwork(const ApiNetwork &);
    ApiNetwork* operator=(const ApiNetwork &);

    QString id; ///< The id of the plugin for which the object has been created.
};

#endif // APINETWORK_H
