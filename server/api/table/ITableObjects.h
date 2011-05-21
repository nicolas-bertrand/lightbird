#ifndef ITABLEOBJECTS_H
# define ITABLEOBJECTS_H

# include <QString>
# include <QStringList>

# include "ITable.h"

namespace LightBird
{
    /// @brief This class represents an object, which can be
    /// a file, a directory, or a collection.
    class ITableObjects : virtual public LightBird::ITable
    {
    public:
        virtual ~ITableObjects() {}

        // Fields
        /// @brief Returns the id of the account that possesses the object.
        virtual QString     getIdAccount() = 0;
        /// @brief Modifies the id of the account that possesses the object.
        virtual bool        setIdAccount(const QString &id_account = "") = 0;
        /// @brief Returns the name of the object.
        virtual QString     getName() = 0;
        /// @brief Modifies the name of the object.
        virtual bool        setName(const QString &name) = 0;

        // Other
        /// @brief Returns true if the accessor has the right on the object.
        /// @see LightBird::ITablePermissions::isAllowed
        virtual bool        isAllowed(const QString &id_accessor, const QString &right) = 0;
        /// @brief Allows to get the list of the rights that the accessor has
        /// on the object.
        /// @see LightBird::ITablePermissions::getRights
        virtual bool        getRights(const QString &id_accessor, QStringList &allowed, QStringList &denied) = 0;
        /// @brief Returns the id of the tags of the object.
        virtual QStringList getTags() = 0;
        /// @brief Returns the limits of the object.
        virtual QStringList getLimits() = 0;
    };
}

Q_DECLARE_INTERFACE(LightBird::ITableObjects, "cc.lightbird.ITableObjects");

#endif // ITABLEOBJECTS_H
