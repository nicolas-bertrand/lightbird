#ifndef IMAGE_H
# define IMAGE_H

# include "IApi.h"
# include "IImage.h"
# include "IPreview.h"
# include "Initialize.h"

/// @brief Implements the IImage and IPreview extensions.
class Image
    : public QObject
    , public LightBird::IImage
    , public LightBird::IPreview
    , public LightBird::Initialize
{
    Q_OBJECT
    Q_INTERFACES(LightBird::IImage
                 LightBird::IPreview)

public:
    Image(LightBird::IApi *api);
    ~Image();

    bool    convert(const QString &source, QString &destination, LightBird::IImage::Format format, unsigned int width = 0, unsigned int height = 0, float quality = -1);
    bool    generate(const QString &source, QString &destination, LightBird::IImage::Format format, unsigned int width = 0, unsigned int height = 0, unsigned int position = 0, float quality = -1);

private:
    /// @brief Checks and returns the quality option [1,100].
    unsigned int _quality(float quality);

    LightBird::IApi *api;         ///< The LightBird API.
    QString         fileTemplate; ///< The template of the temporary files.
};

#endif // IMAGE_H
