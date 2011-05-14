#ifndef ITABLEEVENTS_H
# define ITABLEEVENTS_H

# include <QString>
# include <QMap>
# include <QDateTime>

# include "ITable.h"

namespace LightBird
{
    /// @brief Handle the transactions with the database relating to an event.
    /// Each modifications done in this object is immediatly saved in the database.
    class ITableEvents : virtual public LightBird::ITable
    {
    public:
        virtual ~ITableEvents() {}

        /// @brief Return the id of all the events with the specified name and date.
        /// @param name : The name of the events to return.
        /// @param start : The date on which the search begins.
        /// @param end : The date on which the search ends.
        /// @return The list of the id of the events that match the filters.
        virtual QStringList getEvents(const QString &name, const QDateTime &begin = QDateTime(), const QDateTime &end = QDateTime::currentDateTime()) = 0;
        /// @brief Created a new event.
        /// @param name : The name of the event.
        /// @param id_accessor : The id of the accessor for which the event is defined.
        /// If empty, the event will be linked to any accessor.
        /// @param id_object : The id of the object for which the event is defined.
        /// If empty, the event will be linked to any object.
        /// @param Informations : A map that contains informations about the event.
        /// @return True if the event has been created.
        virtual bool        add(const QString &name, const QMap<QString, QVariant> &informations,
                                const QString &id_accessor = "", const QString &id_object = "") = 0;
        /// @see add
        virtual bool        add(const QString &name, const QString &id_accessor = "", const QString &id_object = "") = 0;

        // Fields
        /// @brief Returns the name of the event.
        virtual QString     getName() = 0;
        /// @brief Modifies the name of the event.
        virtual bool        setName(const QString &name) = 0;
        /// @brief Returns the id of the accessor of the event.
        virtual QString     getIdAccessor() = 0;
        /// @brief Modifies the id of the accessor of the event.
        virtual bool        setIdAccessor(const QString &id_accessor = "") = 0;
        /// @brief Returns the id of the object of the event.
        virtual QString     getIdObject() = 0;
        /// @brief Modifies the id of the object of the event.
        virtual bool        setIdObject(const QString &id_object = "") = 0;

        // Informations
        /// @brief Returns the value of an information of the event.
        /// @param name : The name of the information to return.
        virtual QVariant    getInformation(const QString &name) = 0;
        /// @brief Returns all the informations of the event.
        virtual QMap<QString, QVariant> getInformations() = 0;
        /// @brief Modify the value of an information of the event, or create
        /// it if it doesn't exists.
        /// @param name : The name of the information to create or modify.
        /// @brief value : The new value of the information.
        virtual bool        setInformation(const QString &name, const QVariant &value) = 0;
        /// @brief Modifies or creates multiple informations for the event.
        /// @param informations : The informations to modify or create.
        /// The keys of the map are the keys of the informations, and the
        /// values of the map are the values of the informations.
        virtual bool        setInformations(const QMap<QString, QVariant> &informations) = 0;
        /// @brief Removes an information of the event.
        /// @param name : The name of the information to remove.
        virtual bool        removeInformation(const QString &name) = 0;
        /// @brief Removes multiple informations of the event.
        /// @param informations : This list contains the name of each
        /// informations to remove.
        virtual bool        removeInformations(const QStringList &informations) = 0;
    };
}

Q_DECLARE_INTERFACE(LightBird::ITableEvents, "cc.lightbird.ITableEvents");

#endif // ITABLEEVENTS_H
