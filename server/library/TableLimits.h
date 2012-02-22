#ifndef LIGHTBIRD_TABLELIMITS_H
# define LIGHTBIRD_TABLELIMITS_H

# include <QString>

# include "Table.h"

namespace LightBird
{
    /// @brief Handles the transactions with the database relating to a limit.
    /// Each modifications done in this object is immediatly saved in the database.
    class LIB TableLimits : public LightBird::Table
    {
    public:
        TableLimits(const QString &id = "");
        ~TableLimits();
        TableLimits(const TableLimits &table);
        TableLimits &operator=(const TableLimits &table);

        /// @brief Creates a new limit.
        /// @param name : The name of the limit.
        /// @param value : The value of the limit.
        /// @param id_accessor : The id of the accessor for which the limit is created.
        /// @param id_object : The id of the object for which the limit is created.
        /// @return True if the limit has been created.
        bool    add(const QString &name, const QString &value, const QString &id_accessor = "", const QString &id_object = "");

        // Fields
        /// @brief Returns the id of the accessor on which the limit is applied.
        QString getIdAccessor() const;
        /// @brief Changes the accessor on which the limit is applied.
        bool    setIdAccessor(const QString &id_accessor = "");
        /// @brief Returns the id of the object on which the limit is applied.
        QString getIdObject() const;
        /// @brief Changes the object on which the limit is applied.
        bool    setIdObject(const QString &id_object = "");
        /// @brief Returns the name of the limit.
        QString getName() const;
        /// @brief Modifies the name of the limit.
        bool    setName(const QString &name);
        /// @brief Returns the value of the limit.
        QString getValue() const;
        /// @brief Modifies the value of the limit.
        bool    setValue(const QString &value);
    };
}

#endif // LIGHTBIRD_TABLELIMITS_H
