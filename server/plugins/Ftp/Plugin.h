#ifndef PLUGIN_H
# define PLUGIN_H

# include <QMap>
# include <QObject>
# include <QReadWriteLock>
# include <QString>
# include <QStringList>

# include "IDoSerializeContent.h"
# include "IDoSerializeHeader.h"
# include "IDoExecution.h"
# include "IDoUnserializeContent.h"
# include "IDoSend.h"
# include "IOnConnect.h"
# include "IOnExecution.h"
# include "IOnSerialize.h"
# include "IOnDisconnect.h"
# include "IOnDestroy.h"
# include "IOnFinish.h"
# include "IPlugin.h"

# include "ClientHandler.h"

class Parser;

class Plugin : public QObject,
               public LightBird::IPlugin,
               public LightBird::IOnConnect,
               public LightBird::IDoSend,
               public LightBird::IDoUnserializeContent,
               public LightBird::IDoExecution,
               public LightBird::IOnExecution,
               public LightBird::IOnSerialize,
               public LightBird::IDoSerializeContent,
               public LightBird::IOnFinish,
               public LightBird::IOnDisconnect,
               public LightBird::IOnDestroy
{
    Q_OBJECT
    Q_INTERFACES(LightBird::IPlugin
                 LightBird::IOnConnect
                 LightBird::IDoSend
                 LightBird::IDoUnserializeContent
                 LightBird::IDoExecution
                 LightBird::IOnExecution
                 LightBird::IOnSerialize
                 LightBird::IDoSerializeContent
                 LightBird::IOnFinish
                 LightBird::IOnDisconnect
                 LightBird::IOnDestroy)

public:
    /// Stores the configuration of the plugin.
    struct      Configuration
    {
        QStringList protocols;          ///< The name of the protocols suported by the plugin.
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

    // Unserialize
    bool    doUnserializeContent(LightBird::IClient &client, const QByteArray &data, quint64 &used);

    // Serialize
    bool    doSerializeContent(LightBird::IClient &client, QByteArray &data);

    // Execution
    bool    doExecution(LightBird::IClient &client);
    bool    onExecution(LightBird::IClient &client);

    bool     doSend(LightBird::IClient &client);

    // IOnSerialize
    bool    onSerialize(LightBird::IClient &client, LightBird::IOnSerialize::Serialize type);
    // IOnFinish
    /// @brief Disconnect the client if there is an error in its request
    void    onFinish(LightBird::IClient &client);

    /// @brief Returns the configuration of the plugin
    static Configuration    &getConfiguration();

private:
    LightBird::IApi          *api;          ///< The LightBird's Api.
    QReadWriteLock           mutex;         ///< Make parsers thread-safe.
    static Configuration     configuration; ///< The configuration of the plugin.
    QHash<QString, Parser *> parsers;
    ClientHandler            *handler;      ///< This single object handles all the connections.

};

#endif // PLUGIN_H
