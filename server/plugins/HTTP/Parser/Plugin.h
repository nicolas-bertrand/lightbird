#ifndef PLUGIN_H
# define PLUGIN_H

# include <QObject>
# include <QReadWriteLock>
# include <QMap>

# include "IPlugin.h"
# include "IResources.h"
# include "IOnConnect.h"
# include "IOnProtocol.h"
# include "IDoUnserializeHeader.h"
# include "IDoUnserializeContent.h"
# include "IDoSerializeHeader.h"
# include "IDoSerializeContent.h"
# include "IOnWrote.h"
# include "IOnDisconnect.h"

# include "Parser.h"

class Plugin : public QObject,
               public Streamit::IPlugin,
               public Streamit::IResources,
               public Streamit::IOnConnect,
               public Streamit::IOnProtocol,
               public Streamit::IDoUnserializeHeader,
               public Streamit::IDoUnserializeContent,
               public Streamit::IDoSerializeHeader,
               public Streamit::IDoSerializeContent,
               public Streamit::IOnWrote,
               public Streamit::IOnDisconnect
{
    Q_OBJECT
    Q_INTERFACES(Streamit::IPlugin
                 Streamit::IResources
                 Streamit::IOnConnect
                 Streamit::IOnProtocol
                 Streamit::IDoUnserializeHeader
                 Streamit::IDoUnserializeContent
                 Streamit::IDoSerializeHeader
                 Streamit::IDoSerializeContent
                 Streamit::IOnWrote
                 Streamit::IOnDisconnect)

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
    bool    onLoad(Streamit::IApi *api);
    void    onUnload();
    bool    onInstall(Streamit::IApi *api);
    void    onUninstall(Streamit::IApi *api);

    // IResources
    QString getResourcesPath();

    // Connect / Disconnect
    bool    onConnect(Streamit::IClient &client);
    void    onDisconnect(Streamit::IClient &client);

    // Unserialize
    bool    onProtocol(Streamit::IClient &client, const QByteArray &data, QString &protocol, bool &error);
    bool    doUnserializeHeader(Streamit::IClient &client, const QByteArray &data, quint64 &used);
    bool    doUnserializeContent(Streamit::IClient &client, const QByteArray &data, quint64 &used);

    // Serialize
    void    doSerializeHeader(Streamit::IClient &client, QByteArray &data);
    bool    doSerializeContent(Streamit::IClient &client, QByteArray &data);

    // IOnWrote
    /// @brief Disconnect the client if there is an error in its request
    void    onWrote(Streamit::IClient &client);

    /// @brief Returns the configuration of the plugin
    static Configuration    &getConfiguration();

private:
    Streamit::IApi          *api;           ///< The Streamit's Api.
    QMap<QString, Parser>   parsers;        ///< Associates a parser to a client id.
    QReadWriteLock          mutex;          ///< Make parsers thread-safe.
    static Configuration    configuration;  ///< The configuration of the plugin.

    /// @brief Returns the parser that is in charge of the client, in thread safe way.
    Parser  *_getParser(const Streamit::IClient &client);
};

#endif // PLUGIN_H
