#ifndef PLUGIN_H
# define PLUGIN_H

# include <QMap>
# include <QObject>
# include <QReadWriteLock>
# include <QSharedPointer>
# include <QString>
# include <QStringList>

# include "IPlugin.h"
# include "IOnConnect.h"
# include "IOnDestroy.h"
# include "ITimer.h"

# include "Commands.h"
# include "Control.h"
# include "Data.h"
# include "Mutex.h"
# include "Parser.h"
# include "Timer.h"

class Plugin : public QObject,
               public LightBird::IPlugin,
               public LightBird::IOnConnect,
               public LightBird::IOnDestroy,
               public LightBird::ITimer
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "cc.lightbird.Ftp")
    Q_INTERFACES(LightBird::IPlugin
                 LightBird::IOnConnect
                 LightBird::IOnDestroy
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

    // Other
    bool    onConnect(LightBird::IClient &client);
    void    onDestroy(LightBird::IClient &client);
    bool    timer(const QString &name);

    /// Stores the configuration of the plugin.
    struct      Configuration
    {
        quint32 maxPacketSize;      ///< The maximum size sent at a time to the client.
        quint32 waitConnectionTime; ///< The maximum amount of time in millisecond during which the data connection will wait the control connection to be ready, and vice versa.
        quint32 timeout;            ///< The number of seconds an inactive client can stay connected to the server.
        QList<ushort> passivePorts; ///< The ports on which the clients can etablish a passive data connection.
    };

    /// @brief Sends a message on the control connection.
    /// @param controlId : The id of the control connection.
    /// @param message : The message to send.
    static void     sendControlMessage(const QString &controlId, const Commands::Result &message);
    /// @brief Returns the parser that is in charge of the client.
    static Parser   &getParser(LightBird::IClient &client);
    /// @brief Returns the commands.
    static Commands &getCommands();
    /// @brief Returns the timer manager.
    static Timer    &getTimer();
    /// @brief Returns the configuration of the plugin
    static Configuration &getConfiguration();
    /// @brief Returns the locked plugin mutex.
    static QSharedPointer<Mutex> getMutex(const QString &object, const QString &function);

    // Manages the passive connections
    /// @brief Returns an available passive port for this client.
    static ushort   getPassivePort(LightBird::IClient &client);
    /// @brief Returns the data connection waiting for this control connection, if available.
    /// @return A locked mutex that allows to guarantee the atomicity of the dataId, after the return.
    static QSharedPointer<Mutex> getDataConnection(LightBird::Session &session, LightBird::IClient &client, QString &dataId);
    /// @brief Returns the control connection waiting for this data connection, if available.
    /// @param isValid : Returns false if the client was not expected.
    /// @return A locked mutex that allows to guarantee the atomicity of the controlId, after the return.
    static QSharedPointer<Mutex> getControlConnection(LightBird::IClient &client, QString &controlId, bool &isValid);

private:
    /// @brief Opens the passive data ports, from the contiguration.
    void    _openPassiveDataPorts();
    /// @brief Removes all the occurences of controlId and dataId from the passive connection maps and list.
    /// @param passivePort : True if the passive ports map have to be cleaned.
    void    _cleanPassiveConnections(const QString &controlId, const QString &dataId = "", bool passivePort = true);

    LightBird::IApi *api;           ///< The LightBird's Api.
    static Plugin   *instance;      ///< Allows to access the Plugin instance from a static method.
    Control         *control;       ///< Manages the control connections.
    Data            *data;          ///< Manages the data connections.
    Commands        *commands;      ///< Executes the FTP commands.
    Timer           *timerManager;  ///< Manages the connections timeout.
    Configuration   configuration;  ///< The configuration of the plugin.
    QMutex          mutex;          ///< Makes the class thread safe.
    QStringList     controlWaiting; ///< The list of the control clients waiting for a data connection.
    QMultiMap<ushort, QPair<QString, QHostAddress> > dataWaiting; ///< The list of the data clients waiting for a control connection.
    QMultiMap<ushort, QPair<QString, QHostAddress> > passivePorts; ///< The list of the passive ports waiting to be used.
};

#endif // PLUGIN_H
