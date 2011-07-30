#ifndef TABLEDIRECTORIES_H
# define TABLEDIRECTORIES_H

# include "ITableDirectories.h"

# include "TableObjects.h"

class TableDirectories : virtual public TableObjects,
                         virtual public LightBird::ITableDirectories
{
public:
    TableDirectories(const QString &id = "");
    ~TableDirectories();
    TableDirectories(const TableDirectories &table);
    TableDirectories &operator=(const TableDirectories &table);

    QString     getIdFromVirtualPath(const QString &virtualPath) const;
    bool        setIdFromVirtualPath(const QString &virtualPath);
    bool        add(const QString &name, const QString &id_directory = "", const QString &id_account = "");

    QString     getIdDirectory() const;
    bool        setIdDirectory(const QString &id_directory = "");

    QString     getVirtualPath(bool initialSlash = false, bool finalSlash = false) const;
    bool        setVirtualPath(const QString &virtualPath);
    QStringList getDirectories(const QString &id_accessor = "", const QString &right = "") const;
    QStringList getFiles(const QString &id_accessor = "", const QString &right = "") const;
};

#endif // TABLEDIRECTORIES_H
