#ifndef PLUGIN_H
# define PLUGIN_H

# include <QObject>
# include <QFile>
// INT64_C and UINT64_C are not defined in C++
#define INT64_C(c) (c ## LL)
#define UINT64_C(c) (c ## ULL)
// FFmpeg is a C library
extern "C"
{
    #include <libavformat/avformat.h>
    #include <libavutil/imgutils.h>
    #include <libavutil/timestamp.h>
    #include <libavutil/log.h>
    #include <libavutil/opt.h>
}

# include "IExtension.h"
# include "IPlugin.h"

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

    // IExtensions
    QStringList getExtensionsNames();
    void        *getExtension(const QString &name);
    void        releaseExtension(const QString &name, void *extension);

private:
    AVStream    *_openStream(enum AVMediaType type);
    /// @return True if a frame have been decoded.
    bool        _transcode(AVMediaType type);
    void        _swap(qint64 &a, qint64 &b);

    LightBird::IApi *api; ///< The LightBird Api.
    AVFormatContext *inputFormat;
    AVFormatContext *outputFormat;
    const char      *source;
    const char      *destination;
    AVCodecID       codecId;
    AVStream        *inputVideoStream;
    AVStream        *outputVideoStream;
    AVCodecContext  *decoderContext;
    AVCodecContext  *encoderContext;
    AVFrame         *frame;
    qint64          framePts;
    AVPacket        packetIn;
    AVPacket        packetOut;
};

#endif // PLUGIN_H
