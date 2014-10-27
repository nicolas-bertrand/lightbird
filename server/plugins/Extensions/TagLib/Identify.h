#ifndef IDENTIDY_H
# define IDENTIDY_H

# include "IApi.h"
# include "IIdentify.h"

# include <mpegfile.h>
# include <fileref.h>

/// @brief Implements the IIdentify extension.
class Identify
    : public QObject
    , public LightBird::IIdentify
{
    Q_OBJECT
    Q_INTERFACES(LightBird::IIdentify)

public:
    Identify(LightBird::IApi *_api);
    ~Identify();

    bool    identify(const QString &file, LightBird::IIdentify::Information &information);

private:
    LightBird::IApi         *_api;
    QMap<QString, QString>  _id3v2;

    void    _addTags(LightBird::IIdentify::Information &information, TagLib::FileRef &file);
    void    _addID3v2Data(LightBird::IIdentify::Information &information, TagLib::MPEG::File &file);
    void    _addID3v1Data(LightBird::IIdentify::Information &information, TagLib::MPEG::File &file);
    void    _addApe(LightBird::IIdentify::Information &information, TagLib::MPEG::File &file);
    void    _addData(LightBird::IIdentify::Information &information, const QString &key, const QVariant &value);
};

#endif // IDENTIDY_H
