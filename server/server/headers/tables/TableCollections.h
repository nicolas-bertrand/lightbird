#ifndef TABLECOLLECTIONS_H
# define TABLECOLLECTIONS_H

# include "ITableCollections.h"

# include "TableObjects.h"

class TableCollections : virtual public TableObjects,
                         virtual public LightBird::ITableCollections
{
public:
    TableCollections(const QString &id = "");
    ~TableCollections();
    TableCollections(const TableCollections &table);
    TableCollections &operator=(const TableCollections &table);

    QString     getIdFromVirtualPath(const QString &virtualPath, const QString &id_account = "") const;
    bool        setIdFromVirtualPath(const QString &virtualPath, const QString &id_account = "");
    bool        add(const QString &name, const QString &id_collection = "", const QString &id_account = "");

    QString     getIdCollection() const;
    bool        setIdCollection(const QString &id_collection = "");

    QString     getVirtualPath(bool initialSlash = false, bool finalSlash = false) const;
    bool        setVirtualPath(const QString &virtualPath);
    QStringList getCollections(const QString &id_accessor = "", const QString &right = "") const;
    QStringList getFiles(const QString &id_accessor = "", const QString &right = "") const;
};

#endif // TABLECOLLECTIONS_H
