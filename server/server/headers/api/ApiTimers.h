#ifndef	APITIMERS_H
# define APITIMERS_H

# include <QObject>
# include <QReadWriteLock>

# include "ITimers.h"

# include "Timer.h"
# include "Configuration.h"

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

    bool                        setTimer(const QString &name, unsigned int timeout);
    unsigned int                getTimer(const QString &name);
    QMap<QString, unsigned int> getTimers();
    bool                        removeTimer(const QString &name);

private:
    ApiTimers();
    ApiTimers(const ApiTimers &);
    ApiTimers* operator=(const ApiTimers &);

    QString                 id;         ///< The id of the plugin for which the object has been created.
    int                     maxTimers;  ///< The maximum number of timers autorized at the same time for a plugin.
    QMap<QString, Timer *>  timers;     ///< The list of the timers of the plugin.
    QReadWriteLock          lockTimers; ///< Ensure that the access to the timers is thread safe.
};

#endif // APITIMERS_H
