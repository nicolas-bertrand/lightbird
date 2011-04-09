#ifndef IAUDIO_H
# define IAUDIO_H

# include <QString>

/// @brief Allows plugins to convert audio files.
///
/// Many settings can be defined to change the size and quality of the audio
/// to reduce its volume.
/// If some of these settings are not defined, the default ones are used.
/// Notice that the default quality settings are pretty high.
namespace LightBird
{
    class IAudio
    {
    public:
        virtual ~IAudio() {}

        // Controls
        /// @brief Start the transcoding of the source audio into the destination file,
        /// using the defined settings.
        /// @param source : The name of the file that will be transcoded.
        /// @param destination : The name of the file that will contains the converted audio.
        /// If empty, the transcoded audio is keeped in memory (use read to read the transcoded file).
        /// Its extension defines the container format of the new audio (mp3, ogg, wav...).
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
        /// @brief Set the audio codec.
        virtual void    setCodec(const QString &codec) = 0;
        /// @brief Set the bitrate. (values examples : 64K, 128K)
        virtual void    setBitRate(unsigned int bitRate = 192000) = 0;
        /// @brief Set the sample rate. (values examples : 8000, 11025, 22050, 44100)
        virtual void    setFrequency(unsigned int frequency = 48000) = 0;
        /// @brief Seek to given time position in seconds.
        virtual void    setSeek(unsigned int seek = 0) = 0;
        /// @brief Set the duration of the audio in seconds. If 0, the whole audio
        /// is transcoded.
        virtual void    setDuration(unsigned int duration = 0) = 0;
    };
}

#endif // IAUDIO_H
