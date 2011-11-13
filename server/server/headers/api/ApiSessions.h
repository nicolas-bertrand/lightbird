#ifndef APISESSIONS_H
# define APISESSIONS_H

# include <QObject>
# include <QMutex>
# include <QTimer>

# include "ISessions.h"

/// @brief The server implementation of ISessions which allows plugins to
/// use the sessions at will.
class ApiSessions : public QObject,
                    public LightBird::ISessions
{
    Q_OBJECT
    Q_INTERFACES(LightBird::ISessions)

public:
    ApiSessions(QObject *parent = NULL);
    ~ApiSessions();

    LightBird::Session  create(const QDateTime &expiration = QDateTime(),
                               const QString &id_account = QString(),
                               const QStringList &clients = QStringList(),
                               const QVariantMap &informations = QVariantMap());
    bool                destroy(const QString &id, bool disconnect = false);
    QStringList         getSessions(const QString &id_account = "", const QString &client = "") const;
    LightBird::Session  getSession(const QString &id);
    bool                exists(const QString &id);
    static ApiSessions  *instance();

public slots:
    /// @brief Destroy the expired sessions. It is called by the timer each
    /// time a session has expired. It should also be called when the expiration
    /// date of a session is modified in order to update the interval of the timer.
    void                expiration();

signals:
    /// @brief Allows to call the method expiration in the main thread.
    void                expirationSignal();

private:
    ApiSessions(const ApiSessions &);
    ApiSessions &operator=(const ApiSessions &);

private:
    QMap<QString, LightBird::Session> sessions; ///< A cache that stores the session beeing used by the plugins.
    QTimer              timer;  ///< Calls expiration each time a session expires.
    mutable QMutex      mutex;  ///< Makes this class thread safe.
};

#endif // APISESSIONS_H
