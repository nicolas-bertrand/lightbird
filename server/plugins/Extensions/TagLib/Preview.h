#ifndef PREVIEW_H
# define PREVIEW_H

# include "IApi.h"
# include "IPreview.h"

/// @brief Implements the IPreview extension.
class Preview : public LightBird::IPreview
{
public:
    Preview(LightBird::IApi *api);
    ~Preview();

    bool    generate(const QString &source, QString &destination, LightBird::IImage::Format format, unsigned int width = 0, unsigned int height = 0, unsigned int position = 0);

private:
    LightBird::IApi *api;         ///< The LightBird API.
    QString         fileTemplate; ///< The template of the temporary files.
};

#endif // PREVIEW_H
