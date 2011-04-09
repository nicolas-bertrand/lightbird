#ifndef TABLEFILES_H
# define TABLEFILES_H

# include "ITableFiles.h"

# include "TableObjects.h"

class TableFiles : virtual public TableObjects,
                   virtual public LightBird::ITableFiles
{
public:
    TableFiles(const QString &id = "");
    TableFiles(const TableFiles &t);
    ~TableFiles();
    TableFiles  &operator=(const TableFiles &t);

    QString     getIdFromVirtualPath(const QString &virtualPath);
    bool        setIdFromVirtualPath(const QString &virtualPath);
    bool        add(const QString &name, const QString &path, const QMap<QString, QVariant> &informations,
                    const QString &type = "", const QString &id_directory = "", const QString &id_account = "");
    bool        add(const QString &name, const QString &path, const QString &type = "",
                    const QString &id_directory = "", const QString &id_account = "");

    QString     getPath();
    QString     getFullPath();
    bool        setPath(const QString &path);
    QString     getType();
    bool        setType(const QString &type);
    QString     getIdDirectory();
    bool        setIdDirectory(const QString &id_directory = "");

    QString     getVirtualPath(bool initialSlash = false, bool fileName = false);
    bool        setVirtualPath(const QString &virtualPath);

    QVariant    getInformation(const QString &name);
    bool        setInformation(const QString &name, const QVariant &value);
    QMap<QString, QVariant> getInformations();
    bool        setInformations(const QMap<QString, QVariant> &informations);
    bool        removeInformation(const QString &name);
    bool        removeInformations(const QStringList &informations);

    QStringList getCollections();
    bool        addCollection(const QString &id_collection);
    bool        removeCollection(const QString &id_collection);

    /// @brief Allows to test the TableFiles features.
    /// @return If the tests were successful.
    static bool unitTests();
};

#endif // TABLEFILES_H
