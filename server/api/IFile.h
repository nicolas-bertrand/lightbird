#ifndef IEVENT_H
# define IEVENT_H

# include <QString>
# include <QMap>
# include <QDateTime>

# include "ITable.h"

namespace StreamIt
{
    /**
     * @brief Handle the transactions with the database relating to an event.
     * The objective of this class is to abstract and facilitate the usage of the
     * tables events and events_informations.
     * Each modifications done in this object is immediatly saved in the database.
     * This can be seen as an implementation of the Active Record design pattern.
     */
    class IEvent : public Streamit::ITable
    {
    public:
        virtual ~IEvent() {}
        /**
         * @brief Load an existing event from the database. If an event is already
         * loaded in the current instance of IEvent, it will be replaced. An event
         * must be loaded before calling the methods getName, getDate, getIdAccessor,
         * getIdObject, getInformation and getInformations, and there setters (set*).
         * @param id : The id of the event.
         * @return True if the event has been loaded.
         */
        virtual bool        load(const QString &id) = 0;
        /**
         * @brief Return the id of all the events with the specified name. The
         * names are stored in the table events_actions.
         * @param name : The name of the events to return.
         * @param date : The date from which the search is started.
         * @return The list of the id of the events.
         */
        virtual QStringList  get(const QString &name, const QDateTime &date = QDateTime()) = 0;
        /**
         * @brief Create a new event and load it in the current instance of IEvent.
         * If an event is already loaded in the current instance of IEvent, it will
         * be replaced. The date of the event is set to the current date.
         * @param name : The name of the event (stored in the table events_actions).
         * @param idAccessor : The id of accessor for which the event is defined.
         * If empty, the event will be linked to any accessor.
         * @param idObject : The id of accessor for which the event is defined.
         * If empty, the event will be linked to any object.
         * @param Informations : A map that contains informations about the event.
         * @return True if the event has been created.
         */
        virtual bool        create(const QString &name, const QString &idAccessor, const QString &idObject,
                                   const QMap<QString, QString> &informations) = 0;
        /**
         * @brief Delete the current event from the database.
         * @return True if success.
         */
        virtual bool        remove() = 0;
        /**
         * @return The id of the event.
         */
        virtual QString     getId() = 0;
        /**
         * @return The name of the event, as stored in the table events_actions.
         */
        virtual QString     getName() = 0;
        /**
         * @return The date of the event.
         */
        virtual QDateTime   getDate() = 0;
        /**
         * @return The id of the accessor of the event.
         */
        virtual QString     getIdAccessor() = 0;
        /**
         * @return The id of the object of the event.
         */
        virtual QString     getIdObject() = 0;
        /**
         * @brief Return the value of the informations gived in parameter.
         * @param key : The name of the value to return.
         * @return The value of the information.
         */
        virtual QString     getInformation(const QString &key) = 0;
        /**
         * @return All the informations of the event in a key/value map.
         */
        virtual QMap<QString, QString>  getInformations() = 0;
        /**
         * @brief Modify the name of the event.
         * @param name : The new name.
         * @return True if success.
         */
        virtual bool        setName(const QString &name) = 0;
        /**
         * @brief Modify the date of the event.
         * @param date : The new date.
         * @return True if success.
         */
        virtual bool        setDate(const QDateTime &date) = 0;
        /**
         * @brief Modify the idAccessor of the event.
         * @param idAccessor : The new idAccessor.
         * @return True if success.
         */
        virtual bool        setIdAccessor(const QString &idAccessor) = 0;
        /**
         * @brief Modify the idObject of the event.
         * @param idObject : The new idObject.
         * @return True if success.
         */
        virtual bool        setIdObject(const QString &idObject) = 0;
        /**
         * @brief Modify or create the information of the event, with the given value.
         * @param key : The name of the information.
         * @param value : The value of the information.
         * @return True if success.
         */
        virtual bool        setInformation(const QString &key, const QString &value) = 0;
        /**
         * @brief Modify or create all the informations contained if the map.
         * @param informations : The informations to modify or create.
         * @return True if success.
         */
        virtual bool        setInformations(const QMap<QString, QString> &informations) = 0;
        /**
         * @brief Delete an information.
         * @param key : The key of the information to delete.
         * @return True if success.
         */
        virtual bool        removeInformation(const QString &key) = 0;
    };
}

#endif // IEVENT_H
