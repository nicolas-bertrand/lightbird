#ifndef ITABLEPERMISSIONS_H
# define ITABLEPERMISSIONS_H

# include <QString>

# include "ITable.h"

namespace LightBird
{
    /// @brief Handle the transactions with the database relating to a permission.
    /// Each modifications done in this object is immediatly saved in the database.
    /// This can be seen as an implementation of the Active Record design pattern.
    class ITablePermissions : virtual public LightBird::ITable
    {
    public:
        virtual ~ITablePermissions() {}

        /// @brief Returns the id of the permission that matches the parameters.
        virtual QString getId(const QString &id_accessor, const QString &id_object, const QString &right) = 0;
        /// @brief Set the id of the permission that matches the parameters.
        virtual bool    setId(const QString &id_accessor, const QString &id_object, const QString &right) = 0;
        /// @brief Creates a new permission, using the given parameter.
        /// @param id_accessor : The id of the accessor for which the permission is created.
        /// @param id_object : The id of the object for which the permission is created.
        /// @param right : The right of the permission.
        /// @param granted : If the permission is granted (true) or denied (false).
        /// @return If the permission has been correctly created.
        virtual bool    add(const QString &id_accessor, const QString &id_object, const QString &right, bool granted = true) = 0;

        // Fields
        /// @brief Returns the id of the accessor on which the permission is applied.
        virtual QString getIdAccessor() = 0;
        /// @brief Change the accessor on which the permission is applied.
        virtual bool    setIdAccessor(const QString &id_accessor) = 0;
        /// @brief Returns the id of the object on which the permission is applied.
        virtual QString getIdObject() = 0;
        /// @brief Change the object on which the permission is applied.
        virtual bool    setIdObject(const QString &id_object) = 0;
        /// @brief Returns the right of the permission.
        virtual QString getRight() = 0;
        /// @brief Modify the right of the permission.
        virtual bool    setRight(const QString &right) = 0;
        /// @brief Returns true if the permission is granted.
        virtual bool    isGranted() = 0;
        /// @brief Grants or denies the permission.
        virtual bool    isGranted(bool granted) = 0;

        // Other
        /// @brief Returns true if the accessor possesses the right on the object.
        /// Read the database documentation to learn how the permissions works.
        virtual bool    isAllowed(const QString &id_accessor, const QString &id_object, const QString &right) = 0;
        /// @brief Returns the list of the rights that an accessor had on an object.
        /// This method is not up to date yet.
        /// @deprecated
        virtual QStringList getRights(const QString &id_accessor, const QString &id_object) = 0;
    };
}

Q_DECLARE_INTERFACE(LightBird::ITablePermissions, "cc.LightBird.ITablePermissions");

#endif // ITABLEPERMISSIONS_H
