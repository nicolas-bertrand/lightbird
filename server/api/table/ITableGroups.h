#ifndef ITABLEGROUPS_H
# define ITABLEGROUPS_H

# include <QString>
# include <QDateTime>
# include <QMap>

# include "ITableAccessors.h"

namespace Streamit
{
    /// @brief Handle the transactions with the database relating to a group.
    /// Each modifications done in this object is immediatly saved in the database.
    /// This can be seen as an implementation of the Active Record design pattern.
    class ITableGroups : virtual public Streamit::ITableAccessors
    {
    public:
        virtual ~ITableGroups() {}

        /// @brief Creates a new group, using the given parameter.
        /// @param name : The name of the new account. It must be unique.
        /// @param id_group : The id of the parent group.
        /// @return If the group has been correctly created.
        virtual bool    add(const QString &name, const QString &id_group = "") = 0;

        // Fields
        /// @brief Returns the id of the parent of the group, or empty if
        /// the group is at the root.
        virtual QString getIdGroup() = 0;
        /// @brief Modify the the id of the parent of the group.
        virtual bool    setIdGroup(const QString &id_group = "") = 0;

        // Accounts
        /// @brief Add the group to the account in parameter.
        virtual bool    addAccount(const QString &id_account) = 0;
        /// @brief Remove the group from the account in parameter.
        virtual bool    removeAccount(const QString &id_account) = 0;
        /// @brief Returns the id of the accounts of the group.
        virtual QStringList getAccounts() = 0;
    };
}

Q_DECLARE_INTERFACE (Streamit::ITableGroups, "fr.streamit.ITableGroups");

#endif // ITABLEGROUPS_H
