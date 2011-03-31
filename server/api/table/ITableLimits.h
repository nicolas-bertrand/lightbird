#ifndef ITABLELIMITS_H
# define ITABLELIMITS_H

# include <QString>

# include "ITable.h"

namespace Streamit
{
    /// @brief Handle the transactions with the database relating to a limit.
    /// Each modifications done in this object is immediatly saved in the database.
    /// This can be seen as an implementation of the Active Record design pattern.
    class ITableLimits : virtual public Streamit::ITable
    {
    public:
        virtual ~ITableLimits() {}

        /// @brief Creates a new limit, using the given parameter.
        /// @param name : The name of the limit.
        /// @param value : The value of the limit.
        /// @param id_accessor : The id of the accessor for which the limit is created.
        /// @return If the limit has been correctly created.
        virtual bool    add(const QString &name, const QString &value, const QString &id_accessor = "") = 0;

        // Fields
        /// @brief Returns the name of the limit.
        virtual QString getName() = 0;
        /// @brief Modify the name of the limit.
        virtual bool    setName(const QString &name) = 0;
        /// @brief Returns the value of the limit.
        virtual QString getValue() = 0;
        /// @brief Modify the value of the limit.
        virtual bool    setValue(const QString &value) = 0;
        /// @brief Returns the id of the accessor on which the limit is applied.
        virtual QString getIdAccessor() = 0;
        /// @brief Change the accessor on which the limit is applied.
        virtual bool    setIdAccessor(const QString &id_accessor = "") = 0;
    };
}

Q_DECLARE_INTERFACE(Streamit::ITableLimits, "cc.lightbird.ITableLimits");

#endif // ITABLELIMITS_H
