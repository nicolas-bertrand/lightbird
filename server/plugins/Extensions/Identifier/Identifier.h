#ifndef IDENTIDIER_H
# define IDENTIDIER_H

# include "IApi.h"
# include "IIdentifier.h"
# include "IMime.h"

// The number of bytes read each time to compute the hashes of the file
# define READ_FILE_SIZE 1000000

/// @brief Implements the IIdentifier and IMime extensions.
class Identifier : public LightBird::IIdentifier,
                   public LightBird::IMime
{
public:
    Identifier(LightBird::IApi &api);
    ~Identifier();

    LightBird::IIdentify::Information   identify(const QString &file);
    QString                             getMime(const QString &file);

private:
    LightBird::IApi &api;         ///< The LightBird API.
    QList<QString>  mimeDocument; ///< List the possible MIME type of the documents.
    qint64          maxSizeHash;  ///< The hashes is not calculated for files whose size exceeds this value.
    typedef LightBird::IIdentify::Information Info;

    void    _identify(QMap<LightBird::IIdentify::Type, QVariantMap> info, Info &result);
    bool    _add(LightBird::IIdentify::Type type, QMap<LightBird::IIdentify::Type, QVariantMap> info, Info &result);
    void    _document(Info &result);
    void    _hash(const QString &file, Info &result);
};

#endif // IDENTIDIER_H
