#ifndef PREVIEWS_H
# define PREVIEWS_H

# include <QHash>
# include <QMutex>

# include "IImage.h"

# include "TableFiles.h"

/// @see LightBird::preview
class Preview
{
public:
    Preview();
    ~Preview();

    /// @see LightBird::preview
    QString generate(const QString &fileId, LightBird::IImage::Format format, unsigned int width = 0, unsigned int height = 0, unsigned int position = 0, float quality = -1);

private:
    Preview(const Preview &);
    Preview &operator=(const Preview &);

    /// @brief Stores the parameters required to generate the preview.
    struct Parameters
    {
        QString      previewFileName; ///< The name of the preview file.
        QString      cacheFileName;   ///< The name of the file in the cache.
        unsigned int width;           ///< The width of the preview.
        unsigned int height;          ///< The height of the preview.
        QString      position;        ///< The position of the preview.
        QString      extension;       /// The extension of the format.
        LightBird::TableFiles file;   ///< The source file.
        LightBird::IImage::Format format; ///< The format of the preview.
    };

    /// @brief Returns the size of the image depending on the size requested by the caller.
    void    _size(Parameters &params);
    /// @brief Tries to use the cache instead to generate a new preview.
    bool    _useCache(Parameters &params);
    /// @brief Moves the temporary file to the cache.
    bool    _move(Parameters &params);
    /// @brief Manages the cache.
    void    _cache();

    bool         cacheEnabled;   ///< True if the cache is enabled.
    QString      cachePath;      ///< The path to the directory where is stored the cache.
    unsigned int cacheSizeLimit; ///< Maximum size of the cache directory.
    QMutex       mutex;          ///< Ensure that the management of the cache is atomic.
};

#endif // PREVIEWS_H
