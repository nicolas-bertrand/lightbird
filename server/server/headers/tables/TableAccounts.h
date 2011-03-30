#ifndef TABLEACCOUNTS_H
# define TABLEACCOUNTS_H

# include "ITableAccounts.h"

# include "TableAccessors.h"

class TableAccounts : virtual public TableAccessors,
                      virtual public Streamit::ITableAccounts
{
public:
    TableAccounts(const QString &id = "");
    TableAccounts(const TableAccounts &t);
    ~TableAccounts();
    TableAccounts &operator=(const TableAccounts &t);

    // Main
    bool            setId(const QString &id);
    bool            setIdFromName(const QString &name);
    QString         getIdFromNameAndPassword(const QString &name, const QString &password = "");
    bool            setIdFromNameAndPassword(const QString &name, const QString &password = "");
    QString         getIdFromIdentifiantAndSalt(const QString &identifiant, const QString &salt);
    bool            setIdFromIdentifiantAndSalt(const QString &identifiant, const QString &salt);
    bool            add(const QString &name, const QMap<QString, QVariant> &informations,
                        const QString &password = "", bool administrator = false, bool active = true);
    bool            add(const QString &name, const QString &password = "", bool administrator = false, bool active = true);

    // Fields
    QString         getPassword();
    bool            setPassword(const QString &password);
    bool            isAdministrator();
    bool            isAdministrator(bool administrator);
    bool            isActive();
    bool            isActive(bool active);

    // Informations
    QVariant        getInformation(const QString &name);
    bool            setInformation(const QString &name, const QVariant &value);
    QMap<QString, QVariant> getInformations();
    bool            setInformations(const QMap<QString, QVariant> &informations);
    bool            removeInformation(const QString &name);
    bool            removeInformations(const QStringList &informations);

    // Other
    const QDateTime &getConnectionDate();
    QStringList     getGroups();
    bool            addGroup(const QString &id_group);
    bool            removeGroup(const QString &id_group);

    /// @brief Allows to test the TableAccounts features.
    /// @return If the tests were successful.
    static bool     unitTests();

private:
    QDateTime       connectionDate; ///< The date of the connection of the client.
};

#endif // TABLEACCOUNTS_H
