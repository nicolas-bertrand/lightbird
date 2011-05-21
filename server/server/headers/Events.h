#ifndef EVENTS_H
# define EVENTS_H

# include <QList>
# include <QMutex>
# include <QObject>
# include <QString>
# include <QVariant>

# include "ApiEvents.h"

/// @brief This singleton handles the events sent by the server and the plugins,
/// and discributes them to the plugins that have subscribed.
class Events : public QObject
{
    Q_OBJECT

public:
    static Events *instance(QObject *parent = NULL);

    /// @brief This method is called each time a plugin is loaded to allow it to
    /// receive events.
    void    add(ApiEvents *apiEvents);
    /// @brief Removes a plugin of the events system.
    void    remove(ApiEvents *apiEvents);
    /// @brief Send a new event with an optional property.
    void    send(const QString &event, const QVariant &property = QVariant());

private:
    Events(QObject *parent = NULL);
    ~Events();
    Events(const Events &);
    Events *operator=(const Events &);

    QMutex              mutex;      ///< Ensure that the class is thread safe.
    QList<ApiEvents *>  events;     ///< The list of the plugins that can send and receive events.
    static Events       *_instance; ///< The instance of the singleton.
};

#endif // EVENTS_H
