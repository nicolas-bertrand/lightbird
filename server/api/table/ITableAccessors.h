#ifndef ITABLEACCESSORS_H
# define ITABLEACCESSORS_H

# include "ITable.h"

namespace LightBird
{
    /// @brief This class represents an accessor, which can be an account or a group.
    class ITableAccessors : virtual public LightBird::ITable
    {
    public:
        virtual ~ITableAccessors() {}

        // Fields
        /// @brief Returns the name of the accessor.
        virtual QString getName() = 0;
        /// @brief Modify the name of the accessor.
        virtual bool    setName(const QString &name) = 0;

        // Other
        /// @brief Returns true if the accessor has the right on the object.
        virtual bool        isAllowed(const QString &id_object, const QString &right) = 0;
        /// @brief Allows to get the list of the rights that the accessor has on the object.
        virtual bool        getRights(const QString &id_object, QStringList &allowed, QStringList &denied) = 0;
        /// @brief Returns the limits of the accessor.
        virtual QStringList getLimits() = 0;
    };
}

Q_DECLARE_INTERFACE(LightBird::ITableAccessors, "cc.lightbird.ITableAccessors");

#endif // ITABLEACCESSORS_H
