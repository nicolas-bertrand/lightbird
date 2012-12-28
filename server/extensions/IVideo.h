#ifndef LIGHTBIRD_IVIDEO_H
# define LIGHTBIRD_IVIDEO_H

# include <QByteArray>
# include <QVariantHash>

namespace LightBird
{
    /// @brief Allows to transcode videos.
    ///
    /// Settings can be applied to change the size and quality of the video
    /// and audio streams, in order to reduce their volume.
    /// If some of these settings are not defined, they will copy the ones of
    /// the source, in order to keep its quality.
    class IVideo
    {
    public:
        virtual ~IVideo() {}

        /// @brief Initializes the transcoding session.
        /// Must be called before transcode().
        /// @param source : The video that will be transcoded.
        /// @return True if the source is valid and can be transcoded.
        virtual bool    initialize(const QString &source) = 0;
        /// @brief Transcodes approximately one second of the source video
        /// using the settings, and returns the result. The initialize method
        /// and the settings must be called before. After the first call to
        /// this method, the settings can't be changed. The clear method can be
        /// called at any time to abort the transcoding and start a new one.
        /// This method can be called as long as data are being returned.
        /// @return The transcoded data. Empty when the transcoding is finished.
        virtual QByteArray transcode() = 0;
        /// @brief Returns the number of seconds transcoded so far.
        virtual int     getTime() = 0;
        /// @brief Sets all settings to their default values, and free all the
        /// data allocated so far.
        virtual void    clear() = 0;

        // Settings
        /// @brief Sets the output format.
        /// @example webm, ogg, mp4, avi
        /// @return True if the format is supported.
        virtual bool    setFormat(const QString &format) = 0;
        /// @brief Sets the video codec.
        /// @example vp8, theora, h264
        /// @return True if the codec is supported.
        virtual bool    setVideoCodec(const QString &codec) = 0;
        /// @brief Sets the audio codec.
        /// @example vorbis, aac, mp3
        /// @return True if the codec is supported.
        virtual bool    setAudioCodec(const QString &codec) = 0;
        /// @brief Sets the width of the video. If the value is 0, the width
        /// of the source is used. If the value is -1, the aspect ratio of the
        /// source is maintained.
        virtual void    setWidth(int width) = 0;
        /// @brief Sets the height of the video. If the value is 0, the height
        /// of the source is used. If the value is -1, the aspect ratio of the
        /// source is maintained.
        virtual void    setHeight(int height) = 0;
        /// @brief Sets the video bit rate, which controls the quality of the
        /// video in bits per second.
        /// @example 200K, 1M, 2M
        virtual void    setVideoBitRate(unsigned int bitRate) = 0;
        /// @brief Sets the audio bit rate, which controls the quality of the
        /// audio in bits per second.
        /// @example 64K, 128K, 192K
        virtual void    setAudioBitRate(unsigned int bitRate) = 0;
        /// @brief Sets the number of frames per second of the video.
        /// @example 25
        virtual void    setFrameRate(unsigned int frameRate) = 0;
        /// @brief Sets the number of samples per second of the audio.
        /// @example 8000, 11025, 22050, 44100, 48000
        virtual void    setSampleRate(unsigned int sampleRate) = 0;
        /// @brief Sets the number of audio channels.
        /// @example 2 for stereo, 1 for mono, 6 for 5.1
        virtual void    setChannels(unsigned int channels) = 0;
        /// @brief Sets the position in the source where the transcoding will start,
        /// in seconds.
        virtual void    setStart(double start) = 0;
        /// @brief Sets the duration of the video in seconds. The whole
        /// video is transcoded if the value is 0.
        virtual void    setDuration(double duration) = 0;
        /// @brief Some options that depends on the back end.
        virtual void    setOptions(const QVariantHash &options) = 0;
    };
}

#endif // LIGHTBIRD_IVIDEO_H
