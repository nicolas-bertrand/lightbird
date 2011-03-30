#ifndef IVIDEO_H
# define IVIDEO_H

# include <QMap>
# include <QString>

/// @brief Allows plugins to transcode videos.
///
/// Many settings can be defined to change the size and quality of the video
/// and the audio, in order to reduce its volume.
/// If some of these settings are not defined, the default ones are used.
/// Notice that the default quality settings are pretty high.
namespace Streamit
{
    class IVideo
    {
    public:
        virtual ~IVideo() {}

        // Controls
        /// @brief Start the transcoding of the source video into the destination file,
        /// using the defined settings.
        /// @param source : The name of the file that will be transcoded.
        /// @param destination : The name of the file that will contains the converted video.
        /// If empty, the transcoded video is keeped in memory (use read to read the transcoded file).
        /// Its extension defines the container format of the new video (avi, mkv, mp4...).
        /// @param format : If the destination is empty, this parameter represents the format of the file.
        /// @return True if the conversion has started correctly, or false if an error occured.
        virtual bool    start(const QString &source, const QString &destination, const QString &format = "") = 0;
        /// @brief Read at most maxRead bytes from the convertion stream, even if the
        /// conversion is not finished. This method only work when the destination is empty.
        /// @param maxRead : The maximum number of bytes to read.
        /// @return The data read.
        virtual QByteArray read(unsigned int maxRead) = 0;
        /// @brief Wait until some data are available for read in the conversion stream.
        /// @param msecs : The time out. If msecs is -1, this function will not time out.
        /// @return Returns true if data are available. Otherwise returns false (if
        /// the operation timed out or if an error occurred).
        virtual bool    waitForRead(int msecs = -1) = 0;
        /// @brief Wait until the conversion has finished, or until msecs milliseconds have passed.
        /// @param msecs : The time out. If msecs is -1, this function will not time out.
        /// @return Returns true if the conversion was finished successfully. Otherwise
        /// returns false (if the operation timed out or if an error occurred).
        virtual bool    waitForFinished(int msecs = -1) = 0;
        /// @brief Returns true if the conversion is finished.
        virtual bool    isFinished() = 0;
        /// @brief Stop the conversion.
        virtual void    stop() = 0;
        /// @brief If the conversion is finished, returns true if the file has been
        /// successfully transcoded. Otherwise returns false.
        virtual bool    result() = 0;

        // Settings
        /// @brief Set the width of the destination video. If width is not defined
        /// and height is set, width is kept proportional to height.
        virtual void    setWidth(unsigned int width) = 0;
        /// @brief Set the height of the destination video. If height is not defined
        /// and width is set, height is kept proportional to width.
        virtual void    setHeight(unsigned int height) = 0;
        /// @brief Set the video codec.
        virtual void    setVideoCodec(const QString &codec) = 0;
        /// @brief Set the audio codec.
        virtual void    setAudioCodec(const QString &codec) = 0;
        /// @brief Set the video bitrate. The bitrate defined the quality of the video.
        /// (values examples : 200K)
        virtual void    setVideoBitRate(unsigned int bitRate = 1000000) = 0;
        /// @brief Set the audio bitrate. (values examples : 64K, 128K)
        virtual void    setAudioBitRate(unsigned int bitRate = 192000) = 0;
        /// @brief Set the video frame per second.
        virtual void    setVideoFrameRate(unsigned int frameRate = 25) = 0;
        /// @brief Set the audio sample rate. (values examples : 8000, 11025, 22050, 44100)
        virtual void    setAudioFrequency(unsigned int frequency = 48000) = 0;
        /// @brief Seek to given time position in seconds.
        virtual void    setSeek(unsigned int seek = 0) = 0;
        /// @brief Set the duration of the video in seconds. If 0, the whole video
        /// is transcoded.
        virtual void    setDuration(unsigned int duration = 0) = 0;
        /// @brief Some options that depends on the back end.
        virtual void    setOptions(const QMap<QString, QString> &options) = 0;
    };
}

#endif // IVIDEO_H
