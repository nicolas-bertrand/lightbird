#ifndef LIGHTBIRD_ISESSIONS_H
# define LIGHTBIRD_ISESSIONS_H

# include <QDateTime>
# include <QSharedPointer>
# include <QString>
# include <QStringList>

# include "ISession.h"

namespace LightBird
{
    typedef QSharedPointer<LightBird::ISession> Session;

    /// @brief This class manages the sessions, which allow to combine an account
    /// to several clients in order to identify them. They have an expiration date,
    /// and can survive a server shutdown. A session can also store information.
    /// @see LightBird::ISession
    class ISessions
    {
    public:
        virtual ~ISessions() {}

        /// @brief Creates a session.
        /// @param expiration : The expiration date of the session in local time.
        /// Once exceeded, the session is automatically destroyed. It has around
        /// 10 seconds accuracy. If null, the session will be destroyed during
        /// the server shutdown.
        /// @param id_account : Associates an account to the session.
        /// @param clients : The id of the clients associated with the session.
        /// @param informations : Some informations on the session, if relevant.
        /// @return The new session.
        virtual LightBird::Session  create(const QDateTime &expiration = QDateTime(),
                                           const QString &id_account = QString(),
                                           const QStringList &clients = QStringList(),
                                           const QVariantMap &informations = QVariantMap()) = 0;
        /// @brief Destroys a session.
        /// @param id : The id of the session.
        /// @param disconnect : If the clients associated with the session have
        /// to be disconnected. Keep in mind that a client can be associated with
        /// several sessions.
        virtual bool                destroy(const QString &id, bool disconnect = false) = 0;
        /// @brief Returns the id of the sessions that match the parameters.
        /// @param id_account : The account associated with the sessions. If empty
        /// the account is ignored.
        /// @param client : The client associated with the sessions. If empty
        /// the client is ignored.
        virtual QStringList         getSessions(const QString &id_account = "", const QString &client = "") const = 0;
        /// @brief Returns the session based on its id.
        /// @param id : The id of the session.
        /// @return An object that allows to get the data of a session,
        /// or NULL if it does not exist.
        virtual LightBird::Session  getSession(const QString &id) = 0;
        /// @brief Returns true if the session exists and is not expired.
        virtual bool                exists(const QString &id) = 0;
    };
}

Q_DECLARE_INTERFACE(LightBird::ISessions, "cc.lightbird.ISessions")

#endif // LIGHTBIRD_ISESSIONS_H
