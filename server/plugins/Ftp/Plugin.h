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
# include "IDoDeserializeContent.h"
# include "IDoDeserializeHeader.h"
# include "IDoExecution.h"
# include "IOnExecution.h"
# include "IOnSerialize.h"
# include "IDoSerializeContent.h"
# include "IDoSend.h"
# include "IOnFinish.h"
# include "ITimer.h"

# include "ClientHandler.h"
# include "Parser.h"
# include "Timer.h"

class Plugin : public QObject,
               public LightBird::IPlugin,
               public LightBird::IOnConnect,
               public LightBird::IOnDisconnect,
               public LightBird::IOnDestroy,
               public LightBird::IDoDeserializeHeader,
               public LightBird::IDoDeserializeContent,
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
                 LightBird::IDoDeserializeHeader
                 LightBird::IDoDeserializeContent
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
        quint32 timeout;            ///< The number of seconds an inactive client can stay connected to the server.
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
    bool    doDeserializeHeader(LightBird::IClient &client, const QByteArray &data, quint64 &used);
    bool    doDeserializeContent(LightBird::IClient &client, const QByteArray &data, quint64 &used);
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
    /// @brief Sends a message on the control connection.
    /// @param idClient : The id of the control connection.
    /// @param message : The message to send.
    static void sendControlMessage(const QString &controlId, const Commands::Result &message);
    /// @brief Returns the configuration of the plugin
    static Configuration     &getConfiguration();

private:
    LightBird::IApi          *api;          ///< The LightBird's Api.
    QReadWriteLock           mutex;         ///< Makes parsers thread safe.
    QHash<QString, Parser *> parsers;       ///< Parses the FTP control and data connections.
    ClientHandler            *handler;      ///< This single object handles all the connections.
    Timer                    *timerManager; ///< Manages the identification of the uploaded files and the connections timeout.
    static Configuration     configuration; ///< The configuration of the plugin.
    static Plugin            *instance;     ///< Allows to access the Plugin instance from a static method.
};

#endif // PLUGIN_H
