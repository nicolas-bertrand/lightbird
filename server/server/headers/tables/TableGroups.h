#ifndef TABLEGROUPS_H
# define TABLEGROUPS_H

# include "ITableGroups.h"

# include "TableAccessors.h"

class TableGroups : virtual public TableAccessors,
                    virtual public LightBird::ITableGroups
{
public:
    TableGroups(const QString &id = "");
    TableGroups(const TableGroups &t);
    ~TableGroups();
    TableGroups &operator=(const TableGroups &t);

    QStringList getIdFromName(const QString &name);
    bool        add(const QString &name, const QString &id_group = "");

    QString     getIdGroup();
    bool        setIdGroup(const QString &id_group = "");

    bool        addAccount(const QString &id_account);
    bool        removeAccount(const QString &id_account);
    QStringList getAccounts();
};

#endif // TABLEGROUPS_H
