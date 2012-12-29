#ifndef LIGHTBIRD_TABLEACCOUNTS_H
# define LIGHTBIRD_TABLEACCOUNTS_H

# include <QMap>
# include <QString>
# include <QStringList>
# include <QVariant>

# include "TableAccessors.h"

namespace LightBird
{
    /// @brief Handles the transactions with the database relating to an account.
    /// Each modifications done in this object is immediatly saved in the database.
    class LIB TableAccounts : public LightBird::TableAccessors
    {
    public:
        TableAccounts(const QString &id = "");
        ~TableAccounts();
        TableAccounts(const TableAccounts &table);
        TableAccounts &operator=(const TableAccounts &table);

        /// @brief Returns the id of an account from its name.
        /// @see setIdFromName
        QString         getIdFromName(const QString &name) const;
        /// @brief Identify an account using its name.
        /// @param name : The name of the account.
        /// @return True if the account exists.
        bool            setIdFromName(const QString &name);
        /// @brief Returns the id that corresponds to the parameters.
        /// @see setIdFromNameAndPassword
        QString         getIdFromNameAndPassword(const QString &name, const QString &password = "") const;
        /// @brief Allows to identify a client. The name and the password must
        /// match to an existing account. The id of the current instance of
        /// TableAccounts is set to the account.
        /// @param name : The name of the account.
        /// @param password : The password of the account. The password can be
        /// empty, for anonymous accounts.
        /// @return True if the name and the password match.
        bool            setIdFromNameAndPassword(const QString &name, const QString &password = "");
        /// @brief Returns the id that corresponds to the parameters.
        /// @see setIdFromIdentifiantAndSalt
        QString         getIdFromIdentifiantAndSalt(const QString &identifiant, const QString &salt) const;
        /// @brief Allows to identify an account without knowing its name or password.
        /// These informations are contained in the identifiant with a salt.
        /// @param identifiant : A SHA-256 composed of the name and the password of
        /// the account with a salt.
        /// @param salt : The salt used to create the identifiant.
        /// @return True if the identifiant and the salt are correct.
        bool            setIdFromIdentifiantAndSalt(const QString &identifiant, const QString &salt);
        /// @brief Creates a new account.
        /// @param name : The name of the new account.
        /// @param informations : The informations of the account.
        /// @param password : Its password. Can be empty.
        /// @param administrator : If the account is an administrator.
        /// @param active : If the account is activated.
        /// @return True if the account has been created.
        bool            add(const QString &name, const QVariantMap &informations,
                            const QString &password = "", bool administrator = false, bool active = true);
        /// @see add
        bool            add(const QString &name, const QString &password = "", bool administrator = false, bool active = true);

        // Fields
        /// @brief Returns the password as a SHA-256.
        /// @see passwordHash
        QString         getPassword() const;
        /// @brief Modifies the password of the account.
        bool            setPassword(const QString &password = "");
        /// @brief Returns true if the account is an administrator.
        bool            isAdministrator() const;
        /// @brief Defines if the account is an administrator or not.
        bool            isAdministrator(bool administrator);
        /// @brief Returns true if the account is a actived.
        bool            isActive() const;
        /// @brief Defines if the account is actived.
        bool            isActive(bool active);

        // Informations
        /// @brief Returns the value of an information of the account.
        /// @param name : The name of the information to return.
        QVariant        getInformation(const QString &name) const;
        /// @brief Returns all the informations of the account.
        QVariantMap     getInformations() const;
        /// @brief Modifies the value of an information of the account, or create
        /// it if it doesn't exists.
        /// @param name : The name of the information to create or modify.
        /// @brief value : The new value of the information.
        bool            setInformation(const QString &name, const QVariant &value);
        /// @brief Modifies or creates multiple informations for the account.
        /// @param informations : The informations to modify or create.
        /// The keys of the map are the keys of the informations, and
        /// the values of the map are the values of the informations.
        /// @return The list of the informations that could not be set.
        QStringList     setInformations(const QVariantMap &informations);
        /// @brief Removes an information of the account.
        /// @param name : The name of the information to remove.
        bool            removeInformation(const QString &name);
        /// @brief Removes multiple informations of the account.
        /// @param informations : This list contains the name of each
        /// informations to remove. If empty all the informations are removed.
        bool            removeInformations(const QStringList &informations = QStringList());

        // Other
        /// @brief Returns the ids of the groups of the account.
        QStringList     getGroups() const;
        /// @brief Add the account to the group in parameter.
        bool            addGroup(const QString &id_group);
        /// @brief Remove the account from the group in parameter.
        bool            removeGroup(const QString &id_group);
        /// @brief Converts a password into a hash that can be stored in the
        /// database. Returns the SHA-256 of the password concatened with the id.
        /// @param password : The password to hash.
        /// @param id : The id of the account of the password. It is used inside
        /// the SHA-256 with the password as a salt to ensure that two identical
        /// passwords will have different footprints. If it is empty, the SHA-256
        /// of the password is returned.
        /// @return The hash of the password and its salt.
        QString         passwordHash(const QString &password, const QString &id = "") const;
    };
}

#endif // LIGHTBIRD_TABLEACCOUNTS_H
