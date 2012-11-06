#ifndef PLUGIN_H
# define PLUGIN_H

# include <QObject>
# include <QFile>
// INT64_C and UINT64_C are not defined in C++ and used by FFmpeg
#define INT64_C(c) (c ## LL)
#define UINT64_C(c) (c ## ULL)
// FFmpeg is a C library
extern "C"
{
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
    #include <libavfilter/avfiltergraph.h>
    #include <libavfilter/avcodec.h>
    #include <libavfilter/buffersrc.h>
    #include <libavfilter/buffersink.h>
    #include <libavutil/imgutils.h>
    #include <libavutil/timestamp.h>
    #include <libavutil/log.h>
    #include <libavutil/opt.h>
}

# include "IExtension.h"
# include "IPlugin.h"

# define DEFAULT_SAMPLE_RATE 44100

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
    bool        _configureAudioFilter();
    bool        _configureInputAudioFilter(AVFilterInOut *inputs);
    bool        _configureOutputAudioFilter(AVFilterInOut *outputs);
    /// @param flush : Flush the decoder cache.
    void        _transcodeAudio(bool flush = false);
    /// @return True if a frame have been decoded.
    bool        _transcodeVideo();
    // check that a given sample format is supported by the encoder
    bool        _checkSampleFormat(const AVCodec *codec, enum AVSampleFormat format);
    // just pick the highest supported samplerate
    quint32     _getSampleRate(AVCodec *codec);
    // select layout with the highest channel count
    quint32     _getChannelLayout(AVCodec *codec);
    void        _swap(qint64 &a, qint64 &b);

    LightBird::IApi *api; ///< The LightBird Api.
    AVFormatContext *formatIn;
    AVFormatContext *formatOut;
    const char      *source;
    const char      *destination;
    AVCodecID       audioCodecId;
    AVCodecID       videoCodecId;
    AVStream        *audioStreamIn;
    AVStream        *audioStreamOut;
    AVStream        *videoStreamIn;
    AVStream        *videoStreamOut;
    AVCodecContext  *audioDec;
    AVCodecContext  *audioEnc;
    AVCodecContext  *videoDec;
    AVCodecContext  *videoEnc;
    AVFrame         *frame;
    quint64         audioFramePts;
    qint64          videoFramePts;
    AVPacket        packetIn;
    AVPacket        packetOut;
    AVFilterGraph   *filterGraph;
    AVFilterContext *filterInput;
    AVFilterContext *filterOutput;
};

#endif // PLUGIN_H
