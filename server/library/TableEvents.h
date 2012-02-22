#ifndef LIGHTBIRD_TABLEEVENTS_H
# define LIGHTBIRD_TABLEEVENTS_H

# include <QDateTime>
# include <QMap>
# include <QString>
# include <QStringList>
# include <QVariant>

# include "Table.h"

namespace LightBird
{
    /// @brief Handles the transactions with the database relating to an event.
    /// Each modifications done in this object is immediatly saved in the database.
    class LIB TableEvents : public LightBird::Table
    {
    public:
        TableEvents(const QString &id = "");
        ~TableEvents();
        TableEvents(const TableEvents &table);
        TableEvents &operator=(const TableEvents &table);

        /// @brief Return the id of all the events with the specified name and date.
        /// @param name : The name of the events to return.
        /// @param start : The date on which the search begins.
        /// @param end : The date on which the search ends.
        /// @return The list of the id of the events that match the filters.
        QStringList getEvents(const QString &name, const QDateTime &begin = QDateTime(), const QDateTime &end = QDateTime::currentDateTime()) const;
        /// @brief Created a new event.
        /// @param name : The name of the event.
        /// @param id_accessor : The id of the accessor for which the event is defined.
        /// If empty, the event will be linked to any accessor.
        /// @param id_object : The id of the object for which the event is defined.
        /// If empty, the event will be linked to any object.
        /// @param Informations : A map that contains informations about the event.
        /// @return True if the event has been created.
        bool        add(const QString &name, const QVariantMap &informations,
                        const QString &id_accessor = "", const QString &id_object = "");
        /// @see add
        bool        add(const QString &name, const QString &id_accessor = "", const QString &id_object = "");

        // Fields
        /// @brief Returns the name of the event.
        QString     getName() const;
        /// @brief Modifies the name of the event.
        bool        setName(const QString &name);
        /// @brief Returns the id of the accessor of the event.
        QString     getIdAccessor() const;
        /// @brief Modifies the id of the accessor of the event.
        bool        setIdAccessor(const QString &id_accessor = "");
        /// @brief Returns the id of the object of the event.
        QString     getIdObject() const;
        /// @brief Modifies the id of the object of the event.
        bool        setIdObject(const QString &id_object = "");

        // Informations
        /// @brief Returns the value of an information of the event.
        /// @param name : The name of the information to return.
        QVariant    getInformation(const QString &name) const;
        /// @brief Returns all the informations of the event.
        QVariantMap getInformations() const;
        /// @brief Modify the value of an information of the event, or create
        /// it if it doesn't exists.
        /// @param name : The name of the information to create or modify.
        /// @brief value : The new value of the information.
        bool        setInformation(const QString &name, const QVariant &value);
        /// @brief Modifies or creates multiple informations for the event.
        /// @param informations : The informations to modify or create.
        /// The keys of the map are the keys of the informations, and the
        /// values of the map are the values of the informations.
        bool        setInformations(const QVariantMap &informations);
        /// @brief Removes an information of the event.
        /// @param name : The name of the information to remove.
        bool        removeInformation(const QString &name);
        /// @brief Removes multiple informations of the event.
        /// @param informations : This list contains the name of each
        /// informations to remove. If empty all the informations are removed.
        bool        removeInformations(const QStringList &informations = QStringList());
    };
}

#endif // LIGHTBIRD_TABLEEVENTS_H
