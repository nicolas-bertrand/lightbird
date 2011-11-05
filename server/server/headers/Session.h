#ifndef SESSION_H
# define SESSION_H

# include <QReadWriteLock>

# include "ISession.h"

# include "Table.h"

/// @brief The server implementation of ISession. All the data of this class
/// are stored in the database, except the clients list.
class Session : public Table,
                public LightBird::ISession
{
public:
    Session(const QString &id);
    Session(const QDateTime &expiration = QDateTime(),
            const QString &id_account = QString(),
            const QStringList &clients = QStringList(),
            const QVariantMap &informations = QVariantMap());
    ~Session();
    Session(const Session &session);
    Session &operator=(const Session &session);

    // Fields
    const QString   &getId() const;
    QString         getAccount() const;
    bool            setAccount(const QString &id_account = "");
    bool            isExpired() const;
    QDateTime       getExpiration() const;
    bool            setExpiration(const QDateTime &expiration = QDateTime());
    QDateTime       getCreation() const;

    // Clients
    QStringList     getClients() const;
    bool            setClient(const QString &client);
    bool            setClients(const QStringList &clients);
    bool            removeClient(const QString &client);
    bool            removeClients(const QStringList &clients = QStringList());

    // Informations
    QVariant        getInformation(const QString &name) const;
    QVariantMap     getInformations() const;
    bool            setInformation(const QString &name, const QVariant &value);
    bool            setInformations(const QVariantMap &informations);
    bool            removeInformation(const QString &name);
    bool            removeInformations(const QStringList &informations);

    /// @brief Delete the session in the database.
    bool            destroy();

private:
    QDateTime               expiration;   ///< When this data is reached, the session is destroyed.
    QString                 id_account;   ///< The account to which is associated the session.
    QDateTime               creation;     ///< The creation date of the session in the database.
    QStringList             clients;      ///< Stores the clients of the session.
    QVariantMap             informations; ///< Stores the informations of the session.
    bool                    destroyed;    ///< True when the session has expired and has been destroyed.
    mutable QReadWriteLock  mutex;        ///< Makes this class thread safe.
};

#endif // SESSION_H
