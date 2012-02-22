#ifndef LIGHTBIRD_TABLEACCESSORS_H
# define LIGHTBIRD_TABLEACCESSORS_H

# include <QString>
# include <QStringList>

# include "Table.h"

namespace LightBird
{
    /// @brief This class represents an accessor, which can be an account or a group.
    class LIB TableAccessors : public LightBird::Table
    {
    public:
        ~TableAccessors();

        // Fields
        /// @brief Returns the name of the accessor.
        QString     getName() const;
        /// @brief Modifies the name of the accessor.
        bool        setName(const QString &name);

        // Other
        /// @brief Returns true if the accessor has the right on the object.
        /// @see LightBird::TablePermissions::isAllowed
        bool        isAllowed(const QString &id_object, const QString &right) const;
        /// @brief Allows to get the list of the rights that the accessor has
        /// on the object.
        /// @see LightBird::TablePermissions::getRights
        bool        getRights(const QString &id_object, QStringList &allowed, QStringList &denied) const;
        /// @brief Returns the limits of the accessor.
        QStringList getLimits() const;

    protected:
        TableAccessors();
        TableAccessors(const TableAccessors &table);
        TableAccessors &operator=(const TableAccessors &table);
    };
}

#endif // LIGHTBIRD_TABLEACCESSORS_H
