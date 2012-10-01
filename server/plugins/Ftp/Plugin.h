#ifndef PLUGIN_H
# define PLUGIN_H

# include <QMap>
# include <QObject>
# include <QReadWriteLock>
# include <QString>
# include <QStringList>

# include "IPlugin.h"
# include "IOnConnect.h"
# include "IOnDisconnect.h"
# include "IOnDestroy.h"
# include "IDoSerializeContent.h"
# include "IDoExecution.h"
# include "IDoUnserializeContent.h"
# include "IDoUnserializeHeader.h"
# include "IDoSend.h"
# include "IOnExecution.h"
# include "IOnSerialize.h"
# include "IOnFinish.h"
# include "ITimer.h"

# include "ClientHandler.h"
# include "Parser.h"

class Plugin : public QObject,
               public LightBird::IPlugin,
               public LightBird::IOnConnect,
               public LightBird::IOnDisconnect,
               public LightBird::IOnDestroy,
               public LightBird::IDoUnserializeHeader,
               public LightBird::IDoUnserializeContent,
               public LightBird::IDoExecution,
               public LightBird::IOnExecution,
               public LightBird::IOnSerialize,
               public LightBird::IDoSerializeContent,
               public LightBird::IDoSend,
               public LightBird::IOnFinish,
               public LightBird::ITimer
{
    Q_OBJECT
    Q_INTERFACES(LightBird::IPlugin
                 LightBird::IOnConnect
                 LightBird::IOnDisconnect
                 LightBird::IOnDestroy
                 LightBird::IDoUnserializeHeader
                 LightBird::IDoUnserializeContent
                 LightBird::IDoExecution
                 LightBird::IOnExecution
                 LightBird::IOnSerialize
                 LightBird::IDoSerializeContent
                 LightBird::IDoSend
                 LightBird::IOnFinish
                 LightBird::ITimer)

public:
    /// Stores the configuration of the plugin.
    struct      Configuration
    {
        quint32 maxPacketSize;      ///< The maximum size send at a time to the client.
        quint32 timeWaitControl;    ///< The maximum amount of time in millisecond during which the data connection will wait the control connection to be ready.
        QString dataProtocolName;   ///< The name of the data connection protocol.
        unsigned short passivePort; ///< The port on which the clients can etablish a passive connection.
    };

    Plugin();
    ~Plugin();

    // IPlugin
    bool    onLoad(LightBird::IApi *api);
    void    onUnload();
    bool    onInstall(LightBird::IApi *api);
    void    onUninstall(LightBird::IApi *api);
    void    getMetadata(LightBird::IMetadata &metadata) const;

    // Connect / Disconnect
    bool    onConnect(LightBird::IClient &client);
    bool    onDisconnect(LightBird::IClient &client);
    void    onDestroy(LightBird::IClient &client);

    // Execution
    bool    doUnserializeHeader(LightBird::IClient &client, const QByteArray &data, quint64 &used);
    bool    doUnserializeContent(LightBird::IClient &client, const QByteArray &data, quint64 &used);
    bool    doExecution(LightBird::IClient &client);
    bool    onExecution(LightBird::IClient &client);
    bool    onSerialize(LightBird::IClient &client, LightBird::IOnSerialize::Serialize type);
    bool    doSerializeContent(LightBird::IClient &client, QByteArray &data);
    bool    doSend(LightBird::IClient &client);
    /// @brief Disconnect the client if there is an error in its request
    void    onFinish(LightBird::IClient &client);

    // ITimer
    bool    timer(const QString &name);

    /// @brief Returns the parser that is in charge of the client, in a thread safe way.
    Parser  *_getParser(const LightBird::IClient &client);
    /// @brief Adds a file to be identified in the timer thread.
    /// @param idFile : The id of the file to identify.
    static void identify(const QString &idFile);
    /// @brief Returns the configuration of the plugin
    static Configuration     &getConfiguration();

private:
    LightBird::IApi          *api;          ///< The LightBird's Api.
    QReadWriteLock           mutex;         ///< Makes parsers thread safe.
    QHash<QString, Parser *> parsers;       ///< Parses the FTP control and data connections.
    ClientHandler            *handler;      ///< This single object handles all the connections.
    QStringList              identifyList;  ///< The list of the id of the files to identify in the timer thread.
    QMutex                   identifyMutex; ///< Makes identifyFiles thread safe.
    static Configuration     configuration; ///< The configuration of the plugin.
    static Plugin            *instance;     ///< Allows to access the Plugin instance from a static method.
};

#endif // PLUGIN_H
