#ifndef AUDIO_H
# define AUDIO_H

# include "FFmpeg.h"
# include "LightBird.h"

# include "IApi.h"
# include "IAudio.h"

# define IO_BUFFER_SIZE 32728 ///< The size of the ioBuffer (see avio_alloc_context).

/// @brief Implements the IAudio extension which allows to transcode audios.
class Audio : public LightBird::IAudio
{
public:
    Audio(LightBird::IApi *api, QStringList &formats);
    ~Audio();

    // IAudio
    bool        initialize(const QString &source);
    QByteArray  transcode();
    int         getPosition();
    void        clear();
    bool        setFormat(const QString &format);
    bool        setCodec(const QString &codec);
    void        setBitRate(unsigned int bitRate);
    void        setSampleRate(unsigned int sampleRate);
    void        setChannels(unsigned int channels);
    void        setStart(double start);
    void        setDuration(double duration);
    void        setOptions(const QVariantHash &options);

    /// @brief Called by the muxer to write the transcoded data to the buffer.
    static int  writePacket(void *opaque, uint8_t *buf, int buf_size);

private:
    /// @brief Frees all the allocated structures and sets the default values.
    /// @param free : True if the allocated structures have to be freed.
    /// @param settings : True if the settings have to be initialized.
    void        _clear(bool free = true, bool settings = false);
    /// @brief Opens the best audio stream.
    AVStream    *_openStream(AVCodecContext *&context);
    /// @brief Initializes the output format and streams, and starts the transcoding
    void        _initializeOutput();
    /// @brief Applies the settings.
    void        _settings();
    bool        _openEncoder();
    /// @brief The filters allows to change the sample format of the audio,
    /// the sample rate and the channel layout.
    bool        _configureFilter();
    bool        _configureInputFilter(AVFilterInOut *inputs);
    bool        _configureOutputFilter(AVFilterInOut *outputs);
    /// @brief Returns a comma-separated list of supported sample formats.
    QByteArray  _getSampleFormats(const AVCodec *codec);
    /// @brief Returns a comma-separated list of supported sample rates.
    /// @param request : The prefered sample rate.
    QByteArray  _getSampleRates(const AVCodec *codec, unsigned int prefered = 0);
    /// @brief Returns a comma-separated list of supported channel layouts.
    /// @param request : The prefered channel layout.
    QByteArray  _getChannelLayouts(const AVCodec *codec, unsigned int prefered = 0);
    /// @brief Transcodes the audio stream. The frames are decoded using the
    /// input packet, then they are pushed through the filter, encoded, and
    /// muxed into the buffer.
    /// @param flush : Flushes the decoder cache.
    void        _transcode(bool flush = false);
    /// @brief Returns properties to display in the logs.
    QMap<QString, QString> _getProperties(int errnum = 0, Properties properties = Properties());
    /// @brief Returns the error string that corresponds to errnum.
    QByteArray  _strError(int errnum);

    LightBird::IApi *api;           ///< The LightBird API.
    QStringList     &formats;       ///< The list of the available output formats.
    QString         source;         ///< The input file name.
    QByteArray      buffer;         ///< The data being transcoded.
    int             framesToEncode; ///< The number of frames to encode in the transcode method.
    bool            finished;       ///< The transcoding is finished.
    // Settings
    QString         format;         ///< The output format.
    QString         codec;          ///< The codec of the audio stream.
    unsigned int    bitRate;        ///< Number of bit per second.
    unsigned int    sampleRate;     ///< The sample rate.
    unsigned int    channels;       ///< The number of channels.
    double          start;          ///< The starting position.
    double          duration;       ///< The duration of the audio in second.
    QVariantHash    options;        ///< Back end specific options.
    // FFmpeg
    AVFormatContext *formatIn;
    AVFormatContext *formatOut;
    AVCodecID       codecId;
    AVStream        *streamIn;
    AVStream        *streamOut;
    AVCodecContext  *decoder;
    AVCodecContext  *encoder;
    AVFilterGraph   *filterGraph;
    AVFilterContext *filterIn;
    AVFilterContext *filterOut;
    AVIOContext     *ioContext;
    unsigned char   *ioBuffer;
    AVFrame         *frame;
    AVPacket        packetIn;
    AVPacket        packetOut;
    qint64          framePts;
};

#endif // AUDIO_H
