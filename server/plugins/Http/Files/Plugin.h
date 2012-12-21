#ifndef PLUGIN_H
# define PLUGIN_H

# include <QObject>

# include "IDoExecution.h"
# include "IPlugin.h"

# define DEFAULT_CONTENT_TYPE "application/octet-stream" ///< The default MIME type.

class Plugin : public QObject,
               public LightBird::IPlugin,
               public LightBird::IDoExecution
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "cc.lightbird.Http.Files")
    Q_INTERFACES(LightBird::IPlugin
                 LightBird::IDoExecution)

public:
    Plugin();
    ~Plugin();

    // LightBird::IPlugin
    bool    onLoad(LightBird::IApi *api);
    void    onUnload();
    bool    onInstall(LightBird::IApi *api);
    void    onUninstall(LightBird::IApi *api);
    void    getMetadata(LightBird::IMetadata &metadata) const;

    bool    doExecution(LightBird::IClient &client);

private:
    /// @brief Authenticate the client using the basic access authentication of HTTP.
    bool    _authenticate(LightBird::IClient &client, LightBird::IRequest &request, LightBird::IResponse &response);
    /// @brief Returns the MIME type of the file, based on its extension.
    QString _getMime(const QString &file);
    /// @brief Converts the size into a string with the closest unit.
    QString _size(quint64 size);

    LightBird::IApi *api;
    QString         content;
    static const QString link;
};

#endif // PLUGIN_H
