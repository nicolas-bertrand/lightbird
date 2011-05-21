#ifndef IEVENT_H
# define IEVENT_H

# include <QString>
# include <QVariant>

namespace LightBird
{
    /// @brief Allows plugins to receive events subscribed using IEvents API.
    /// The other way to receive events is the method receive of IEvents.
    class IEvent
    {
    public:
        virtual ~IEvent() {}

        /// @brief This method is called each time an events for which the
        /// plugin has subscribed occur. It is called in a thread dedicated
        /// to the plugin. While the plugin has not returned from this method,
        /// the other events received in the meantime are stored in a queue.
        /// It is guaranteed that only one call to event is made at a given time.
        /// @param event : The name of the event received.
        /// @param value : Some events can be associated with a property which
        /// is stored in this parameter.
        /// @see LightBird::IEvents
        virtual void    event(const QString &event, const QVariant &property = QVariant()) = 0;
    };
}

Q_DECLARE_INTERFACE(LightBird::IEvent, "cc.lightbird.IEvent");

#endif // IEVENT_H
