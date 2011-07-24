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

    QString     getIdFromVirtualPath(const QString &virtualPath) const;
    bool        setIdFromVirtualPath(const QString &virtualPath);
    bool        add(const QString &name, const QString &path, const QMap<QString, QVariant> &informations,
                    const QString &type = "", const QString &id_directory = "", const QString &id_account = "");
    bool        add(const QString &name, const QString &path, const QString &type = "",
                    const QString &id_directory = "", const QString &id_account = "");

    QString     getPath() const;
    QString     getFullPath() const;
    bool        setPath(const QString &path);
    QString     getType() const;
    bool        setType(const QString &type = "");
    QString     getIdDirectory() const;
    bool        setIdDirectory(const QString &id_directory = "");

    QString     getVirtualPath(bool initialSlash = false, bool fileName = false) const;
    bool        setVirtualPath(const QString &virtualPath);

    QVariant    getInformation(const QString &name) const;
    QMap<QString, QVariant> getInformations() const;
    bool        setInformation(const QString &name, const QVariant &value);
    bool        setInformations(const QMap<QString, QVariant> &informations);
    bool        removeInformation(const QString &name);
    bool        removeInformations(const QStringList &informations);

    QStringList getCollections() const;
    bool        addCollection(const QString &id_collection);
    bool        removeCollection(const QString &id_collection);

private:
    QStringList types;  ///< The list of the possible types.
};

#endif // TABLEFILES_H
