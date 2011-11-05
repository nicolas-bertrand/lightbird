#ifndef TABLEACCOUNTS_H
# define TABLEACCOUNTS_H

# include "ITableAccounts.h"

# include "TableAccessors.h"

class TableAccounts : virtual public TableAccessors,
                      virtual public LightBird::ITableAccounts
{
public:
    TableAccounts(const QString &id = "");
    ~TableAccounts();
    TableAccounts(const TableAccounts &table);
    TableAccounts &operator=(const TableAccounts &table);

    // Main
    bool            setId(const QString &id);
    QString         getIdFromName(const QString &name) const;
    bool            setIdFromName(const QString &name);
    QString         getIdFromNameAndPassword(const QString &name, const QString &password = "") const;
    bool            setIdFromNameAndPassword(const QString &name, const QString &password = "");
    QString         getIdFromIdentifiantAndSalt(const QString &identifiant, const QString &salt) const;
    bool            setIdFromIdentifiantAndSalt(const QString &identifiant, const QString &salt);
    bool            add(const QString &name, const QMap<QString, QVariant> &informations,
                        const QString &password = "", bool administrator = false, bool active = true);
    bool            add(const QString &name, const QString &password = "", bool administrator = false, bool active = true);

    // Fields
    QString         getPassword() const;
    bool            setPassword(const QString &password = "");
    bool            isAdministrator() const;
    bool            isAdministrator(bool administrator);
    bool            isActive() const;
    bool            isActive(bool active);

    // Informations
    QVariant        getInformation(const QString &name) const;
    QMap<QString, QVariant> getInformations() const;
    bool            setInformation(const QString &name, const QVariant &value);
    bool            setInformations(const QMap<QString, QVariant> &informations);
    bool            removeInformation(const QString &name);
    bool            removeInformations(const QStringList &informations);

    // Other
    QStringList     getGroups() const;
    bool            addGroup(const QString &id_group);
    bool            removeGroup(const QString &id_group);
    QString         passwordHash(const QString &password, const QString &id = "") const;
};

#endif // TABLEACCOUNTS_H
