#ifndef IDENTIFY_H
# define IDENTIFY_H

# include <QMutex>
# include <QObject>
# include <QStringList>
# include <QThread>

# include "IIdentify.h"
# include "TableFiles.h"

// The number of bytes read each time to compute the hashes of the file
# define READ_FILE_SIZE 1000000

/// @see LightBird::identify
class Identify : public QObject
{
    Q_OBJECT

public:
    Identify();
    ~Identify();

    /// @see LightBird::identify
    void    identify(const QString &fileId);
    /// @see LightBird::identify
    void    identify(const QString &filePath, LightBird::IIdentify::Information &information);

private slots:
    /// @brief Deletes the thread when it is finished.
    void    finished();

private:
    typedef LightBird::IIdentify::Information Info;
    typedef void (Identify::*Method)(LightBird::TableFiles &, Identify::Info &);

    /// @brief Calls _identify and insert the result in the database.
    void    _identifyThread(LightBird::TableFiles &file, Identify::Info &information);
    /// @brief Calls _hash and puts the result in the database.
    void    _hashThread(LightBird::TableFiles &file, Identify::Info &information);
    /// @brief Identifies the file by calling the IIdentify interfaces.
    /// @param computeHash : True if the hashes of the file have to be computed.
    Info    _identify(const QString &file, const QString &fileName, bool computeHash);
    void    _identify(QMap<LightBird::IIdentify::Type, QVariantMap> info, Info &result);
    bool    _add(LightBird::IIdentify::Type type, QMap<LightBird::IIdentify::Type, QVariantMap> info, Info &result);
    void    _document(Info &result);
    void    _typeFromMime(Info &result);
    /// @brief Computes the SHA1 and the MD5 of the file.
    void    _hash(const QString &file, Info &result);

    /// @brief Identifies or hashes the files in a dedicated thread.
    struct Thread : public QThread
    {
        void        run();

        QStringList files;    ///< The files to identify or hash.
        Thread      **thread; ///< Points to one of the thread members in Identify.
        Method      method;   ///< The method that identifies of computes the hashes of the files.
    };

    qint64         maxSizeHash;     ///< The hashes are not computed for the files whose size exceeds this value.
    QList<QString> mimeDocument;    ///< List the possible MIME type of the documents.
    Thread         *identifyThread; ///< Identifies the files and puts the result in the database.
    Thread         *hashThread;     ///< Computes the hashes of the files.
    QMutex         mutex;           ///< Makes the class thread safe.
    QHash<LightBird::IIdentify::Type, QString> typeString; ///< Associates the files types to their string.
};

#endif // IDENTIFY_H
