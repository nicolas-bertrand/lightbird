#ifndef TABLEPERMISSIONS_H
# define TABLEPERMISSIONS_H

# include "ITablePermissions.h"

# include "Table.h"

class TablePermissions : virtual public Table,
                         virtual public LightBird::ITablePermissions
{
public:
    TablePermissions(const QString &id = "");
    TablePermissions(const TablePermissions &t);
    ~TablePermissions();
    TablePermissions &operator=(const TablePermissions &t);

    QString     getId(const QString &id_accessor, const QString &id_object, const QString &right);
    bool        setId(const QString &id_accessor, const QString &id_object, const QString &right);
    bool        add(const QString &id_accessor, const QString &id_object, const QString &right, bool granted = true);

    QString     getIdAccessor();
    bool        setIdAccessor(const QString &id_accessor);
    QString     getIdObject();
    bool        setIdObject(const QString &id_object);
    QString     getRight();
    bool        setRight(const QString &right);
    bool        isGranted();
    bool        isGranted(bool granted);

    bool        isAllowed(const QString &id_accessor, const QString &id_object, const QString &right);
    QStringList getRights(const QString &id_accessor, const QString &id_object);

    /// @brief Allows to test the ITablePermissions features.
    /// @return If the tests were successful.
    static bool unitTests();

private:
    int         _checkPermission(const QStringList &accessors, const QString &idObject, const QString &right);
};

#endif // TABLEPERMISSIONS_H
