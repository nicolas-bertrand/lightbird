#ifndef ITABLEDIRECTORIES_H
# define ITABLEDIRECTORIES_H

# include <QMap>

# include "ITableObjects.h"

namespace LightBird
{
    /// @brief Handle the transactions with the database relating to a directory.
    /// Each modifications done in this object is immediatly saved in the database.
    /// This can be seen as an implementation of the Active Record design pattern.
    class ITableDirectories : virtual public LightBird::ITableObjects
    {
    public:
        virtual ~ITableDirectories() {}

        /// @brief Returns the id of the directory from its virtual path.
        virtual QString     getIdFromVirtualPath(const QString &virtualPath) = 0;
        /// @brief Set the id of the directory from its virtual path.
        virtual bool        setIdFromVirtualPath(const QString &virtualPath) = 0;
        /// @brief Creates a new directory from the given parameters.
        /// @param name : The name of the new directory.
        /// @param id_directory : The id of the parent directory, or empty
        /// if the new directory is at the root.
        /// @param id_account : The id of the account that owns the directory.
        /// @return If the directory has been correctly created.
        virtual bool        add(const QString &name, const QString &id_directory = "", const QString &id_account = "") = 0;

        // Fields
        /// @brief Returns the id of the parent of the directory. It is empty
        /// If the directory is at the root.
        virtual QString     getIdDirectory() = 0;
        /// @brief Modify the parent of the directory.
        virtual bool        setIdDirectory(const QString &id_directory = "") = 0;

        // Other
        /// @brief Returns the virtual path of the directory.
        /// @param initialSlash : If true, the first char of the result will be "/".
        /// @param initialSlash : If true, the last char of the result will be "/".
        virtual QString     getVirtualPath(bool initialSlash = false, bool finalSlash = false) = 0;
        /// @brief Moves the directory to the directory localized by the virtual
        /// path in parameter.
        virtual bool        setVirtualPath(const QString &virtualPath) = 0;
        /// @brief Returns the id of all the directories in the current directory.
        /// If id_accessor and right are not empty, only the directories for which the
        /// accessor has the right will be returned.
        /// @param id_accessor : If not empty, only the files for which this accessor has
        /// the right will be returned.
        /// @param right : The name of the right that the accessor must have.
        virtual QStringList getDirectories(const QString &id_accessor = "", const QString &right = "") = 0;
        /// @brief Returns the id of all the files in the current directory.
        /// If id_accessor and right are not empty, only the files for which the
        /// accessor has the right will be returned.
        /// @param id_accessor : If not empty, only the files for which this accessor has
        /// the right will be returned.
        /// @param right : The name of the right that the accessor must have.
        virtual QStringList getFiles(const QString &id_accessor = "", const QString &right = "") = 0;
    };
}

Q_DECLARE_INTERFACE(LightBird::ITableDirectories, "cc.lightbird.ITableDirectories");

#endif // ITABLEDIRECTORIES_H
