#ifndef UPLOADS_H
# define UPLOADS_H

# include <QDateTime>
# include <QFile>
# include <QMap>
# include <QMutex>
# include <QObject>
# include <QSharedPointer>

# include "IClient.h"

# include "TableFiles.h"

# define MAX_HEADER_LENGTH 4096 ///< The maximum length of a multipart/form-data header.
# define REMOVE_COMPLETE_UPLOAD_TIME 3600 ///< The time after which the completed uploads are removed in second.

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
        QString name;        ///< The name of the file as it will appear in the database.
        QString path;        ///< The path of the file in the file system including its name (without the filesPath).
        QString contentType; ///< The content type of the file.
    };

    /// @brief Stores the information of an upload.
    struct Upload
    {
        QString     id;             ///< The id of the upload.
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

    /// @brief Starts the download of the files.
    void    onDeserializeHeader(LightBird::IClient &client);
    /// @brief Download the files one by one by reading the multipart/form-data
    /// of the content of the request.
    void    onDeserializeContent(LightBird::IClient &client);
    /// @brief Starts the identification of the files in a timer thread.
    void    doExecution(LightBird::IClient &client);
    /// @brief Identifies the files in the identification queue.
    bool    timer();
    /// @brief Returns the list of the files in the directory in order to allow
    /// the client to check that the files that will be uploaded are not already
    /// on the server.
    void    check(LightBird::IClient &client);
    /// @brief Returns the number of bytes downloaded so far, in JSON.
    void    progress(LightBird::IClient &client);
    /// @brief Stops the upload and removes the current file.
    void    stop(LightBird::IClient &client);
    /// @brief Cancels the upload and removes all the files uploaded so far.
    /// The files are removed even if the upload is already finished.
    void    cancel(LightBird::IClient &client);
    void    onFinish(LightBird::IClient &client);
    void    onDestroy(LightBird::IClient &client);

private:
    Uploads(const Uploads &);
    Uploads &operator=(const Uploads &);

    /// @brief Removes the uploads completed after REMOVE_COMPLETE_UPLOAD_TIME
    /// seconds. This allows to keep their data accessible by the progress
    /// method even after they are finished.
    void    _removeCompleteUploads();
    /// @brief Inserts the file in the database and creates it in the file system.
    void    _createFile(LightBird::IClient &client, Upload &upload, File &file);
    /// @brief The file has been completly uploaded.
    void    _fileComplete(LightBird::IClient &client, Upload &upload);
    /// @brief Get ready for the next file.
    void    _clean(Upload &upload);
    /// @brief An error occured during the upload and the client is disconnected.
    void    _error(LightBird::IClient &client, Upload &upload, const QString &error);

    QMap<QString, Upload>   uploads;         ///< List of the upload requests being processed.
    qint64                  maxHeaderLength; ///< The maximum length of the headers in the content of the request.
    QMutex                  mutex;           ///< Makes this class thread safe.
};


#endif // UPLOADS_H
