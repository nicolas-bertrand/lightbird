#ifndef EVENTS_H
# define EVENTS_H

# include <QList>
# include <QMutex>
# include <QObject>
# include <QString>
# include <QVariant>

# include "ApiEvents.h"

/// @brief Handles the events sent by the server and the plugins, and disctibutes
/// them to the plugins that have subscribed.
class Events : public QObject
{
    Q_OBJECT

public:
    Events(QObject *parent = NULL);
    ~Events();

    /// @brief This method is called each time a plugin is loaded, to allow it to
    /// receive events.
    void    add(ApiEvents *apiEvents);
    /// @brief Removes a plugin from the events system.
    void    remove(ApiEvents *apiEvents);
    /// @brief Send a new event with an optional property.
    void    send(const QString &event, const QVariant &property = QVariant());
    /// @brief Returns the instance of this class created by the Server.
    static Events *instance();

private:
    Events(const Events &);
    Events &operator=(const Events &);

    QList<ApiEvents *> events; ///< The list of the plugins that can send and receive events.
    QMutex             mutex;  ///< Makes this class thread safe.
};

#endif // EVENTS_H
