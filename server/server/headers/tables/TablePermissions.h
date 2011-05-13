#ifndef TABLEPERMISSIONS_H
# define TABLEPERMISSIONS_H

# include <QList>
# include <QMap>
# include <QString>
# include <QStringList>
# include <QVector>

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
    bool        getRights(const QString &id_accessor, const QString &id_object, QStringList &allowed, QStringList &denied);

    /// @brief Allows to test the ITablePermissions features.
    /// @return If the tests were successful.
    static bool unitTests();

private:
    /// @brief Check if the accessors has the right on the object.
    /// @return 2 if the permission is granted, 1 if it is not, and 0 if the answer is unknow.
    unsigned    _idAllowed(const QStringList &accessors, const QList<QStringList> &groups, const QString &idObject, const QString &right);
    /// @brief Check the rights of the accessors in the database.
    /// @return The same values as _isAllowed.
    unsigned    _checkRights(const QVector<QMap<QString, QVariant> > &rights, const QStringList &accessors, const QString &right);
    /// @brief Fills the lists allowed and denied with the rights of the accessors of the object.
    void        _getRights(const QStringList &accessors, const QList<QStringList> &groups, const QString &idObject, QStringList &allowed, QStringList &denied);
    /// @brief Merges the source rights to the destination rights.
    void        _mergeRights(QStringList &allowedSrc, QStringList &deniedSrc, QStringList &allowedDest, QStringList &deniedDest);
};

#endif // TABLEPERMISSIONS_H
