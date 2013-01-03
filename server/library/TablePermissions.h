#ifndef LIGHTBIRD_TABLEPERMISSIONS_H
# define LIGHTBIRD_TABLEPERMISSIONS_H

# include <QList>
# include <QString>
# include <QStringList>
# include <QVariantMap>
# include <QVector>

# include "Table.h"

namespace LightBird
{
    /// @brief Handles the transactions with the database relating to a permission.
    /// Each modifications done in this object is immediatly saved in the database.
    class LIB TablePermissions : public LightBird::Table
    {
    public:
        TablePermissions(const QString &id = "");
        ~TablePermissions();
        TablePermissions(const TablePermissions &table);
        TablePermissions &operator=(const TablePermissions &table);

        /// @brief Returns the id of the permission that matches the parameters.
        QString     getId(const QString &id_accessor, const QString &id_object, const QString &right) const;
        /// @brief Set the id of the permission that matches the parameters.
        bool        setId(const QString &id_accessor, const QString &id_object, const QString &right);
        /// @brief Creates a new permission.
        /// @param id_accessor : The id of the accessor for which the permission is created.
        /// @param id_object : The id of the object for which the permission is created.
        /// @param right : The right of the permission.
        /// @param granted : If the permission is granted (true) or denied (false).
        /// @return True if the permission has been created.
        bool        add(const QString &id_accessor, const QString &id_object, const QString &right, bool granted = true);

        // Fields
        /// @brief Returns the id of the accessor on which the permission is applied.
        QString     getIdAccessor() const;
        /// @brief Changes the accessor on which the permission is applied.
        bool        setIdAccessor(const QString &id_accessor = "");
        /// @brief Returns the id of the object on which the permission is applied.
        QString     getIdObject() const;
        /// @brief Changes the object on which the permission is applied.
        bool        setIdObject(const QString &id_object = "");
        /// @brief Returns the right of the permission.
        QString     getRight() const;
        /// @brief Modifies the right of the permission.
        bool        setRight(const QString &right = "");
        /// @brief Returns true if the permission is granted.
        bool        isGranted() const;
        /// @brief Grants or denies the permission.
        bool        isGranted(bool granted);

        // Other
        /// @brief Returns true if the accessor possesses the right on the object.
        /// Read the database documentation to learn how the permissions works.
        bool        isAllowed(const QString &id_accessor, const QString &id_object, const QString &right) const;
        /// @brief Allows to get all the rights of an accessor on an object.
        /// @param allowed : The list of the allowed rights. If it is empty,
        /// no rights are allowed. If it has an empty element, all the rights
        /// are allowed except the rights in denied.
        /// @param denied : The list of the denied rights. If it is empty,
        /// no rights are denied. If it has an empty element, all the rights
        /// are denied except the rights in allowed.
        /// @return False if either the accessor or the object does not exist.
        bool        getRights(const QString &id_accessor, const QString &id_object, QStringList &allowed, QStringList &denied) const;

    private:
        /// @brief Check if the accessors has the right on the object.
        /// @return 2 if the permission is granted, 1 if it is not, and 0 if the answer is unknow.
        unsigned    _idAllowed(const QStringList &accessors, const QList<QStringList> &groups, const QString &id_object, const QString &right) const;
        /// @brief Check the rights of the accessors in the database.
        /// @return The same values as _isAllowed.
        unsigned    _checkRights(const QVector<QVariantMap> &rights, const QStringList &accessors, const QString &right) const;
        /// @brief Fills the lists allowed and denied with the rights of the accessors of the object.
        void        _getRights(const QStringList &accessors, const QList<QStringList> &groups, const QString &id_object, QStringList &allowed, QStringList &denied) const;
        /// @brief Merges the source rights to the destination rights.
        void        _mergeRights(QStringList &allowedSrc, QStringList &deniedSrc, QStringList &allowedDest, QStringList &deniedDest) const;
    };
}

#endif // LIGHTBIRD_TABLEPERMISSIONS_H
