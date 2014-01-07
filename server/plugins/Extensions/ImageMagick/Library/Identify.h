#ifndef IDENTIDY_H
# define IDENTIDY_H

# include <Magick++.h>
# include <magick/colorspace.h>

# include "IApi.h"
# include "IIdentify.h"
# include "Initialize.h"

/// @brief Implements the IIdentify extension.
class Identify : public LightBird::IIdentify,
                 public LightBird::Initialize
{
public:
    Identify(LightBird::IApi *api);
    ~Identify();

    bool    identify(const QString &file, LightBird::IIdentify::Information &information);

private:
    LightBird::IApi *api; ///< The LightBird API.

    QMap<MagickCore::ColorspaceType, QString> colorSpace;
    QMap<MagickCore::CompressionType, QString> compression;
};

#endif // IDENTIDY_H
