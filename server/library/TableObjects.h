#ifndef LIGHTBIRD_TABLEOBJECTS_H
# define LIGHTBIRD_TABLEOBJECTS_H

# include <QString>
# include <QStringList>

# include "Table.h"

namespace LightBird
{
    /// @brief This class represents an object, which can be
    /// a file, a directory, or a collection.
    class LIB TableObjects : public LightBird::Table
    {
    public:
        ~TableObjects();

        // Fields
        /// @brief Returns the id of the account that possesses the object.
        QString     getIdAccount() const;
        /// @brief Modifies the id of the account that possesses the object.
        bool        setIdAccount(const QString &id_account = "");
        /// @brief Returns the name of the object.
        QString     getName() const;
        /// @brief Modifies the name of the object.
        bool        setName(const QString &name);

        // Other
        /// @brief Returns true if the accessor has the right on the object.
        /// @see LightBird::TablePermissions::isAllowed
        bool        isAllowed(const QString &id_accessor, const QString &right) const;
        /// @brief Allows to get the list of the rights that the accessor has
        /// on the object.
        /// @see LightBird::TablePermissions::getRights
        bool        getRights(const QString &id_accessor, QStringList &allowed, QStringList &denied) const;
        /// @brief Returns the id of the tags of the object.
        QStringList getTags() const;
        /// @brief Returns the limits of the object.
        QStringList getLimits() const;

    protected:
        TableObjects();
        TableObjects(const TableObjects &table);
        TableObjects &operator=(const TableObjects &table);
    };
}

#endif // LIGHTBIRD_TABLEOBJECTS_H
