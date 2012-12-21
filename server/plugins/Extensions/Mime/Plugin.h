#ifndef PLUGIN_H
# define PLUGIN_H

# include <QObject>

# include "IExtension.h"
# include "IMime.h"
# include "IPlugin.h"

class Plugin : public QObject,
               public LightBird::IPlugin,
               public LightBird::IExtension,
               public LightBird::IMime
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "cc.lightbird.Extensions.Mime")
    Q_INTERFACES(LightBird::IPlugin
                 LightBird::IExtension)

public:
    Plugin();
    ~Plugin();

    // IPlugin
    bool        onLoad(LightBird::IApi *api);
    void        onUnload();
    bool        onInstall(LightBird::IApi *api);
    void        onUninstall(LightBird::IApi *api);
    void        getMetadata(LightBird::IMetadata &metadata) const;

    // IExtension
    QStringList getExtensionsNames();
    void        *getExtension(const QString &name);
    void        releaseExtension(const QString &name, void *extension);

    // IMime
    QString     getMime(const QString &file);

private:
    LightBird::IApi *api; ///< The LightBird's Api.
};

#endif // PLUGIN_H
