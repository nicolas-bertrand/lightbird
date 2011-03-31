#ifndef ITABLEACCOUNTS_H
# define ITABLEACCOUNTS_H

# include <QString>
# include <QDateTime>
# include <QMap>
# include <QVariant>

# include "ITableAccessors.h"
# include "ITableGroups.h"

namespace Streamit
{
    /// @brief Handle the transactions with the database relating to an account.
    /// Each modifications done in this object is immediatly saved in the database.
    /// This can be seen as an implementation of the Active Record design pattern.
    class ITableAccounts : virtual public Streamit::ITableAccessors
    {
    public:
        virtual ~ITableAccounts() {}

        /// @brief Identify an account using its name.
        /// @param name : The name of the account.
        /// @return True if the account exists.
        virtual bool    setIdFromName(const QString &name) = 0;
        /// @brief Returns the id that corresponds to the parameters.
        /// @see setIdFromNameAndPassword
        virtual QString getIdFromNameAndPassword(const QString &name, const QString &password = "") = 0;
        /// @brief Use this function to identify a client. The name and the password must
        /// match to an existing account. If it is the case, the id of this instance is set
        /// to the account.
        /// @param name : The name of the account.
        /// @param password : The password of the account. The password can be empty,
        /// for anonymous accounts.
        /// @return True if the identifiants are correct.
        virtual bool    setIdFromNameAndPassword(const QString &name, const QString &password = "") = 0;
        /// @brief Returns the id that corresponds to the parameters.
        /// @see setIdFromIdentifiantAndSalt
        virtual QString getIdFromIdentifiantAndSalt(const QString &identifiant, const QString &salt) = 0;
        /// @brief This function allows to identify a client without knowing his name
        /// or his password. These informations are contains in the identifiant, with a salt.
        /// @param identifiant : The identifiant of the account.
        /// @param salt : The salt of the account.
        /// @return True if the identifiant and the salt are correct.
        virtual bool    setIdFromIdentifiantAndSalt(const QString &identifiant, const QString &salt) = 0;
        /// @brief Creates a new account from the given parameters.
        /// @param name : The name of the new account.
        /// @param informations : The informations of the account.
        /// @param password : Its password. Can be empty.
        /// @param administrator : If the account is an administrator.
        /// @param active : If the account is activated.
        /// @return If the account has been correctly created.
        virtual bool    add(const QString &name, const QMap<QString, QVariant> &informations,
                            const QString &password = "", bool administrator = false, bool active = true) = 0;
        /// @see add
        virtual bool    add(const QString &name, const QString &password = "", bool administrator = false,
                            bool active = true) = 0;

        // Fields
        /// @brief Returns the password of the account.
        virtual QString getPassword() = 0;
        /// @brief Modify the password of the account.
        virtual bool    setPassword(const QString &password) = 0;
        /// @brief Returns true if the account is an administrator.
        virtual bool    isAdministrator() = 0;
        /// @brief Defines if the account is an administrator or not.
        virtual bool    isAdministrator(bool administrator) = 0;
        /// @brief Returns true if the account is a actived.
        virtual bool    isActive() = 0;
        /// @brief Defines if the account is actived.
        virtual bool    isActive(bool active) = 0;

        // Informations
        /// @brief Returns the value of an information of the account.
        /// @param name : The name of the information to return.
        virtual QVariant getInformation(const QString &name) = 0;
        /// @brief Modify the value of an information of the account, or create it if it doesn't exists.
        /// @param name : The name of the information to create of modify.
        /// @brief value : The new value of the information.
        virtual bool    setInformation(const QString &name, const QVariant &value) = 0;
        /// @brief Returns all the informations of the account.
        virtual QMap<QString, QVariant> getInformations() = 0;
        /// @brief Modifies or creates multiple informations for the account.
        /// @param informations : The infotmations to modify or create. The keys of the map are the
        /// keys of the informations, and the values of the map are the values of the informations.
        virtual bool    setInformations(const QMap<QString, QVariant> &informations) = 0;
        /// @brief Removes an information of the account.
        /// @param name : The name of the information to remove.
        virtual bool    removeInformation(const QString &name) = 0;
        /// @brief Removes multiple informations of the account.
        /// @param informations : This list contains the name of each informations to remove.
        virtual bool    removeInformations(const QStringList &informations) = 0;

        // Other
        /// @brief Returns the date when the account id of the current instance has been
        /// set (using setId(), setIdFromNameAndPassword(), or setIdFromIdentifiantAndSalt()).
        virtual const QDateTime &getConnectionDate() = 0;
        /// @brief Returns the ids of the groups of the account.
        virtual QStringList getGroups() = 0;
        /// @brief Add the account to the group in parameter.
        virtual bool    addGroup(const QString &id_group) = 0;
        /// @brief Remove the account from the group in parameter.
        virtual bool    removeGroup(const QString &id_group) = 0;
    };
}

Q_DECLARE_INTERFACE(Streamit::ITableAccounts, "cc.lightbird.ITableAccounts");

#endif // ITABLEACCOUNTS_H
