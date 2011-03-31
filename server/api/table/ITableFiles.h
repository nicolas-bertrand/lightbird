#ifndef ITABLEFILES_H
# define ITABLEFILES_H

# include <QMap>

# include "ITableCollections.h"
# include "ITableDirectories.h"
# include "ITableObjects.h"

namespace Streamit
{
    /// @brief Handle the transactions with the database relating to a file.
    /// Each modifications done in this object is immediatly saved in the database.
    /// This can be seen as an implementation of the Active Record design pattern.
    class ITableFiles : virtual public Streamit::ITableObjects
    {
    public:
        virtual ~ITableFiles() {}

        /// @brief Returns the id of a file, using its virtual path.
        virtual QString     getIdFromVirtualPath(const QString &virtualPath) = 0;
        /// @brief Set the id of a file, using its virtual path.
        virtual bool        setIdFromVirtualPath(const QString &virtualPath) = 0;
        /// @brief Add a new file.
        /// @param name : The name of the file.
        /// @param path : The path of the file.
        /// @param Informations : A map that contains informations about the file.
        /// @param type : The type of the file.
        /// @param id_directory : The id of the directory of the file. If empty,
        /// it is at the root.
        /// @param id_account : The id of account that owns the file.
        /// @return True if the file has been created.
        virtual bool        add(const QString &name, const QString &path, const QMap<QString, QVariant> &informations,
                                const QString &type = "", const QString &id_directory = "", const QString &id_account = "") = 0;
        /// @see add
        virtual bool        add(const QString &name, const QString &path, const QString &type = "",
                                const QString &id_directory = "", const QString &id_account = "") = 0;

        // Fields
        /// @brief Returns the path of the file.
        virtual QString     getPath() = 0;
        /// @brief If the path is relative, getPath returns the path from the
        /// filesPath directory, which is not the working directory of the server.
        /// The path returned by getFullPath contains the filesPath if it is relative,
        /// or just the path if it is absolute. Nothing is returned if the file has not
        /// been found.
        virtual QString     getFullPath() = 0;
        /// @brief Modify the path of the file.
        virtual bool        setPath(const QString &path) = 0;
        /// @brief Returns the type of the file.
        virtual QString     getType() = 0;
        /// @brief Modify the type of the file.
        virtual bool        setType(const QString &type) = 0;
        /// @brief Returns the id of the directory of the file.
        virtual QString     getIdDirectory() = 0;
        /// @brief Modify the id of the directory of the file.
        virtual bool        setIdDirectory(const QString &id_directory = "") = 0;

        // Informations
        /// @brief Returns the value of an information of the file.
        /// @param name : The name of the information to return.
        virtual QVariant    getInformation(const QString &name) = 0;
        /// @brief Modify the value of an information of the file, or create it if it doesn't exists.
        /// @param name : The name of the information to create of modify.
        /// @brief value : The new value of the information.
        virtual bool        setInformation(const QString &name, const QVariant &value) = 0;
        /// @brief Returns all the informations of the file.
        virtual QMap<QString, QVariant> getInformations() = 0;
        /// @brief Modifies or creates multiple informations for the file.
        /// @param informations : The infotmations to modify or create. The keys of the map are the
        /// keys of the informations, and the values of the map are the values of the informations.
        virtual bool        setInformations(const QMap<QString, QVariant> &informations) = 0;
        /// @brief Removes an information of the file.
        /// @param name : The name of the information to remove.
        virtual bool        removeInformation(const QString &name) = 0;
        /// @brief Removes multiple informations of the file.
        /// @param informations : This list contains the name of each informations to remove.
        virtual bool        removeInformations(const QStringList &informations) = 0;

        // Directories
        /// @brief Returns the virtual path of the file.
        /// @param initialSlash : If true, the first char of the result will be "/".
        /// @param fileName : If true, the file name is included in the result.
        virtual QString     getVirtualPath(bool initialSlash = false, bool fileName = false) = 0;
        /// @brief Moves the file to the directory localized by the virtual
        /// path in parameter.
        virtual bool        setVirtualPath(const QString &virtualPath) = 0;

        // Collections
        /// @brief Returns the list of the id collection of the file.
        virtual QStringList getCollections() = 0;
        /// @brief Add the file to the given collection.
        virtual bool        addCollection(const QString &id_collection) = 0;
        /// @brief Removes the file from the given collection.
        virtual bool        removeCollection(const QString &id_collection) = 0;
    };
}

Q_DECLARE_INTERFACE(Streamit::ITableFiles, "cc.lightbird.ITableFiles");

#endif // ITABLEFILES_H
