#ifndef PLUGIN_H
# define PLUGIN_H

# include "IExtension.h"
# include "IPlugin.h"

# include "Identify.h"
# include "Image.h"

class Plugin : public QObject,
               public LightBird::IPlugin,
               public LightBird::IExtension
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "cc.lightbird.Extensions.ImageMagick")
    Q_INTERFACES(LightBird::IPlugin LightBird::IExtension)

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

private:
    LightBird::IApi *api;      ///< The LightBird Api.
    Identify        *identify; ///< Implements the IIdentify extension.
    Image           *image;    ///< Implements the IImage and IPreview extensions.
};

#endif // PLUGIN_H
