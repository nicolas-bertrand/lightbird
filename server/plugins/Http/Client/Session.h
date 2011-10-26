#ifndef SESSION_H
# define SESSION_H

# include "ITableAccounts.h"

class Session
{
public:
    Session();
    ~Session();
    Session(const Session &session);
    Session &operator=(const Session &session);

    bool                        operator==(const QString &id);
    const QString               &getId();
    LightBird::ITableAccounts   &getAccount();
    /// @brief Identify a client using the identifiant in parameter (SHA1(nameSHA1(password)sid)).
    bool                        identify(const QString &identifiant);
    /// @brief Returns the date of the creation of the session.
    QDateTime                   &getCreation();
    /// @brief Returns the date of the last update of the session.
    QDateTime                   &getUpdate();
    /// @brief Update the session.
    void                        setUpdate();

private:
    QString                     id;       ///< The id of the session (sid).
    LightBird::ITableAccounts   *account; ///< The account from which the client if connected.
    QDateTime                   creation; ///< The date of the creation of the session.
    QDateTime                   update;   ///< The date of the last update of the session.
};

#endif // SESSION_H
