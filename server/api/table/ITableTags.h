#ifndef ITABLETAGS_H
# define ITABLETAGS_H

# include <QString>

# include "ITable.h"

namespace LightBird
{
    /// @brief Handle the transactions with the database relating to a tag.
    /// Each modifications done in this object is immediatly saved in the database.
    class ITableTags : virtual public LightBird::ITable
    {
    public:
        virtual ~ITableTags() {}

        /// @brief Creates a new tag.
        /// @param id_object : The id of the object for which the tag is created.
        /// @param name : The name of the tag.
        /// @return True if the tag has been created.
        virtual bool    add(const QString &id_object, const QString &name) = 0;

        // Fields
        /// @brief Returns the id of the object on which the tag is applied.
        virtual QString getIdObject() const = 0;
        /// @brief Changes the object on which the tag is applied.
        virtual bool    setIdObject(const QString &id_object) = 0;
        /// @brief Returns the name of the tag.
        virtual QString getName() const = 0;
        /// @brief Modifies the name of the tag.
        virtual bool    setName(const QString &name) = 0;
    };
}

Q_DECLARE_INTERFACE(LightBird::ITableTags, "cc.lightbird.ITableTags");

#endif // ITABLETAGS_H
