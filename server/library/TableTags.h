#ifndef LIGHTBIRD_TABLETAGS_H
# define LIGHTBIRD_TABLETAGS_H

# include <QString>

# include "Table.h"

namespace LightBird
{
    /// @brief Handles the transactions with the database relating to a tag.
    /// Each modifications done in this object is immediatly saved in the database.
    class LIB TableTags : public LightBird::Table
    {
    public:
        TableTags(const QString &id = "");
        ~TableTags();
        TableTags(const TableTags &table);
        TableTags &operator=(const TableTags &table);

        /// @brief Creates a new tag.
        /// @param id_object : The id of the object for which the tag is created.
        /// @param name : The name of the tag.
        /// @return True if the tag has been created.
        bool    add(const QString &id_object, const QString &name);

        // Fields
        /// @brief Returns the id of the object on which the tag is applied.
        QString getIdObject() const;
        /// @brief Changes the object on which the tag is applied.
        bool    setIdObject(const QString &id_object);
        /// @brief Returns the name of the tag.
        QString getName() const;
        /// @brief Modifies the name of the tag.
        bool    setName(const QString &name);
    };
}

#endif // LIGHTBIRD_TABLETAGS_H
