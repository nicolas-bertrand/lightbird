#ifndef ITABLETAGS_H
# define ITABLETAGS_H

# include <QString>

# include "ITable.h"

namespace Streamit
{
    /// @brief Handle the transactions with the database relating to a tag.
    /// Each modifications done in this object is immediatly saved in the database.
    /// This can be seen as an implementation of the Active Record design pattern.
    class ITableTags : virtual public Streamit::ITable
    {
    public:
        virtual ~ITableTags() {}

        /// @brief Creates a new tag, using the given parameter.
        /// @param id_object : The id of the object for which the tag is created.
        /// @param name : The name of the tag.
        /// @return If the tag has been correctly created.
        virtual bool    add(const QString &id_object, const QString &name) = 0;

        // Fields
        /// @brief Returns the id of the object on which the tag is applied.
        virtual QString getIdObject() = 0;
        /// @brief Change the object on which the tag is applied.
        virtual bool    setIdObject(const QString &id_object) = 0;
        /// @brief Returns the name of the tag.
        virtual QString getName() = 0;
        /// @brief Modify the name of the tag.
        virtual bool    setName(const QString &name) = 0;
    };
}

Q_DECLARE_INTERFACE(Streamit::ITableTags, "cc.lightbird.ITableTags");

#endif // ITABLETAGS_H
