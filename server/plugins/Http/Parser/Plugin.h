#ifndef PLUGIN_H
# define PLUGIN_H

# include <QMap>
# include <QObject>
# include <QReadWriteLock>
# include <QString>
# include <QStringList>

# include "IDoSerializeContent.h"
# include "IDoSerializeHeader.h"
# include "IDoUnserializeContent.h"
# include "IDoUnserializeHeader.h"
# include "IOnConnect.h"
# include "IOnDisconnect.h"
# include "IOnFinish.h"
# include "IOnProtocol.h"
# include "IPlugin.h"

# include "Parser.h"

class Plugin : public QObject,
               public LightBird::IPlugin,
               public LightBird::IOnConnect,
               public LightBird::IOnProtocol,
               public LightBird::IDoUnserializeHeader,
               public LightBird::IDoUnserializeContent,
               public LightBird::IDoSerializeHeader,
               public LightBird::IDoSerializeContent,
               public LightBird::IOnFinish,
               public LightBird::IOnDisconnect
{
    Q_OBJECT
    Q_INTERFACES(LightBird::IPlugin
                 LightBird::IOnConnect
                 LightBird::IOnProtocol
                 LightBird::IDoUnserializeHeader
                 LightBird::IDoUnserializeContent
                 LightBird::IDoSerializeHeader
                 LightBird::IDoSerializeContent
                 LightBird::IOnFinish
                 LightBird::IOnDisconnect)

public:
    /// Stores the configuration of the plugin.
    struct      Configuration
    {
        QStringList protocols;          ///< The name of the protocols suported by the plugin.
        quint32     maxHeaderSize;      ///< The maximum size of the header. After that, an error is sent to the client, and it is disconnected.
        quint32     maxPacketSize;      ///< The maximum size send at a time to the client.
        quint32     maxContentInMemory; ///< The maximum content size to store in the memory. Above that, it is stored in a temporary file.
        QStringList methodContent;      ///< The name of the methods that can have a content in the request.
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
    void    onDisconnect(LightBird::IClient &client);

    // Unserialize
    bool    onProtocol(LightBird::IClient &client, const QByteArray &data, QString &protocol, bool &error);
    bool    doUnserializeHeader(LightBird::IClient &client, const QByteArray &data, quint64 &used);
    bool    doUnserializeContent(LightBird::IClient &client, const QByteArray &data, quint64 &used);

    // Serialize
    void    doSerializeHeader(LightBird::IClient &client, QByteArray &data);
    bool    doSerializeContent(LightBird::IClient &client, QByteArray &data);

    // IOnFinish
    /// @brief Disconnect the client if there is an error in its request
    void    onFinish(LightBird::IClient &client);

    /// @brief Returns the configuration of the plugin
    static Configuration    &getConfiguration();

private:
    LightBird::IApi         *api;           ///< The LightBird's Api.
    QMap<QString, Parser *> parsers;        ///< Associates a parser to a client id.
    QReadWriteLock          mutex;          ///< Make parsers thread-safe.
    static Configuration    configuration;  ///< The configuration of the plugin.

    /// @brief Returns the parser that is in charge of the client, in thread safe way.
    Parser  *_getParser(const LightBird::IClient &client);
};

#endif // PLUGIN_H
