#ifndef UPLOADS_H
# define UPLOADS_H

# include <QFile>
# include <QMap>
# include <QMutex>
# include <QObject>
# include <QSharedPointer>

# include "IClient.h"

# define MAX_HEADER_LENGTH 4096 ///< The maximum length of a multipart/form-data header.

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
        QString path;        ///< The real path of the file including its real name.
        QString contentType; ///< The content type of the file.
    };

    /// @brief Stores the information of an upload.
    struct Upload
    {
        qint64      size;           ///< The total size of the upload (content-length)
        qint64      progress;       ///< The number of bytes received so far.
        QString     path;           ///< The virtual destination directory.
        QString     boundary;       ///< The boundary that separates the files in the content (multipart/form-data).
        QString     clientId;       ///< The id of the client that sends the data.
        bool        header;         ///< If a header is being parsed. Otherwise a file content is being copied.
        bool        complete;       ///< If all the files have been received.
        QList<File> files;          ///< The list of the files received so far (the last one is not complete yet).
        QByteArray  oldData;        ///< A part of the previous piece of content received (boundary size).
        QSharedPointer<QFile> file; ///< The file currently writing.
    };

    /// @brief Starts the download of the files.
    void    onUnserializeHeader(LightBird::IClient &client);
    /// @brief Download the files one by one by reading the multipart/form-data of the content of the request.
    void    onUnserializeContent(LightBird::IClient &client);
    /// @brief Starts the identification of the files in a timer thread.
    void    doExecution(LightBird::IClient &client);
    /// @brief Identifies the files in the identification queue.
    bool    timer();
    void    onFinish(LightBird::IClient &client);
    void    onDestroy(LightBird::IClient &client);

private:
    Uploads(const Uploads &);
    Uploads &operator=(const Uploads &);

    /// @brief Insert the file in the database.
    void    _insert(LightBird::IClient &client, Upload &upload);
    void    _clean(Upload &upload);
    void    _error(LightBird::IClient &client, Upload &upload);

    QMap<QString, Upload>   uploads;         ///< List of the upload requests being processed.
    qint64                  maxHeaderLength; ///< The maximum length of the headers in the content of the request.
    QStringList             identify;        ///< The list of the files waiting to be identified.
    QMutex                  mutex;           ///< Makes this class thread safe.
};


#endif // UPLOADS_H
