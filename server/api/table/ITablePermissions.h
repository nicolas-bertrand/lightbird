#ifndef ITABLEPERMISSIONS_H
# define ITABLEPERMISSIONS_H

# include <QString>

# include "ITable.h"

namespace LightBird
{
    /// @brief Handle the transactions with the database relating to a permission.
    /// Each modifications done in this object is immediatly saved in the database.
    class ITablePermissions : virtual public LightBird::ITable
    {
    public:
        virtual ~ITablePermissions() {}

        /// @brief Returns the id of the permission that matches the parameters.
        virtual QString getId(const QString &id_accessor, const QString &id_object, const QString &right) = 0;
        /// @brief Set the id of the permission that matches the parameters.
        virtual bool    setId(const QString &id_accessor, const QString &id_object, const QString &right) = 0;
        /// @brief Creates a new permission.
        /// @param id_accessor : The id of the accessor for which the permission is created.
        /// @param id_object : The id of the object for which the permission is created.
        /// @param right : The right of the permission.
        /// @param granted : If the permission is granted (true) or denied (false).
        /// @return True if the permission has been created.
        virtual bool    add(const QString &id_accessor, const QString &id_object, const QString &right, bool granted = true) = 0;

        // Fields
        /// @brief Returns the id of the accessor on which the permission is applied.
        virtual QString getIdAccessor() = 0;
        /// @brief Changes the accessor on which the permission is applied.
        virtual bool    setIdAccessor(const QString &id_accessor = "") = 0;
        /// @brief Returns the id of the object on which the permission is applied.
        virtual QString getIdObject() = 0;
        /// @brief Changes the object on which the permission is applied.
        virtual bool    setIdObject(const QString &id_object = "") = 0;
        /// @brief Returns the right of the permission.
        virtual QString getRight() = 0;
        /// @brief Modifies the right of the permission.
        virtual bool    setRight(const QString &right = "") = 0;
        /// @brief Returns true if the permission is granted.
        virtual bool    isGranted() = 0;
        /// @brief Grants or denies the permission.
        virtual bool    isGranted(bool granted) = 0;

        // Other
        /// @brief Returns true if the accessor possesses the right on the object.
        /// Read the database documentation to learn how the permissions works.
        virtual bool    isAllowed(const QString &id_accessor, const QString &id_object, const QString &right) = 0;
        /// @brief Allows to get all the rights of an accessor on an object.
        /// @param allowed : The list of the allowed rights. If it is empty,
        /// no rights are allowed. If it has an empty element, all the rights
        /// are allowed except the rights in denied.
        /// @param denied : The list of the denied rights. If it is empty,
        /// no rights are denied. If it has an empty element, all the rights
        /// are denied except the rights in allowed.
        /// @return False if either the accessor or the object doesn't exists.
        virtual bool    getRights(const QString &id_accessor, const QString &id_object, QStringList &allowed, QStringList &denied) = 0;
    };
}

Q_DECLARE_INTERFACE(LightBird::ITablePermissions, "cc.LightBird.ITablePermissions");

#endif // ITABLEPERMISSIONS_H
