#ifndef TABLEDIRECTORIES_H
# define TABLEDIRECTORIES_H

# include "ITableDirectories.h"

# include "TableObjects.h"

class TableDirectories : virtual public TableObjects,
                         virtual public Streamit::ITableDirectories
{
public:
    TableDirectories(const QString &id = "");
    TableDirectories(const TableDirectories &t);
    ~TableDirectories();
    TableDirectories  &operator=(const TableDirectories &t);

    QString     getIdFromVirtualPath(const QString &virtualPath);
    bool        setIdFromVirtualPath(const QString &virtualPath);
    bool        add(const QString &name, const QString &id_directory = "", const QString &id_account = "");

    QString     getIdDirectory();
    bool        setIdDirectory(const QString &id_directory = "");

    QString     getVirtualPath(bool initialSlash = false, bool finalSlash = false);
    bool        setVirtualPath(const QString &virtualPath);
    QStringList getDirectories(const QString &id_accessor = "", const QString &right = "");
    QStringList getFiles(const QString &id_accessor = "", const QString &right = "");

    /// @brief Allows to test the TableDirectories features.
    /// @return If the tests were successful.
    static bool unitTests();
};

#endif // TABLEDIRECTORIES_H
