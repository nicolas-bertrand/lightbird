#ifndef PLUGIN_H
# define PLUGIN_H

# include <QHash>
# include <QMap>
# include <QMutex>
# include <QObject>

# include "IDoExecution.h"
# include "IOnDisconnect.h"
# include "IOnFinish.h"
# include "IOnSerialize.h"
# include "IOnUnserialize.h"
# include "IPlugin.h"
# include "ITimer.h"

# define DEFAULT_CONTENT_TYPE    "application/octet-stream" // This is the default MIME type. The browser may download the content.
# define DEFAULT_INTERFACE_NAME  "desktop"
# define DATE_FORMAT             "yyyy-MM-dd hh:mm"
# define IDENTIFICATION_ATTEMPTS 5  // Number of identifications attempts that can be done before the connection is systematically refused.
# define IDENTIFICATION_TIME     30 // The period during which the failed identifications are kept.

class Plugin : public QObject,
               public LightBird::IPlugin,
               public LightBird::IOnUnserialize,
               public LightBird::IDoExecution,
               public LightBird::IOnSerialize,
               public LightBird::IOnFinish,
               public LightBird::IOnDisconnect,
               public LightBird::ITimer
{
    Q_OBJECT
    Q_INTERFACES(LightBird::IPlugin LightBird::IOnUnserialize LightBird::IDoExecution LightBird::IOnSerialize
                 LightBird::IOnFinish LightBird::IOnDisconnect LightBird::ITimer)

public:
    Plugin();
    ~Plugin();

    // IPlugin
    bool    onLoad(LightBird::IApi *api);
    void    onUnload();
    bool    onInstall(LightBird::IApi *api);
    void    onUninstall(LightBird::IApi *api);
    void    getMetadata(LightBird::IMetadata &metadata) const;

    // IOnUnserialize, IDoExecution, IOnSerialize, IOnFinish, ITimer
    void    onUnserialize(LightBird::IClient &client, LightBird::IOnUnserialize::Unserialize type);
    bool    doExecution(LightBird::IClient &client);
    bool    onSerialize(LightBird::IClient &client, LightBird::IOnSerialize::Serialize type);
    void    onFinish(LightBird::IClient &client);
    void    onDisconnect(LightBird::IClient &client);
    bool    timer(const QString &name);

    // Other
    static Plugin   &getInstance();
    /// @brief Returns the LightBird Api.
    static LightBird::IApi &api();
    /// @brief Send a response to the client.
    static void     response(LightBird::IClient &client, int code, const QString &message, const QByteArray &content = "");
    /// @brief Returns the value of a cookie using its name.
    /// @param name : The name of the cookie to return.
    static QString  getCookie(LightBird::IClient &client, const QString &name);
    /// @brief Adds a cookie in the response of the client.
    static void     addCookie(LightBird::IClient &client, const QString &name, const QString &value = QString());
    /// @brief Converts a date in the proper HTTP format.
    /// @param date : The date to convert.
    /// @param separator : If true, the separator of the date (dd MM yyyy) is "-".
    /// Otherwise it is " ".
    QString         httpDate(const QDateTime &date, bool separator = false);
    /// @brief Checks if the client is allowed to attempt a new identification.
    /// If it failed a few times over a given period, the attempt is refused.
    bool            identificationAllowed(LightBird::IClient &client);
    /// @brief Called when a identification attempt failed.
    void            identificationFailed(LightBird::IClient &client);

private:
    /// @brief Manages the session.
    void    _session(LightBird::IClient &client, const QString &uri);
    /// @brief Checks that the token is correct.
    bool    _checkToken(LightBird::IClient &client, LightBird::Session &session, const QByteArray &token, const QString &uri);
    /// @brief Returns the name of the interface used by the user.
    QString _getInterface(LightBird::IClient &client);
    /// @brief Returns a file that is stored in the filesPath instead of the www directory.
    /// The account must have the right to read the file.
    void    _getFile(LightBird::IClient &client);
    /// @brief Returns the mime type of the file in parameter.
    QString _getMime(const QString &file);

    LightBird::IApi     *_api;      ///< The LightBird's Api.
    static Plugin       *instance;  ///< The instance of the plugin singleton.
    QStringList         interfaces; ///< Contains the name of the interfaces available.
    QString             wwwDir;     ///< The path to the www directory (where the interface is stored).
    QMap<int, QString>  daysOfWeek; ///< The names of the days of the week in english in three letters.
    QMap<int, QString>  months;     ///< The names of the months in three letters.
    QHash<QHostAddress, QPair<QDateTime, quint32> > attempts; ///< Stores the number of failed connection attempts per ip and date.
    QMutex              mutex;      ///< Makes this class thread safe.
};

#endif // PLUGIN_H
