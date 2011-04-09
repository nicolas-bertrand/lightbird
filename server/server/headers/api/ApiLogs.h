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
    ApiLogs(const QString &id, QObject *parent = 0);
    ~ApiLogs();

    void    write(LightBird::ILogs::level level, const QString &message, const QMap<QString, QString> &properties, const QString &className, const QString &method);
    void    fatal(const QString &message, const QString &className = "", const QString &method = "");
    void    fatal(const QString &message, QMap<QString, QString> &properties, const QString &className = "", const QString &method = "");
    void    error(const QString &message, const QString &className = "", const QString &method = "");
    void    error(const QString &message, QMap<QString, QString> &properties, const QString &className = "", const QString &method = "");
    void    warning(const QString &message, const QString &className = "", const QString &method = "");
    void    warning(const QString &message, QMap<QString, QString> &properties, const QString &className = "", const QString &method = "");
    void    info(const QString &message, const QString &className = "", const QString &method = "");
    void    info(const QString &message, QMap<QString, QString> &properties, const QString &className = "", const QString &method = "");
    void    debug(const QString &message, const QString &className = "", const QString &method = "");
    void    debug(const QString &message, QMap<QString, QString> &properties, const QString &className = "", const QString &method = "");
    void    trace(const QString &message, const QString &className = "", const QString &method = "");
    void    trace(const QString &message, QMap<QString, QString> &properties, const QString &className = "", const QString &method = "");
    LightBird::ILogs::level getlevel();
    void    setLevel(LightBird::ILogs::level level);
    bool    isError();
    bool    isWarning();
    bool    isInfo();
    bool    isDebug();
    bool    isTrace();

private:
    ApiLogs();
    ApiLogs(const ApiLogs &);
    ApiLogs* operator=(const ApiLogs &);

    QString id; ///< The id of the plugin for which the object has been created.
};

#endif // APILOGS_H
