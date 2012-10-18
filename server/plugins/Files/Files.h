#ifndef FILES_H
# define FILES_H

# include <QFileSystemWatcher>
# include <QMap>
# include <QMutex>
# include <QObject>
# include <QTimer>

# include "IApi.h"

class Files : public QObject
{
    Q_OBJECT

public:
    Files(LightBird::IApi *api, const QString &timerName);
    ~Files();

    /// @brief Called periodically to check if some file can be removed from the
    /// file system.
    void        timer();

private slots:
    /// @brief Called when something changed in the filesPath.
    void        _directoryChanged(const QString &path);
    /// @brief Adds the files added in the filesPath to the database.
    void        _addFiles();

private:
    /// @brief Removes the files that could not be removed before.
    /// @return The list of the files that could not be removed this time.
    QStringList _removeFiles();

    LightBird::IApi     *api;
    QFileSystemWatcher  fileSystemWatcher; ///< Tells when something has changed in the filesPath.
    QString             timerName;         ///< The name of the timer that manages the thread of this class.
    unsigned int        removeFileTimer;   ///< The time to check and remove the files that could not be removed before from the filesPath.
    QTimer              addFiles;          ///< Delays the call to _addFiles so that the files removed in the filesPath are not added.
};

#endif // FILES_H
