#ifndef	LOG_H
# define LOG_H

# include <QDateTime>
# include <QMutex>
# include <QThread>
# include <QWaitCondition>

# include "ILogs.h"

# include "Properties.h"

/// @brief Writes log entries.
///
/// If a plugin implements ILog, the entry is passed to it.
/// If the display mode of the configuration is at true, the logs are written on the standard output.
/// Handles several levels that can be used to set the importance of the logs. If the level of a
/// log is lower than this->level, the log will not be written. For example, to write an info, call
/// "Log::info("Your log");". The logs are written in a separate thread (except for the standard output)
/// to ensure that they will not slow down the thread from which they are launched. Indeed, logs can
/// be very slow because they are usually sent throught the network, or wrote in a file via plugins.
class Log : public QThread
{
    Q_OBJECT

public:
    /// @brief The available modes of the log.
    enum Mode
    {
        BUFFER, ///< The logs are buffered in a member variable.
        WRITE,  ///< The logs are wrote in a plugin and on the standard output.
        PRINT   ///< The logs are printed on the standard output.
    };

    static Log  *instance(QObject *parent = 0);

    /// @brief The generic method that writes all the logs. Emit a signal that write the log in the
    /// log thread. This method is called by the other public methods.
    /// @param level : The level of the current log.
    /// @param message : The message of the log.
    /// @param properties : A map of properties that can give additionals informations on the log.
    /// @param plugin : The name of the plugin from witch the log is written, or empty for the server.
    /// @param object : The name of the class from witch the log is written.
    /// @param method : The name of the method from witch the log is written.
    void        write(LightBird::ILogs::Level level, const QString &message, const QString &plugin, const Properties &properties, const QString &object, const QString &method);
    /// @brief Writes a fatal log.
    static void fatal(const QString &message, const QString &object = "", const QString &method = "");
    /// @brief Writes a fatal log.
    static void fatal(const QString &message, const Properties &properties, const QString &object = "", const QString &method = "");
    /// @brief Writes an error log.
    static void error(const QString &message, const QString &object = "", const QString &method = "");
    /// @brief Writes an error log.
    static void error(const QString &message, const Properties &properties, const QString &object = "", const QString &method = "");
    /// @brief Writes a warning log.
    static void warning(const QString &message, const QString &object = "", const QString &method = "");
    /// @brief Writes a warning log.
    static void warning(const QString &message, const Properties &properties, const QString &object = "", const QString &method = "");
    /// @brief Writes an info log.
    static void info(const QString &message, const QString &object = "", const QString &method = "");
    /// @brief Writes an info log.
    static void info(const QString &message, const Properties &properties, const QString &object = "", const QString &method = "");
    /// @brief Writes a debug log.
    static void debug(const QString &message, const QString &object = "", const QString &method = "");
    /// @brief Writes a debug log.
    static void debug(const QString &message, const Properties &properties, const QString &object = "", const QString &method = "");
    /// @brief Writes a trace log.
    static void trace(const QString &message, const QString &object = "", const QString &method = "");
    /// @brief Writes a trace log.
    static void trace(const QString &message, const Properties &properties, const QString &object = "", const QString &method = "");
    LightBird::ILogs::Level getLevel() const;
    void    setLevel(LightBird::ILogs::Level level);
    bool    isDisplay() const;
    void    isDisplay(bool display);
    bool    isError() const;
    bool    isWarning() const;
    bool    isInfo() const;
    bool    isDebug() const;
    bool    isTrace() const;

    /// @brief Set the mode of the log.
    void        setMode(Mode mode);
    /// @brief Displays all the logs of the buffer in the standard output.
    void        print();
    /// @brief The main method of the thread. The logs are wrote in the event loop of this method.
    void        run();

signals:
    /// @brief This signal is emited to write a log.
    void        writeLog(LightBird::ILogs::Level level, const QDateTime &date, const QString &message, const Properties &properties, const QString &thread, const QString &plugin, const QString &object, const QString &method);

private:
    Log(QObject *parent = 0);
    ~Log();
    Log(const Log &);
    Log &operator=(const Log &);

    /// @brief Store all the informations used in a log entry.
    struct LogInformations
    {
        LogInformations(LightBird::ILogs::Level level, const QDateTime &date, const QString &message, const Properties &properties, const QString &thread, const QString &plugin, const QString &object, const QString &method);
        LightBird::ILogs::Level level;
        QString                 message;
        Properties              properties;
        QString                 plugin;
        QString                 object;
        QString                 method;
        QString                 thread;
        QDateTime               date;
    };

    /// @brief Initialize the WRITE mode.
    void        _initializeWrite();
    /// @brief Convert a QMap to a QString. The Entries are separate by ", "
    /// @return The result of the conversion.
    QString     _mapToString(const QMap<QString, QString> &properties) const;
    /// @brief Print the log in parameter on the standard output.
    /// @see write
    void        _print(LightBird::ILogs::Level level, const QDateTime &date, const QString &message, const Properties &properties, const QString &thread, const QString &plugin, const QString &object, const QString &method) const;

private slots:
    /// @brief Write a log to the plugins that implements ILog.
    void        _write(LightBird::ILogs::Level level, const QDateTime &date, const QString &message, const Properties &properties, const QString &thread, const QString &plugin, const QString &object, const QString &method);

private:
    LightBird::ILogs::Level                 level;      ///< The minimum level required to write a log.
    static Log                              *_instance; ///< The instance of the Singleton.
    QMap<LightBird::ILogs::Level, QString>  levels;     ///< Combines the levels and their names.
    mutable QMutex                          mutex;      ///< Ensures that only one log is written at the same time.
    QList<LogInformations>                  buffer;     ///< A buffer that stores the logs that haven't been saved yet.
    Mode                                    mode;       ///< The current mode of the log.
    bool                                    display;    ///< If the logs have to be displayed in the standard output.
    QWaitCondition                          waitRun;    ///< This condition is awakened when the thread is running for the first time.
    QMutex                                  waitMutex;  ///< Used by QWaitCondition.
    bool                                    awake;      ///< If the wait condition has been called.
    QObject                                 *parent;    ///< The parent of the Log.
};

#endif // LOG_H
