#ifndef ITABLEGROUPS_H
# define ITABLEGROUPS_H

# include <QString>
# include <QDateTime>
# include <QMap>

# include "ITableAccessors.h"

namespace LightBird
{
    /// @brief Handle the transactions with the database relating to a group.
    /// Each modifications done in this object is immediatly saved in the database.
    class ITableGroups : virtual public LightBird::ITableAccessors
    {
    public:
        virtual ~ITableGroups() {}

        /// @brief Returns the id of the groups with this name.
        virtual QStringList getIdFromName(const QString &name) = 0;
        /// @brief Creates a new group.
        /// @param name : The name of the new account. It must be unique.
        /// @param id_group : The id of the parent group.
        /// @return True if the group has been created.
        virtual bool        add(const QString &name, const QString &id_group = "") = 0;

        // Fields
        /// @brief Returns the id of the parent of the group, or empty if
        /// it is at the root.
        virtual QString     getIdGroup() = 0;
        /// @brief Modifies the id of the parent of the group.
        virtual bool        setIdGroup(const QString &id_group = "") = 0;

        // Accounts
        /// @brief Add the group to the account in parameter.
        virtual bool        addAccount(const QString &id_account) = 0;
        /// @brief Remove the group from the account in parameter.
        virtual bool        removeAccount(const QString &id_account) = 0;
        /// @brief Returns the id of the accounts of the group.
        virtual QStringList getAccounts() = 0;
    };
}

Q_DECLARE_INTERFACE(LightBird::ITableGroups, "cc.lightbird.ITableGroups");

#endif // ITABLEGROUPS_H
