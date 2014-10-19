#ifndef FILE_H
# define FILE_H

# include <QDir>
# include <QFile>
# include <QMap>
# include <QMutex>
# include <QObject>
# include <QString>
# include <QStringList>

# include "ILog.h"
# include "ILogs.h"
# include "IPlugin.h"
# include "ITimer.h"

class File : public QObject,
             public LightBird::IPlugin,
             public LightBird::ILog,
             public LightBird::ITimer
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "cc.lightbird.Log.File")
    Q_INTERFACES(LightBird::IPlugin
                 LightBird::ILog
                 LightBird::ITimer)

public:
    File();
    ~File();

    // IPlugin
    bool        onLoad(LightBird::IApi *api);
    void        onUnload();
    bool        onInstall(LightBird::IApi *api);
    void        onUninstall(LightBird::IApi *api);
    void        getMetadata(LightBird::IMetadata &metadata) const;

    // ILog
    /// @brief The logs are buffered when this method is called. Then they are
    /// wrote in the file in the timer.
    void        log(LightBird::ILogs::Level level, const QDateTime &date, const QString &message,
                    const QMap<QString, QString> &properties, const QString &thread,
                    const QString &plugin, const QString &object, const QString &method);

    // ITimer
    /// @brief Writes the logs in the file periodically.
    bool        timer(const QString &name);

private:
    /// @brief Open the log file with the appropriate attributes.
    /// @return If the log file has been correctly opened.
    bool        _createLogFile();
    /// @brief Manages the files that are used by the log manager, which includes the
    /// current log file and its archives.
    bool        _manageFiles();
    /// @brief Converts a QMap to a QString. The Entries are separate by ", "
    /// @return The result of the conversion.
    QString     _mapToString(const QMap<QString, QString> &properties);


    LightBird::IApi     *api;                       ///< The LightBird API.
    QString             path;                       ///< The path of the log directory.
    QString             name;                       ///< The name of the log file.
    qint64              maxSize;                    ///< The maximum size of the log file in byte. When it exceeds this size, an archive file is created and the log file is gutted.
    int                 expires;                    ///< The maximum number of days during witch the archives of the log file are kept.
    int                 maxNbOfFile;                ///< The maximum number of files in the log directory.
    bool                display;                    ///< If the logs have to be displayed on the standard output
    QDir                directory;                  ///< The log directory.
    QFile               file;                       ///< The log file.
    QString             lastError;                  ///< Date of the last error (used to avoid repetitions).
    QMutex              mutex;                      ///< Makes this class thread safe.
    QStringList         buffer;                     ///< Buffered the logs to write in the timer.
    QMap<LightBird::ILogs::Level, QString> levels;  ///< Combines the levels and their names.
};

#endif // FILE_H
