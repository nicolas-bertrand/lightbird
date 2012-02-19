#ifndef ILOGS_H
# define ILOGS_H

# include <QString>
# include <QMap>

namespace LightBird
{
    /// @brief The plugins can log informations from this interface. Notice that
    /// the log may not be immediatly written, since it use the Qt slot/signal facilities.
    /// Therefore, logs are written asynchronously.
    /// This interface is threaeSafe.
    class ILogs
    {
    public:
        virtual ~ILogs() {}

        /// @brief List of available log levels.
        enum Level
        {
            FATAL = 5,   ///< Severe errors that cause premature termination.
            ERROR = 4,   ///< Other runtime errors or unexpected conditions.
            WARNING = 3, ///< Use of deprecated APIs, poor use of API, 'almost' errors, other runtime situations that are undesirable or unexpected, but not necessarily "wrong".
            INFO = 2,    ///< Interesting runtime events (startup/shutdown). Should be keep to a minimum.
            DEBUG = 1,   ///< Detailed information on the flow through the system.
            TRACE = 0    ///< More detailed information.
         };

        /// @brief Writes a log entry.
        /// @param level : The level of the entry.
        /// @param message : The message of the log.
        /// @param properties : A map of properties that can give additional informations on the log entry.
        /// @param object : The name of the class from which the log is written.
        /// @param method : The name of the method from which the log is written.
        virtual void        write(LightBird::ILogs::Level level, const QString &message, const QMap<QString, QString> &properties, const QString &object, const QString &method) const = 0;
        /// @brief Writes a fatal log level
        /// @see write
        virtual void        fatal(const QString &message, const QString &object = "", const QString &method = "") const = 0;
        /// @brief Writes a fatal log level
        /// @see write
        virtual void        fatal(const QString &message, const QMap<QString, QString> &properties, const QString &object = "", const QString &method = "") const = 0;
        /// @brief Writes an error log level
        /// @see write
        virtual void        error(const QString &message, const QString &object = "", const QString &method = "") const = 0;
        /// @brief Writes an error log level
        /// @see write
        virtual void        error(const QString &message, const QMap<QString, QString> &properties, const QString &object = "", const QString &method = "") const = 0;
        /// @brief Writes a warning log level
        /// @see write
        virtual void        warning(const QString &message, const QString &object = "", const QString &method = "") const = 0;
        /// @brief Writes a warning log level
        /// @see write
        virtual void        warning(const QString &message, const QMap<QString, QString> &properties, const QString &object = "", const QString &method = "") const = 0;
        /// @brief Writes an info log level
        /// @see write
        virtual void        info(const QString &message, const QString &object = "", const QString &method = "") const = 0;
        /// @brief Writes an info log level
        /// @see write
        virtual void        info(const QString &message, const QMap<QString, QString> &properties, const QString &object = "", const QString &method = "") const = 0;
        /// @brief Writes a debug log level
        /// @see write
        virtual void        debug(const QString &message, const QString &object = "", const QString &method = "") const = 0;
        /// @brief Writes a debug log level
        /// @see write
        virtual void        debug(const QString &message, const QMap<QString, QString> &properties, const QString &object = "", const QString &method = "") const = 0;
        /// @brief Writes a trace log level
        /// @see write
        virtual void        trace(const QString &message, const QString &object = "", const QString &method = "") const = 0;
        /// @brief Writes a trace log level
        /// @see write
        virtual void        trace(const QString &message, const QMap<QString, QString> &properties, const QString &object = "", const QString &method = "") const = 0;
        /// @brief The current log level. The logs below this level are not saved.
        /// For example, WARNING is below ERROR, and INFO is below WARNING...
        /// @return The current log level.
        virtual LightBird::ILogs::Level getLevel() const = 0;
        /// @brief Modifies the log level.
        /// @param level : The new log level.
        virtual void        setLevel(LightBird::ILogs::Level level) = 0;
        /// @brief Returns true if the logs are displayed on the standard output.
        virtual bool        isDisplay() const = 0;
        /// @brief Display or hide the logs on the standard output.
        virtual void        isDisplay(bool display) = 0;
        /// @return True if the error logs are saved.
        virtual bool        isError() const = 0;
        /// @return True if the warning logs are saved.
        virtual bool        isWarning() const = 0;
        /// @return True if the info logs are saved.
        virtual bool        isInfo() const = 0;
        /// @return True if the debug logs are saved.
        virtual bool        isDebug() const = 0;
        /// @return True if the trace logs are saved.
        virtual bool        isTrace() const = 0;
    };
}

Q_DECLARE_INTERFACE(LightBird::ILogs, "cc.lightbird.ILogs");

#endif // ILOGS_H
