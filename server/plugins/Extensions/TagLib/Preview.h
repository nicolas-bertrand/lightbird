#ifndef PREVIEW_H
# define PREVIEW_H

# include "IApi.h"
# include "IPreview.h"

/// @brief Implements the IPreview extension.
class Preview
    : public QObject
    , public LightBird::IPreview
{
    Q_OBJECT
    Q_INTERFACES(LightBird::IPreview)

public:
    Preview(LightBird::IApi *api);
    ~Preview();

    bool    generate(const QString &source, QString &destination, LightBird::IImage::Format format = LightBird::IImage::NONE, unsigned int width = 0, unsigned int height = 0, unsigned int position = 0, float quality = -1);

private:
    LightBird::IApi *api;         ///< The LightBird API.
    QString         fileTemplate; ///< The template of the temporary files.
};

#endif // PREVIEW_H
