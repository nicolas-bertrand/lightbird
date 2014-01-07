#ifndef IMAGE_H
# define IMAGE_H

# include "IApi.h"
# include "IImage.h"
# include "IPreview.h"
# include "Initialize.h"

/// @brief Implements the IImage and IPreview extensions.
class Image : public LightBird::IImage,
              public LightBird::IPreview,
              public LightBird::Initialize
{
public:
    Image(LightBird::IApi *api);
    ~Image();

    bool    convert(const QString &source, QString &destination, LightBird::IImage::Format format, unsigned int width = 0, unsigned int height = 0, float quality = -1);
    bool    generate(const QString &source, QString &destination, LightBird::IImage::Format format, unsigned int width = 0, unsigned int height = 0, unsigned int position = 0, float quality = -1);

private:
    /// @brief Returns the size to give to the command line tool.
    QString _resize(unsigned int width, unsigned int height);
    /// @brief Returns the quality option if the quality is positive.
    QString _quality(float quality);

    LightBird::IApi *api;            ///< The LightBird API.
    QString         imageMagickPath; ///< The path to the ImageMagick binaries.
    QString         binaryName;      ///< The name of the convert tool of ImageMagick.
    QString         fileTemplate;    ///< The template of the temporary files.
    unsigned int    timeout;         ///< The maximum duration of the convert program.
};

#endif // IMAGE_H
