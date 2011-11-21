#ifndef ITABLEACCOUNTS_H
# define ITABLEACCOUNTS_H

# include <QDateTime>
# include <QMap>
# include <QString>
# include <QStringList>
# include <QVariant>

# include "ITableAccessors.h"

namespace LightBird
{
    /// @brief Handle the transactions with the database relating to an account.
    /// Each modifications done in this object is immediatly saved in the database.
    class ITableAccounts : virtual public LightBird::ITableAccessors
    {
    public:
        virtual ~ITableAccounts() {}

        /// @brief Returns the id of an account from its name.
        /// @see setIdFromName
        virtual QString getIdFromName(const QString &name) const = 0;
        /// @brief Identify an account using its name.
        /// @param name : The name of the account.
        /// @return True if the account exists.
        virtual bool    setIdFromName(const QString &name) = 0;
        /// @brief Returns the id that corresponds to the parameters.
        /// @see setIdFromNameAndPassword
        virtual QString getIdFromNameAndPassword(const QString &name, const QString &password = "") const = 0;
        /// @brief Allows to identify a client. The name and the password must
        /// match to an existing account. The id of the current instance of
        /// ITableAccounts is set to the account.
        /// @param name : The name of the account.
        /// @param password : The password of the account. The password can be
        /// empty, for anonymous accounts.
        /// @return True if the name and the password match.
        virtual bool    setIdFromNameAndPassword(const QString &name, const QString &password = "") = 0;
        /// @brief Returns the id that corresponds to the parameters.
        /// @see setIdFromIdentifiantAndSalt
        virtual QString getIdFromIdentifiantAndSalt(const QString &identifiant, const QString &salt) const = 0;
        /// @brief Allows to identify an account without knowing its name or password.
        /// These informations are contained in the identifiant with a salt.
        /// @param identifiant : A SHA-256 composed of the name and the password of
        /// the account with a salt.
        /// @param salt : The salt used to create the identifiant.
        /// @return True if the identifiant and the salt are correct.
        virtual bool    setIdFromIdentifiantAndSalt(const QString &identifiant, const QString &salt) = 0;
        /// @brief Creates a new account.
        /// @param name : The name of the new account.
        /// @param informations : The informations of the account.
        /// @param password : Its password. Can be empty.
        /// @param administrator : If the account is an administrator.
        /// @param active : If the account is activated.
        /// @return True if the account has been created.
        virtual bool    add(const QString &name, const QVariantMap &informations, const QString &password = "",
                            bool administrator = false, bool active = true) = 0;
        /// @see add
        virtual bool    add(const QString &name, const QString &password = "", bool administrator = false,
                            bool active = true) = 0;

        // Fields
        /// @brief Returns the password as a SHA-256.
        /// @see passwordHash
        virtual QString getPassword() const = 0;
        /// @brief Modifies the password of the account.
        virtual bool    setPassword(const QString &password = "") = 0;
        /// @brief Returns true if the account is an administrator.
        virtual bool    isAdministrator() const = 0;
        /// @brief Defines if the account is an administrator or not.
        virtual bool    isAdministrator(bool administrator) = 0;
        /// @brief Returns true if the account is a actived.
        virtual bool    isActive() const = 0;
        /// @brief Defines if the account is actived.
        virtual bool    isActive(bool active) = 0;

        // Informations
        /// @brief Returns the value of an information of the account.
        /// @param name : The name of the information to return.
        virtual QVariant getInformation(const QString &name) const = 0;
        /// @brief Returns all the informations of the account.
        virtual QVariantMap getInformations() const = 0;
        /// @brief Modifies the value of an information of the account, or create
        /// it if it doesn't exists.
        /// @param name : The name of the information to create or modify.
        /// @brief value : The new value of the information.
        virtual bool    setInformation(const QString &name, const QVariant &value) = 0;
        /// @brief Modifies or creates multiple informations for the account.
        /// @param informations : The informations to modify or create.
        /// The keys of the map are the keys of the informations, and
        /// the values of the map are the values of the informations.
        virtual bool    setInformations(const QVariantMap &informations) = 0;
        /// @brief Removes an information of the account.
        /// @param name : The name of the information to remove.
        virtual bool    removeInformation(const QString &name) = 0;
        /// @brief Removes multiple informations of the account.
        /// @param informations : This list contains the name of each
        /// informations to remove. If empty all the informations are removed.
        virtual bool    removeInformations(const QStringList &informations = QStringList()) = 0;

        // Other
        /// @brief Returns the ids of the groups of the account.
        virtual QStringList getGroups() const = 0;
        /// @brief Add the account to the group in parameter.
        virtual bool    addGroup(const QString &id_group) = 0;
        /// @brief Remove the account from the group in parameter.
        virtual bool    removeGroup(const QString &id_group) = 0;
        /// @brief Converts a password into a hash that can be stored in the
        /// database. Returns the SHA-256 of the password concatened with the id.
        /// @param password : The password to hash.
        /// @param id : The id of the account of the password. It is used inside
        /// the SHA-256 with the password as a salt to ensure that two identical
        /// passwords will have different footprints. If it is empty, the SHA-256
        /// of the password is returned.
        /// @return The hash of the password and its salt.
        virtual QString passwordHash(const QString &password, const QString &id = "") const = 0;
    };
}

Q_DECLARE_INTERFACE(LightBird::ITableAccounts, "cc.lightbird.ITableAccounts");

#endif // ITABLEACCOUNTS_H
