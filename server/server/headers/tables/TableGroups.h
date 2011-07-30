#ifndef TABLEGROUPS_H
# define TABLEGROUPS_H

# include "ITableGroups.h"

# include "TableAccessors.h"

class TableGroups : virtual public TableAccessors,
                    virtual public LightBird::ITableGroups
{
public:
    TableGroups(const QString &id = "");
    ~TableGroups();
    TableGroups(const TableGroups &table);
    TableGroups &operator=(const TableGroups &table);

    QStringList getIdFromName(const QString &name) const;
    bool        add(const QString &name, const QString &id_group = "");

    QString     getIdGroup() const;
    bool        setIdGroup(const QString &id_group = "");

    bool        addAccount(const QString &id_account);
    bool        removeAccount(const QString &id_account);
    QStringList getAccounts() const;
};

#endif // TABLEGROUPS_H
