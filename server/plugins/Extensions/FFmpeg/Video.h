#ifndef VIDEO_H
# define VIDEO_H

# include "FFmpeg.h"
# include "LightBird.h"

# include "IApi.h"
# include "IVideo.h"

# define IO_BUFFER_SIZE 32728 ///< The size of the ioBuffer (see avio_alloc_context).
# define BUFFERS_NUMBER  5    ///< The number of buffers to keep in order to allows the seeking.

/// @brief Implements the IVideo extension which allows to transcode videos.
class Video : public LightBird::IVideo
{
public:
    Video(LightBird::IApi *api, QStringList &formats);
    ~Video();

    // IVideo
    bool        initialize(const QString &source);
    QByteArray  transcode();
    int         getPosition();
    void        clear();
    bool        setFormat(const QString &format);
    bool        setVideoCodec(const QString &codec);
    bool        setAudioCodec(const QString &codec);
    void        setWidth(int width);
    void        setHeight(int height);
    void        setVideoBitRate(unsigned int bitRate);
    void        setAudioBitRate(unsigned int bitRate);
    void        setFrameRate(unsigned int frameRate);
    void        setSampleRate(unsigned int sampleRate);
    void        setChannels(unsigned int channels);
    void        setStart(double start);
    void        setDuration(double duration);
    void        setOptions(const QVariantHash &options);

    /// @brief Called by the muxer to write the transcoded data to the buffers.
    /// Because the muxer can seek through the data while we are streaming it
    /// (in general for each key frames), we have to keep several seconds of
    /// data in the buffers before returning it in the transcode method.
    static int  writePacket(void *opaque, uint8_t *buf, int buf_size);
    /// @brief Seeks through the buffers. If the muxer seeks to a buffer
    /// already returned, writePacket will do nothing.
    static int64_t seekPacket(void *opaque, int64_t offset, int whence);
    /// @brief This method should not be called by AVIO.
    static int  readPacket(void *opaque, uint8_t *buf, int buf_size);

private:
    /// @brief Frees all the allocated structures and sets the default values.
    /// @param free : True if the allocated structures have to be freed.
    /// @param settings : True if the settings have to be initialized.
    void        _clear(bool free = true, bool settings = false);
    /// @brief Opens the best audio or video stream.
    AVStream    *_openStream(AVCodecContext *&context, enum AVMediaType type);
    /// @brief Initializes the output format and streams, and starts the transcoding
    void        _initializeOutput();
    /// @brief Applies the settings.
    void        _settings();
    bool        _openVideoEncoder();
    bool        _openAudioEncoder();
    /// @brief The video filters allows to scale the video, change its pixel
    /// format and its frame rate.
    bool        _configureVideoFilter();
    bool        _configureInputVideoFilter(AVFilterInOut *inputs);
    bool        _configureOutputVideoFilter(AVFilterInOut *outputs);
    /// @brief The audio filters allows to change the sample format of the audio,
    /// its sample rate and its channel layout.
    bool        _configureAudioFilter();
    bool        _configureInputAudioFilter(AVFilterInOut *inputs);
    bool        _configureOutputAudioFilter(AVFilterInOut *outputs);
    /// @brief Returns a colon-separated list of supported pixel formats.
    QByteArray  _getPixelFormats(const AVCodec *codec) const;
    /// @brief Returns a comma-separated list of supported sample formats.
    QByteArray  _getSampleFormats(const AVCodec *codec);
    /// @brief Returns a comma-separated list of supported sample rates.
    /// @param request : The prefered sample rate.
    QByteArray  _getSampleRates(const AVCodec *codec, unsigned int prefered = 0);
    /// @brief Returns a comma-separated list of supported channel layouts.
    /// @param request : The prefered channel layout.
    QByteArray  _getChannelLayouts(const AVCodec *codec, unsigned int prefered = 0);
    /// @brief Transcodes the video stream. The frame is decoded using the
    /// input packet, then is it pushed through the filter, encoded, and muxed
    /// into the buffers.
    /// @return True if a frame have been decoded.
    bool        _transcodeVideo();
    /// @brief Transcodes the audio stream. The frames are decoded using the
    /// input packet, then they are pushed through the filter, encoded, and
    /// muxed into the buffers.
    /// @param flush : Flushes the decoder cache.
    void        _transcodeAudio(bool flush = false);
    /// @brief Returns properties to display in the logs.
    QMap<QString, QString> _getProperties(int errnum = 0, Properties properties = Properties());
    /// @brief Returns the error string that corresponds to errnum.
    QByteArray  _strError(int errnum);
    /// @brief Writes a raw frame, for debug purpose.
    void        _printRawFrame();

    LightBird::IApi *api;            ///< The LightBird API.
    QStringList     &formats;        ///< The list of the available output formats.
    QString         source;          ///< The source video.
    QList<QByteArray> buffers;       ///< The transcoded data not returned yet. Each element contains approximately one second of encoded video and audio streams.
    int             framesToEncode;  ///< The number of frames to encode in the transcode method.
    int             position;        ///< The number of second transcoded so far.
    qint64          seek;            ///< The position in the transcoded data.
    qint64          bytesTranscoded; ///< The number of bytes returned by the transcode method so far.
    bool            finished;        ///< The transcoding is finished.
    // Settings
    QString         format;          ///< The output format.
    QString         videoCodec;      ///< The codec of the video stream.
    QString         audioCodec;      ///< The codec of the audio stream.
    int             width;           ///< The width of the video.
    int             height;          ///< The height of the video.
    unsigned int    videoBitRate;    ///< Number of bit per second for the video.
    unsigned int    audioBitRate;    ///< Number of bit per second for the audio.
    unsigned int    frameRate;       ///< The number of frame per second.
    unsigned int    sampleRate;      ///< The audio sample rate.
    unsigned int    channels;        ///< The number of audio channels.
    double          start;           ///< The starting position.
    double          duration;        ///< The duration of the video in second.
    QVariantHash    options;         ///< Back end specific options.
    // FFmpeg
    AVFormatContext *formatIn;
    AVFormatContext *formatOut;
    AVCodecID       videoCodecId;
    AVCodecID       audioCodecId;
    AVStream        *videoStreamIn;
    AVStream        *audioStreamIn;
    AVStream        *videoStreamOut;
    AVStream        *audioStreamOut;
    AVCodecContext  *videoDec;
    AVCodecContext  *audioDec;
    AVCodecContext  *videoEnc;
    AVCodecContext  *audioEnc;
    AVFilterGraph   *videoFilterGraph;
    AVFilterGraph   *audioFilterGraph;
    AVFilterContext *videoFilterIn;
    AVFilterContext *audioFilterIn;
    AVFilterContext *videoFilterOut;
    AVFilterContext *audioFilterOut;
    AVIOContext     *ioContext;
    unsigned char   *ioBuffer;
    AVFrame         *frame;
    AVPacket        packetIn;
    AVPacket        packetOut;
    qint64          videoFramePts;
    qint64          audioFramePts;
};

#endif // VIDEO_H
