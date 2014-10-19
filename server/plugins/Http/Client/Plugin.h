#ifndef PLUGIN_H
# define PLUGIN_H

# include <QHash>
# include <QMap>
# include <QMutex>
# include <QObject>

# include "IPlugin.h"
# include "IOnDeserialize.h"
# include "IDoExecution.h"
# include "IOnSerialize.h"
# include "IOnFinish.h"
# include "IOnDisconnect.h"
# include "IOnDestroy.h"
# include "ITimer.h"

# include "Commands.h"
# include "Context.h"
# include "Files.h"
# include "Medias.h"
# include "Uploads.h"
# include "Properties.h"

# define DEFAULT_CONTENT_TYPE    "application/octet-stream" // This is the default MIME type. The browser should download the content.
# define DEFAULT_INTERFACE_NAME  "desktop"
# define IDENTIFICATION_ATTEMPTS 10 // Number of identifications attempts that can be done before the connection is systematically refused.
# define IDENTIFICATION_TIME     60 // The period during which the failed identifications are kept.

class Plugin : public QObject,
               public LightBird::IPlugin,
               public LightBird::IOnDeserialize,
               public LightBird::ITimer
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "cc.lightbird.Http.Client")
    Q_INTERFACES(LightBird::IPlugin
                 LightBird::IOnDeserialize
                 LightBird::ITimer)

public:
    Plugin();
    ~Plugin();

    // LightBird::IPlugin
    bool    onLoad(LightBird::IApi *api);
    void    onUnload();
    bool    onInstall(LightBird::IApi *api);
    void    onUninstall(LightBird::IApi *api);
    void    getMetadata(LightBird::IMetadata &metadata) const;

    // LightBird::IOnDeserialize
    void    onDeserialize(LightBird::IClient &client, LightBird::IOnDeserialize::Deserialize type);

    // These interfaces are called by the Context
    void    onDeserializeContext(LightBird::IClient &client, LightBird::IOnDeserialize::Deserialize type);
    bool    doExecution(LightBird::IClient &client);
    bool    onSerialize(LightBird::IClient &client, LightBird::IOnSerialize::Serialize type);
    void    onFinish(LightBird::IClient &client);
    bool    onDisconnect(LightBird::IClient &client);
    void    onDestroy(LightBird::IClient &client);

    // LightBird::ITimer
    bool    timer(const QString &name);

    // Static methods
    static Plugin   &instance();
    static LightBird::IApi &api();
    static Files    &files();
    static Medias   &medias();
    static Uploads  &uploads();
    /// @brief Sends a response to the client.
    static void     response(LightBird::IClient &client, int code, const QString &message, const QByteArray &content = "");
    /// @brief Returns the value of a cookie using its name.
    /// @param name : The name of the cookie to return.
    static QString  getCookie(LightBird::IClient &client, const QString &name);
    /// @brief Adds a cookie in the response of the client.
    static void     addCookie(LightBird::IClient &client, const QString &name, const QString &value = QString());

    // Miscellaneous
    /// @brief Converts a date in the proper HTTP format.
    /// @param date : The date to convert, expressed in UTC.
    /// @param separator : If true, the separator of the date (dd MM yyyy) is "-".
    /// Otherwise it is " ".
    QString         httpDate(const QDateTime &date, bool separator = false);
    /// @brief Checks if the client is allowed to attempt a new identification.
    /// If it failed a few times over a given period, the attempt is refused.
    bool            identificationAllowed(LightBird::IClient &client);
    /// @brief Called when a identification attempt failed.
    void            identificationFailed(LightBird::IClient &client);

    /// @brief Sets an http error in the response of the client.
    static inline void httpError(LightBird::IClient &client, int code, const QString &message);
    /// @brief Sets an http error in the response of the client.
    static inline void httpError(LightBird::IClient &client, int code, const QString &message, const QByteArray &content);
    /// @brief Sets an http error in the response of the client.
    static inline void httpError(LightBird::IClient &client, int code, const QString &message, const QByteArray &content, const QString &log, const QString &object, const QString &method, const Properties &properties, LightBird::ILogs::Level level);

private:
    /// @brief Manages the session.
    void    _session(LightBird::IClient &client, const QString &uri);
    /// @brief Checks if the client identifiant is valid.
    bool    _checkIdentifiant(LightBird::IClient &client, LightBird::Session &session, const QString &identifiant);
    /// @brief Returns the name of the interface used by the user.
    QString _getInterface(LightBird::IClient &client);
    /// @brief Returns the correct translation of the client.
    void    _translation(LightBird::IClient &client, const QString &interface);
    /// @brief Returns a file that is stored in the filesPath instead of the www directory.
    /// The account must have the right to read the file.
    void    _getFile(LightBird::IClient &client);

    LightBird::IApi     *_api;      ///< The LightBird's Api.
    static Plugin       *_instance; ///< The instance of the plugin.
    QStringList         interfaces; ///< Contains the name of the interfaces available.
    QString             wwwDir;     ///< The path to the www directory (where the interface is stored).
    QMap<int, QString>  daysOfWeek; ///< The names of the days of the week in english in three letters.
    QMap<int, QString>  months;     ///< The names of the months in three letters.
    QHash<QHostAddress, QPair<QDateTime, quint32> > attempts; ///< Stores the number of failed connection attempts per ip and date.
    Commands            commands;   ///< Executes the commands of the client.
    Context             _context;   ///< The context that calls the network interfaces.
    Files               _files;     ///< Manages the files.
    Medias              _medias;    ///< Manages the medias.
    Uploads             _uploads;   ///< Manages the uploads.
    QMutex              mutex;      ///< Makes this class thread safe.
};

void    Plugin::httpError(LightBird::IClient &client, int code, const QString &message)
{
    client.getResponse().getContent().setStorage(LightBird::IContent::BYTEARRAY);
    client.getResponse().setCode(code);
    client.getResponse().setMessage(message);
}

void    Plugin::httpError(LightBird::IClient &client, int code, const QString &message, const QByteArray &content)
{
    Plugin::httpError(client, code, message);
    if (!content.isEmpty())
        client.getResponse().getContent().setData(content);
}

void    Plugin::httpError(LightBird::IClient &client, int code, const QString &message, const QByteArray &content, const QString &log, const QString &object, const QString &method, const Properties &properties, LightBird::ILogs::Level level)
{
    Plugin::httpError(client, code, message, content);
    if (!log.isEmpty() && Plugin::api().log().getLevel() >= level)
        Plugin::api().log().write(level, log, properties.toMap(), object, method);
}

#endif // PLUGIN_H
