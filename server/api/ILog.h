#ifndef ILOG_H
# define ILOG_H

# include <QString>
# include <QMap>
# include <QDateTime>
# include "ILogs.h"

namespace Streamit
{
    /// @brief This interface allows one to handle the logs.
    class ILog
    {
    public:
        virtual ~ILog() {}

        /// @brief Calls when a log is submit.
        /// @param level : The level of the current log.
        /// @param message : The message of the log.
        /// @param properties : A map of properties that can give additionals informations on the log.
        /// @param thread : The address of the thread that emits the log, in base 64.
        /// @param plugin : The id of the plugin from which the log is written (empty for the server).
        /// @param object : The name of the class from which the log is written.
        /// @param method : The name of the method from which the log is written.
        virtual void    log(Streamit::ILogs::level level, const QDateTime &date, const QString &message,
                            const QMap<QString, QString> &properties, const QString &thread,
                            const QString &plugin, const QString &object, const QString &method) = 0;
    };
}

Q_DECLARE_INTERFACE(Streamit::ILog, "cc.lightbird.ILog");

#endif // ILOG_H
