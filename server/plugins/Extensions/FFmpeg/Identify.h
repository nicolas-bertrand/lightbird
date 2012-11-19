#ifndef IDENTIDY_H
# define IDENTIDY_H

# include "FFmpeg.h"

# include "IApi.h"
# include "IIdentify.h"

/// @brief Implements the IIdentify extension.
class Identify : public LightBird::IIdentify
{
public:
    Identify(LightBird::IApi *api);
    ~Identify();

    bool    identify(const QString &file, LightBird::IIdentify::Information &information);

private:
    /// @brief Opens the best audio or video stream.
    AVStream    *_openStream(AVFormatContext *format, AVCodecContext *&context, AVMediaType type);
    /// @brief Inserts the metadata into the map.
    void        _addMetadata(AVDictionary *metadata, QVariantMap &map);
    /// @brief Returns the string that corresponds to the error code.
    QByteArray  _errorToString(int code);
    /// @brief Returns true if the video stream is an image.
    bool        _isImage(AVStream *videoStream);

    LightBird::IApi *api; ///< The LightBird API.
};

#endif // IDENTIDY_H
