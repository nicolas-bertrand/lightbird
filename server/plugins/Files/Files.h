#ifndef FILES_H
# define FILES_H

# include <QDateTime>
# include <QFileSystemWatcher>
# include <QMap>
# include <QMutex>
# include <QObject>

# include "IApi.h"

class Files : public QObject
{
    Q_OBJECT

public:
    Files(LightBird::IApi &api, const QString &timerName);
    ~Files();

    /// @brief Called periodically to check if some file can be removed from the
    /// file system or added to the database.
    void    timer();

private slots:
    /// @brief Called when a file is added in the filesPath.
    void    _directoryChanged(const QString &path);

private:
    struct File
    {
        unsigned int    size; ///< The size of the file.
        QDateTime       date; ///< The date of the size.
    };

    LightBird::IApi     &api;
    QMutex              mutex;             ///< Makes this class thread safe.
    QFileSystemWatcher  fileSystemWatcher; ///< Tells when a file is modified in the files directory.
    QString             timerName;         ///< The name of the timer that manages the thread of this class.
    QMap<QString, File> newFiles;          ///< The list of the new files that may be added to the database if their size don't change.
    QDateTime           removeFileDate;    ///< The last time we checked for files to remove.
    unsigned int        removeFileTimer;   ///< The time to check and remove the files that could not be removed before from the filesPath.
    unsigned int        directoryChangedTimer; ///< The time to wait before a file added in the filesPath is added to the database.
};

#endif // FILES_H
