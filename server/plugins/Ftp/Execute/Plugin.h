#ifndef PLUGIN_H
# define PLUGIN_H

# include <QMap>
# include <QObject>
# include <QReadWriteLock>
# include <QString>
# include <QStringList>

# include "IPlugin.h"
# include "IOnConnect.h"
# include "IDoExecution.h"
# include "IDoSend.h"
# include "IOnFinish.h"

# include "ClientHandler.h"

class Plugin : public QObject,
               public LightBird::IPlugin,
               public LightBird::IOnConnect,
               public LightBird::IDoExecution,
               public LightBird::IDoSend,
               public LightBird::IOnFinish
{
    Q_OBJECT
    Q_INTERFACES(LightBird::IPlugin
                 LightBird::IOnConnect
                 LightBird::IDoExecution
                 LightBird::IDoSend
                 LightBird::IOnFinish)

public:
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

    // IOnExecution
    bool    doExecution(LightBird::IClient &client);
    bool    doSend(LightBird::IClient &client);
    
    // IOnFinish
    /// @brief Disconnect the client if there is an error in its request
    void    onFinish(LightBird::IClient &client);

private:
    LightBird::IApi         *api;           ///< The LightBird's Api.
    QReadWriteLock          mutex;          ///< Make parsers thread-safe.

    ClientHandler *handler; // This single object handles all the connections.


};

#endif // PLUGIN_H
