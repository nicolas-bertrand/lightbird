#ifndef ISESSION_H
# define ISESSION_H

# include <QDateTime>
# include <QMap>
# include <QString>
# include <QStringList>
# include <QVariant>

namespace LightBird
{
    /// @brief This class provides access to the information of a session.
    ///
    /// Unlike other data of the session, the clients do not survive the server
    /// shutdown, but they are not removed automatically of the session when
    /// they disconnect. So a session can be associated with a client that
    /// no longer exists. Use the network API to check the clients still connected.
    class ISession
    {
    public:
        virtual ~ISession() {}

        /// @brief Returns the id of the session.
        virtual const QString &getId() const = 0;
        /// @brief Returns the id of the account of the session, if there is one.
        virtual QString     getAccount() const = 0;
        /// @brief Modifies the account for which the session has been created.
        virtual bool        setAccount(const QString &id_account = "") = 0;
        /// @brief Returns true if the session has already expired.
        virtual bool        isExpired() const = 0;
        /// @brief Returns the expiration date of the session. Once exceeded,
        /// the session is automatically destroyed. If null, the session will
        /// be destroyed during the server shutdown. The expiration has around
        /// 10 seconds accuracy.
        virtual QDateTime   getExpiration() const = 0;
        /// @brief Modifies the expiration date of the session. Once exceeded,
        /// the session is automatically destroyed. If null, the session will
        /// be destroyed during the server shutdown. The expiration has around
        /// 10 seconds accuracy.
        virtual bool        setExpiration(const QDateTime &expiration = QDateTime()) = 0;
        /// @brief Returns the creation date of the session.
        virtual QDateTime   getCreation() const = 0;
        /// @see LightBird::ISessions::destroy
        bool                destroy(bool disconnect = false);

        // Clients
        /// @brief Returns the list of the clients associated with the session.
        virtual QStringList getClients() const = 0;
        /// @brief Associates a client to the session.
        virtual bool        setClient(const QString &client) = 0;
        /// @brief Associates several clients to the session.
        virtual bool        setClients(const QStringList &clients) = 0;
        /// @brief Removes a client of the session.
        virtual bool        removeClient(const QString &client) = 0;
        /// @brief Removes several clients of the session.
        /// @param clients : The id of the clients to remove. If empty,
        /// all the clients are removed.
        virtual bool        removeClients(const QStringList &clients = QStringList()) = 0;

        // Informations
        /// @brief Returns the value of an information of the session.
        /// @param name : The name of the information to return.
        virtual QVariant    getInformation(const QString &name) const = 0;
        /// @brief Returns all the informations of the session.
        virtual QVariantMap getInformations() const = 0;
        /// @brief Modifies the value of an information of the session, or create
        /// it if it doesn't exists.
        /// @param name : The name of the information to create or modify.
        /// @brief value : The new value of the information.
        virtual bool        setInformation(const QString &name, const QVariant &value) = 0;
        /// @brief Modifies or creates multiple informations for the session.
        /// @param informations : The informations to modify or create.
        /// The keys of the map are the keys of the informations, and
        /// the values of the map are the values of the informations.
        virtual bool        setInformations(const QVariantMap &informations) = 0;
        /// @brief Removes an information of the session.
        /// @param name : The name of the information to remove.
        virtual bool        removeInformation(const QString &name) = 0;
        /// @brief Removes multiple informations of the session.
        /// @param informations : This list contains the name of each
        /// informations to remove.
        virtual bool        removeInformations(const QStringList &informations) = 0;
    };
}

Q_DECLARE_INTERFACE(LightBird::ISession, "cc.lightbird.ISession");

#endif // ISESSION_H
