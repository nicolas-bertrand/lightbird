#ifndef PLUGIN_H
# define PLUGIN_H

# include <QObject>

# include "IPlugin.h"
# include "IOnDeserialize.h"

# include "Context.h"

# define DEFAULT_CONTENT_TYPE "application/octet-stream" ///< The default MIME type.

class Plugin : public QObject,
               public LightBird::IPlugin,
               public LightBird::IOnDeserialize
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "cc.lightbird.Http.Files")
    Q_INTERFACES(LightBird::IPlugin
                 LightBird::IOnDeserialize)

public:
    Plugin();
    ~Plugin();

    // LightBird::IPlugin
    bool    onLoad(LightBird::IApi *api);
    void    onUnload();
    bool    onInstall(LightBird::IApi *api);
    void    onUninstall(LightBird::IApi *api);
    void    getMetadata(LightBird::IMetadata &metadata) const;

    /// @brief Sets the context name of the network if the URL starts with "/f/".
    void    onDeserialize(LightBird::IClient &client, LightBird::IOnDeserialize::Deserialize type);
    /// @brief Called by the network context to execute the request.
    bool    doExecution(LightBird::IClient &client);

private:
    /// @brief Authenticate the client using the basic access authentication of HTTP.
    bool    _authenticate(LightBird::IClient &client, LightBird::IRequest &request, LightBird::IResponse &response);
    /// @brief Converts the size into a string with the closest unit.
    QString _size(quint64 size);

    LightBird::IApi *api;      ///< The LightBird API.
    Context         *context;  ///< The network context from which doExecution is called.
    QString         content;   ///< The directory HTML file content.
    static const QString link; ///< A generic link.
};

#endif // PLUGIN_H
