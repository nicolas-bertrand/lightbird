#ifndef IEVENTS_H
# define IEVENTS_H

# include <QList>
# include <QPair>
# include <QString>
# include <QStringList>
# include <QVariant>

namespace LightBird
{
    /// @brief Via this interface plugins can subscribe send and receive events.
    /// Events are sent by the server or other plugins.
    class IEvents
    {
    public:
        virtual ~IEvents() {}

        /// @brief Subscribe to an event.
        /// @param event : The event to subscribe.
        /// @see LightBird::IEvent
        virtual void        subscribe(const QString &event) = 0;
        /// @brief Subscribe to events.
        /// @param events : The list of events to subscribe.
        /// @see LightBird::IEvent
        virtual void        subscribe(const QStringList &events) = 0;
        /// @brief Unsubscribe from an event.
        /// @param event : The event to unsubscribe.
        virtual void        unsubscribe(const QString &event) = 0;
        /// @brief Unsubscribe from events.
        /// @param events : The list of events to unsubscribe.
        virtual void        unsubscribe(const QStringList &events) = 0;
        /// @brief Returns the list of the subscribed events.
        virtual QStringList getEvents() const = 0;
        /// @brief Sends an event that will be received by the plugins that
        /// have subscribed to it.
        /// @param event : The name of the event to send.
        /// @param property : An optional property that the event may require.
        virtual void        send(const QString &event, const QVariant &property = QVariant()) = 0;
        /// @brief Allows plugins to receive events without implementing IEvent.
        /// All the events are removed of the queue after a call to this method.
        /// @return The list of all the events in the queue.
        /// @see LightBird::IEvent
        virtual QList<QPair<QString, QVariant> > receive() = 0;
        /// @brief Returns true if events are ready to be received.
        virtual bool        isAvailable() const = 0;
    };
}

Q_DECLARE_INTERFACE(LightBird::IEvents, "cc.lightbird.IEvents");

#endif // IEVENTS_H
