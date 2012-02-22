#ifndef LIGHTBIRD_TABLEGROUPS_H
# define LIGHTBIRD_TABLEGROUPS_H

# include <QString>
# include <QStringList>

# include "TableAccessors.h"

namespace LightBird
{
    /// @brief Handles the transactions with the database relating to a group.
    /// Each modifications done in this object is immediatly saved in the database.
    class LIB TableGroups : public LightBird::TableAccessors
    {
    public:
        TableGroups(const QString &id = "");
        ~TableGroups();
        TableGroups(const TableGroups &table);
        TableGroups &operator=(const TableGroups &table);

        /// @brief Returns the id of the groups with this name.
        QStringList getIdFromName(const QString &name) const;
        /// @brief Creates a new group.
        /// @param name : The name of the new account. It must be unique.
        /// @param id_group : The id of the parent group.
        /// @return True if the group has been created.
        bool        add(const QString &name, const QString &id_group = "");

        // Fields
        /// @brief Returns the id of the parent of the group, or empty if
        /// it is at the root.
        QString     getIdGroup() const;
        /// @brief Modifies the id of the parent of the group.
        bool        setIdGroup(const QString &id_group = "");

        // Accounts
        /// @brief Add the group to the account in parameter.
        bool        addAccount(const QString &id_account);
        /// @brief Remove the group from the account in parameter.
        bool        removeAccount(const QString &id_account);
        /// @brief Returns the id of the accounts of the group.
        QStringList getAccounts() const;
    };
}

#endif // LIGHTBIRD_TABLEGROUPS_H
