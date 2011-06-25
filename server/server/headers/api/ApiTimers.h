#ifndef	APITIMERS_H
# define APITIMERS_H

# include <QObject>
# include <QReadWriteLock>

# include "ITimers.h"

# include "Configuration.h"
# include "Timer.h"

/// @brief This class is the server implementation of the ITimer interface
/// that allows plugins to manage their timers.
class ApiTimers : public QObject,
                  public LightBird::ITimers
{
    Q_OBJECT
    Q_INTERFACES(LightBird::ITimers)

public:
    ApiTimers(const QString &id, QObject *parent = 0);
    ~ApiTimers();

    void                        setTimer(const QString &name, unsigned int timeout);
    unsigned int                getTimer(const QString &name);
    QMap<QString, unsigned int> getTimers();
    bool                        removeTimer(const QString &name);

private:
    ApiTimers();
    ApiTimers(const ApiTimers &);
    ApiTimers* operator=(const ApiTimers &);

    QString                 id;     ///< The id of the plugin for which the timers are managed.
    QMap<QString, Timer *>  timers; ///< The list of the timers of the plugin.
    QReadWriteLock          mutex;  ///< Makes this class thread safe.
};

#endif // APITIMERS_H
