#ifndef PLUGIN_H
# define PLUGIN_H

# include <QObject>
# include <QFile>
# include "FFmpeg.h"

# include "IExtension.h"
# include "IPlugin.h"

# include "Identify.h"

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

private:
    void        _loadConfiguration();
    bool        _transcode();
    AVStream    *_openStream(AVCodecContext *&context, enum AVMediaType type);
    bool        _openAudioEncoder();
    bool        _openVideoEncoder();
    bool        _configureAudioFilter();
    bool        _configureInputAudioFilter(AVFilterInOut *inputs);
    bool        _configureOutputAudioFilter(AVFilterInOut *outputs);
    bool        _configureVideoFilter();
    bool        _configureInputVideoFilter(AVFilterInOut *inputs);
    bool        _configureOutputVideoFilter(AVFilterInOut *outputs);
    /// @param flush : Flush the decoder cache.
    void        _transcodeAudio(bool flush = false);
    /// @return True if a frame have been decoded.
    bool        _transcodeVideo();
    /// @brief Returns a comma-separated list of supported sample formats.
    QByteArray  _getSampleFormats(const AVCodec *codec);
    /// @brief Returns a comma-separated list of supported sample rates.
    QByteArray  _getSampleRates(const AVCodec *codec);
    /// @brief Returns a comma-separated list of supported channel layouts.
    QByteArray  _getChannelLayouts(const AVCodec *codec);
    /// @brief Returns a colon-separated list of supported pixel formats.
    QByteArray  _getPixelFormats(const AVCodec *codec) const;

    LightBird::IApi *api;      ///< The LightBird Api.
    Identify        *identify; ///< Implements the IIdentify extension.
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
    AVFilterGraph   *audioFilterGraph;
    AVFilterContext *audioFilterIn;
    AVFilterContext *audioFilterOut;
    AVFilterGraph   *videoFilterGraph;
    AVFilterContext *videoFilterIn;
    AVFilterContext *videoFilterOut;
};

#endif // PLUGIN_H
