#ifndef ITABLEOBJECTS_H
# define ITABLEOBJECTS_H

# include "ITable.h"

namespace Streamit
{
    /// @brief This class represents an object, which can be
    /// a file, a directory, or a collection.
    class ITableObjects : virtual public Streamit::ITable
    {
    public:
        virtual ~ITableObjects() {}

        // Fields
        /// @brief Returns the id of the account that possesses the object.
        virtual QString     getIdAccount() = 0;
        /// @brief Change the id of the account that possesses the object.
        virtual bool        setIdAccount(const QString &id_account = "") = 0;
        /// @brief Returns the name of the object.
        virtual QString     getName() = 0;
        /// @brief Modify the name of the object.
        virtual bool        setName(const QString &name) = 0;

        // Other
        /// @brief Returns true if the accessor has the right on the current directory.
        virtual bool        isAllowed(const QString &id_accessor, const QString &right) = 0;
        /// @brief Returns the list of the rights that the accessor has on the current directory.
        virtual QStringList getRights(const QString &id_accessor) = 0;
        /// @brief Returns the id of the tags of the object.
        virtual QStringList getTags() = 0;
    };
}

Q_DECLARE_INTERFACE(Streamit::ITableObjects, "cc.lightbird.ITableObjects");

#endif // ITABLEOBJECTS_H
