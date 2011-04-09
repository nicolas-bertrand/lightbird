#ifndef TABLECOLLECTIONS_H
# define TABLECOLLECTIONS_H

# include "ITableCollections.h"

# include "TableObjects.h"

class TableCollections : virtual public TableObjects,
                         virtual public LightBird::ITableCollections
{
public:
    TableCollections(const QString &id = "");
    TableCollections(const TableCollections &t);
    ~TableCollections();
    TableCollections  &operator=(const TableCollections &t);

    QString     getIdFromVirtualPath(const QString &virtualPath);
    bool        setIdFromVirtualPath(const QString &virtualPath);
    bool        add(const QString &name, const QString &id_collection = "", const QString &id_account = "");

    QString     getIdCollection();
    bool        setIdCollection(const QString &id_collection = "");

    QString     getVirtualPath(bool initialSlash = false, bool finalSlash = false);
    bool        setVirtualPath(const QString &virtualPath);
    QStringList getCollections(const QString &id_accessor = "", const QString &right = "");
    QStringList getFiles(const QString &id_accessor = "", const QString &right = "");

    /// @brief Allows to test the TableCollections features.
    /// @return If the tests were successful.
    static bool unitTests();
};

#endif // TABLECOLLECTIONS_H
