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
/// If a plugin implements LightBird::ILog, the entry is passed to it.
/// If the display node of the configuration is at true, the logs are written
/// on the standard output.
/// Handles several levels that can be used to set the importance of the logs.
/// If the level of a log is lower than this->level, the log will not be
/// written. For example, to write an info, call "Log::info("Your log");".
/// The logs are written in a separate thread (except for the standard output)
/// to ensure that they will not slow down the thread from which they are throwed.
/// Indeed, logs can be very slow because they are usually sent through the
/// network, or wrote in a file via the plugins.
class Log : public QThread
{
    Q_OBJECT

public:
    /// @brief The states of the logs, which depend on the state of the server.
    enum State
    {
        BEGIN,         ///< The logs are the first thing loaded, so they are just buffered for now.
        CONFIGURATION, ///< The configuration have been loaded, so we can print the logs on the standard output while buffering them.
        PLUGIN,        ///< The plugins have been loaded, so the logs are written to LightBird::ILog and printed on the standard output.
        END            ///< The logs are only printed on the standard output.
    };

    static Log  *instance();

    /// @brief The generic method that writes all the logs.
    /// Emits a signal that write the log in the log thread.
    /// This method is called by the other public methods.
    /// @param level : The level of the current log.
    /// @param message : The message of the log.
    /// @param properties : A map of properties that can give additionals informations on the log.
    /// @param plugin : The name of the plugin from witch the log is written, or empty for the server.
    /// @param object : The name of the class from witch the log is written.
    /// @param method : The name of the method from witch the log is written.
    void        write(LightBird::ILogs::Level level
                    , const QString &message
                    , const QString &plugin
                    , const Properties &properties
                    , const QString &object
                    , const QString &method);
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

    /// @brief Set the state of the log, which depends on the state of the server.
    void    setState(State state);
    /// @brief Displays all the logs of the buffer in the standard output.
    void    print();
    /// @brief The main method of the thread. The logs are wrote in the event loop of this method.
    void    run();

signals:
    /// @brief This signal is emitted to write a log.
    void    writeLog(LightBird::ILogs::Level level, const QDateTime &date, const QString &message, const Properties &properties, const QString &thread, const QString &plugin, const QString &object, const QString &method);

private slots:
    /// @brief Write a log to the plugins that implements LightBird::ILog.
    void    _write(LightBird::ILogs::Level level, const QDateTime &date, const QString &message, const Properties &properties, const QString &thread, const QString &plugin, const QString &object, const QString &method);

private:
    Log();
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

    /// @brief Loads the configuration of the Logs.
    void    _initializeConfiguration();
    /// @brief Initialize the WRITE mode.
    void    _initializePlugin();
    /// @brief Convert a QMap to a QString. The Entries are separate by ", "
    /// @return The result of the conversion.
    QString _mapToString(const QMap<QString, QString> &properties) const;
    /// @brief Print the log in parameter on the standard output.
    /// @see write
    void    _print(LightBird::ILogs::Level level, const QDateTime &date, const QString &message, const Properties &properties, const QString &thread, const QString &plugin, const QString &object, const QString &method) const;

    LightBird::ILogs::Level level;      ///< The minimum level required to write a log.
    static Log              *_instance; ///< The instance of the Singleton.
    mutable QMutex          mutex;      ///< Ensures that only one log is written at the same time.
    mutable QMutex          printMutex; ///< Ensures that only one log is printed on the standard output at the same time.
    QList<LogInformations>  buffer;     ///< A buffer that stores the logs that haven't been saved yet.
    State                   state;      ///< The current mode of the log.
    bool                    display;    ///< If the logs have to be displayed in the standard output.
    QWaitCondition          waitRun;    ///< This condition is awakened when the thread is running for the first time.
    QMutex                  waitMutex;  ///< Used by QWaitCondition.
    bool                    awake;      ///< If the wait condition has been called.
    QObject                 *parent;    ///< The parent of the Log.
    QMap<LightBird::ILogs::Level, QString>  levels; ///< Combines the levels and their names.
};

/// Undefines the defines of LightBird::ILogs.
# undef LOG_FATAL
# undef LOG_ERROR
# undef LOG_WARNING
# undef LOG_INFO
# undef LOG_DEBUG
# undef LOG_TRACE
/// Optimizes the use of the log by checking its level before issuing the log.
/// This way the parameters of the log are only evaluated when the level is correct,
/// which avoids unnecessary computation in higher log levels.
# define LOG_FATAL   Log::fatal
# define LOG_ERROR   if (Log::instance()->isError())   Log::error
# define LOG_WARNING if (Log::instance()->isWarning()) Log::warning
# define LOG_INFO    if (Log::instance()->isInfo())    Log::info
# define LOG_DEBUG   if (Log::instance()->isDebug())   Log::debug
# define LOG_TRACE   if (Log::instance()->isTrace())   Log::trace

#endif // LOG_H
