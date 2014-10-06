#ifndef UPLOADS_H
# define UPLOADS_H

# include <QDateTime>
# include <QFile>
# include <QMap>
# include <QMutex>
# include <QObject>
# include <QSharedPointer>

# include "IClient.h"
# include "IOnDeserialize.h"

# include "TableFiles.h"

# define MAX_HEADER_LENGTH 4096 ///< The maximum length of a multipart/form-data header.
# define REMOVE_COMPLETE_UPLOAD_TIME 3600 ///< The time after which the completed uploads are removed in second.
# define CANCEL_MAX_CONTENT_LENGTH 100000 ///< The maximum content length of the cancel request.

/// @brief Manages the uploads.
class Uploads : public QObject
{
    Q_OBJECT

public:
    Uploads(QObject *parent = NULL);
    ~Uploads();

    /// @brief Some information on an uploaded file.
    struct File
    {
        QString id;          ///< The id of the file in the database.
        QString name;        ///< The name of the file as it will appear in the database.
        QString path;        ///< The path of the file in the file system including its name (without the filesPath).
        QString contentType; ///< The content type of the file.
    };

    /// @brief Stores the information of an upload.
    struct Upload
    {
        QString     id;             ///< The id of the upload.
        QString     cancelId;       ///< An id given by the client to the upload in order to cancel it.
        qint64      size;           ///< The total size of the upload (content-length).
        qint64      progress;       ///< The number of bytes received so far.
        QString     path;           ///< The virtual destination directory.
        QString     boundary;       ///< The boundary that separates the files in the content (multipart/form-data).
        QString     idClient;       ///< The id of the client that sends the data.
        QString     idAccount;      ///< The id of the account associated with the client.
        bool        header;         ///< If a header is being parsed. Otherwise a file content is being copied.
        bool        complete;       ///< If all the files have been received.
        QList<File> files;          ///< The list of the files received so far (the last one is not complete yet).
        QByteArray  oldData;        ///< A part of the previous piece of content received (boundary size).
        QDateTime   finished;       ///< The date at which the upload has been completed.
        QSharedPointer<QFile> file; ///< The file currently writing.
        LightBird::TableFiles fileTable; ///< The file in the database.
    };

    void    onDeserialize(LightBird::IClient &client, LightBird::IOnDeserialize::Deserialize type, const QString &uri);
    void    onFinish(LightBird::IClient &client);
    void    onDestroy(LightBird::IClient &client);
    /// @brief Cancels the uploads using the cancelId in the content of the request.
    void    cancel(LightBird::IClient &client);

private:
    Uploads(const Uploads &);
    Uploads &operator=(const Uploads &);

    // Uploads files
    /// @brief Starts the download of the files.
    void    _send_onDeserializeHeader(LightBird::IClient &client);
    /// @brief Download the files one by one by reading the multipart/form-data
    /// of the content of the request.
    void    _send_onDeserializeContent(LightBird::IClient &client);
    /// @brief Removes the uploads completed after REMOVE_COMPLETE_UPLOAD_TIME
    /// seconds. This allows to keep their data accessible even after they are finished.
    void    _send_removeCompleteUploads();
    /// @brief Inserts the file in the database and creates it in the file system.
    void    _send_createFile(LightBird::IClient &client, Upload &upload, File &file);
    /// @brief The file has been completly uploaded.
    void    _send_fileComplete(LightBird::IClient &client, Upload &upload);
    /// @brief Get ready for the next file.
    void    _send_clean(Upload &upload);
    /// @brief An error occurred during the upload and the client is disconnected.
    void    _send_error(LightBird::IClient &client, Upload &upload, const QString &error);

    // Cancels the uploads
    void    _cancel_onDeserializeHeader(LightBird::IClient &client);

    QMap<QString, QSharedPointer<Upload> > uploads; ///< List of the upload requests being processed.
    QMutex mutex;///< Makes this->uploads thread safe.
};


#endif // UPLOADS_H
