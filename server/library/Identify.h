#ifndef IDENTIFY_H
# define IDENTIFY_H

# include <QMutex>
# include <QStringList>
# include <QThread>

# include "IIdentify.h"

// The number of bytes read each time to compute the hashes of the file
# define READ_FILE_SIZE 1000000

/// @see LightBird::identify
class Identify
{
public:
    Identify();
    ~Identify();

    /// @see LightBird::identify
    void    identify(const QString &file, LightBird::IIdentify::Information *information = NULL);

private:
    typedef LightBird::IIdentify::Information Info;

    LightBird::IIdentify::Information _identify(const QString &file);
    void    _identify(QMap<LightBird::IIdentify::Type, QVariantMap> info, Info &result);
    bool    _add(LightBird::IIdentify::Type type, QMap<LightBird::IIdentify::Type, QVariantMap> info, Info &result);
    void    _document(Info &result);
    void    _typeFromMime(Info &result);
    void    _hash(const QString &file, Info &result);

    /// @brief This thread identifies the files in the list into the the databse.
    struct Thread : public QThread
    {
        void    run();
    };

    qint64         maxSizeHash;  ///< The hashes are not computed for files whose size exceeds this value.
    QList<QString> mimeDocument; ///< List the possible MIME type of the documents.
    QStringList    files;        ///< The list of the thread to identify.
    Thread         *thread;      ///< Identifies the files in the database.
    QMutex         mutex;        ///< Makes the class thread safe.
    QHash<LightBird::IIdentify::Type, QString> typeString; ///< Associates the files types to their string.
};

#endif // IDENTIFY_H
