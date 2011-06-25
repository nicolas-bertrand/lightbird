#ifndef	APIEVENTS_H
# define APIEVENTS_H

# include <QMutex>
# include <QThread>
# include <QWaitCondition>

# include "IEvents.h"

/// @brief This is the server implementation of the IEvents interface.
/// It also calls the IEvent interface for the plugins that implements it.
class ApiEvents : public QThread,
                  public LightBird::IEvents
{
    Q_OBJECT
    Q_INTERFACES(LightBird::IEvents)

public:
    /// @param event : True if the plugin implements the interface IEvent.
    /// In this case a thread is created to deliver the events using this interface.
    ApiEvents(const QString &id, bool event = false);
    ~ApiEvents();

    /// @brief If the plugin implements IEvent a thread is created to deliver
    /// the events to the plugin via this interface.
    void    run();
    /// @brief Add a new event to the queue if the plugin has subscribed to it.
    void    post(const QString &event, const QVariant &property = QVariant());

    // LightBird::IEvents
    void    subscribe(const QStringList &events);
    void    send(const QString &event, const QVariant &property = QVariant());
    QList<QPair<QString, QVariant> > receive();
    bool    isAvailable();

private:
    ApiEvents();
    ApiEvents(const ApiEvents &);
    ApiEvents* operator=(const ApiEvents &);

    QString         id;         ///< The id of the plugin for which the object has been created.
    QMutex          mutex;      ///< Makes this class thread safe.
    QWaitCondition  wait;       ///< Allows to wait that the thread is started before return from the constructor.
    bool            awake;      ///< If the wait condition has been called.
    QStringList     subscribed; ///< List the events to which the plugin has subscribed.
    QList<QPair<QString, QVariant> > events; ///< The list of the pending events.

private slots:
    /// @brief Calls IEvent for the plugins that implements it. This slot is
    /// called in the events thread each time an event occur if the plugin
    /// implements IEvent.
    void            _newEvent();

signals:
    /// @brief Signal emitted by post() each time an event for which the
    /// plugin has subscribed occur.
    void            newEvent();
};

#endif // APIEVENTS_H
