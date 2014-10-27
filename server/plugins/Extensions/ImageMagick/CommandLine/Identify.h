#ifndef IDENTIDY_H
# define IDENTIDY_H

# include "IApi.h"
# include "IIdentify.h"
# include "Initialize.h"

/// @brief Implements the IIdentify extension.
class Identify
    : public QObject
    , public LightBird::IIdentify
    , public LightBird::Initialize
{
    Q_OBJECT
    Q_INTERFACES(LightBird::IIdentify)

public:
    Identify(LightBird::IApi *api);
    ~Identify();

    bool    identify(const QString &file, LightBird::IIdentify::Information &information);

private:
    LightBird::IApi *api;            ///< The LightBird API.
    QString         imageMagickPath; ///< The path to the ImageMagick binaries.
    QString         binaryName;      ///< The name of the identify tool of ImageMagick.
    unsigned int    timeout;         ///< The maximum duration of the identify program.

    /// @brief Adds information from the image.
    void    _addData(LightBird::IIdentify::Information &information, const QString &key, const QString &value);
};

#endif // IDENTIDY_H
