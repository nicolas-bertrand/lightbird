#ifndef PLUGIN_H
# define PLUGIN_H

# include <QObject>
# include <QMutex>
# include "FFmpeg.h"

# include "IExtension.h"
# include "IPlugin.h"

# include "Audio.h"
# include "Identify.h"
# include "Preview.h"
# include "Video.h"

class Plugin : public QObject,
               public LightBird::IPlugin,
               public LightBird::IExtension
{
    Q_OBJECT
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

    /// @brief Calls avcodec_open2 in a thread safe way.
    static int  avcodec_open2(AVCodecContext *avctx, const AVCodec *codec, AVDictionary **options);

private:
    void        _loadConfiguration();

    LightBird::IApi *api;      ///< The LightBird Api.
    QStringList     formats;   ///< The list of the available output formats.
    QMutex          mutex;     ///< Makes the class thread safe.
    static Plugin   *instance; ///< The global instance of the plugin, used by the static methods.
    Identify        *identify; ///< The IIdentify extension.
    Preview         *preview;  ///< The IPreview extension.
    QList<QSharedPointer<Audio> > audios; ///< The IAudio extensions.
    QList<QSharedPointer<Video> > videos; ///< The IVideo extensions.
};

#endif // PLUGIN_H
