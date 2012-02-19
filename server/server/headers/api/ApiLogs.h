#ifndef	APILOGS_H
# define APILOGS_H

# include <QObject>

# include "ILogs.h"

/// @brief This is the server implementation of the ILogs interface
/// that allows plugins to write logs.
class ApiLogs : public QObject,
                public LightBird::ILogs
{
    Q_OBJECT
    Q_INTERFACES(LightBird::ILogs)

public:
    /// @param plugin : True if the instance is dedicated to a plugin. If false,
    /// the "plugin" value of the properties is used to determine the plugin
    /// that emits the log.
    ApiLogs(const QString &id, bool plugin = true);
    ~ApiLogs();

    void    write(LightBird::ILogs::Level level, const QString &message, const QMap<QString, QString> &properties, const QString &className, const QString &method) const;
    void    fatal(const QString &message, const QString &className = "", const QString &method = "") const;
    void    fatal(const QString &message, const QMap<QString, QString> &properties, const QString &className = "", const QString &method = "") const;
    void    error(const QString &message, const QString &className = "", const QString &method = "") const;
    void    error(const QString &message, const QMap<QString, QString> &properties, const QString &className = "", const QString &method = "") const;
    void    warning(const QString &message, const QString &className = "", const QString &method = "") const;
    void    warning(const QString &message, const QMap<QString, QString> &properties, const QString &className = "", const QString &method = "") const;
    void    info(const QString &message, const QString &className = "", const QString &method = "") const;
    void    info(const QString &message, const QMap<QString, QString> &properties, const QString &className = "", const QString &method = "") const;
    void    debug(const QString &message, const QString &className = "", const QString &method = "") const;
    void    debug(const QString &message, const QMap<QString, QString> &properties, const QString &className = "", const QString &method = "") const;
    void    trace(const QString &message, const QString &className = "", const QString &method = "") const;
    void    trace(const QString &message, const QMap<QString, QString> &properties, const QString &className = "", const QString &method = "") const;
    LightBird::ILogs::Level getLevel() const;
    void    setLevel(LightBird::ILogs::Level level);
    bool    isDisplay() const;
    void    isDisplay(bool display);
    bool    isError() const;
    bool    isWarning() const;
    bool    isInfo() const;
    bool    isDebug() const;
    bool    isTrace() const;

private:
    ApiLogs();
    ApiLogs(const ApiLogs &);
    ApiLogs &operator=(const ApiLogs &);

    QString id;     ///< The id of the plugin for which the object has been created.
    bool    plugin; /// True if the instance is dedicated to a plugin.
};

#endif // APILOGS_H
