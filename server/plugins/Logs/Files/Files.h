#ifndef FILES_H
# define FILES_H

# include <QObject>
# include <QFile>
# include <QDir>

# include "IPlugin.h"
# include "ILog.h"

class Files : public QObject,
              public LightBird::IPlugin,
              public LightBird::ILog
{
    Q_OBJECT
    Q_INTERFACES(LightBird::IPlugin LightBird::ILog)

public:
    Files();
    ~Files();

    // IPlugin
    bool    onLoad(LightBird::IApi *api);
    void    onUnload();
    bool    onInstall(LightBird::IApi *api);
    void    onUninstall(LightBird::IApi *api);
    void    getMetadata(LightBird::IMetadata &metadata) const;

    // ILog
    void    log(LightBird::ILogs::level level, const QDateTime &date, const QString &message,
                const QMap<QString, QString> &properties, const QString &thread,
                const QString &plugin, const QString &object, const QString &method);

private:
    /**
     * @brief Return the value of the node in parameter from the configuration
     * of the server, or the plugin.
     */
    QString _getNodeValue(const QString &nodeName);
    /**
     * @brief Converts a string to a int using the caractere at the end of the string.
     * This caractere could be K (Kilobyte), M (Megabyte), or G (Gigabyte). For example
     * "42K" is converted to 43008 (42 * 1024), and "42" is converted to 42.
     * @param string : The string to convert.
     * @return The result of the conversion. The minumum value returned is 1.
     */
    int     _toByte(const QString &string);
    /**
     * @brief Open the log file with the appropriate attributes.
     * @return If the log file has been correctly opened.
     */
    bool     _createLogFile();
    /**
     * @brief Manages the files that are used by the log manager, which includes the
     * current log file and its archives.
     */
    bool    _manageFiles();
    /**
     * @brief Converts a QMap to a QString. The Entries are separate by ", "
     * @return The result of the conversion.
     */
    QString _mapToString(const QMap<QString, QString> &properties);


    LightBird::IApi     *api;                       ///< The LightBird API.
    QString             path;                       ///< The path of the log directory.
    QString             name;                       ///< The name of the log file.
    int                 maxSize;                    ///< The maximum size of the log file in byte. When it exceeds this size, an archive file is created and the log file is gutted.
    int                 validityPeriod;             ///< The maximum number of days during witch the archives of the log file are kept.
    int                 maxNbOfFile;                ///< The maximum number of files in the log directory.
    bool                display;                    ///< If the logs have to be displayed on the standard output
    QDir                directory;                  ///< The log directory.
    QFile               file;                       ///< The log file.
    QString             lastError;                  ///< Date of the last error (to avoid repetitions).
    QMap<LightBird::ILogs::level, QString> levels;   ///< Combines the levels and their names.
};

#endif // FILES_H
