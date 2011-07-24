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
    TableCollections &operator=(const TableCollections &t);

    QString     getIdFromVirtualPath(const QString &virtualPath) const;
    bool        setIdFromVirtualPath(const QString &virtualPath);
    bool        add(const QString &name, const QString &id_collection = "", const QString &id_account = "");

    QString     getIdCollection() const;
    bool        setIdCollection(const QString &id_collection = "");

    QString     getVirtualPath(bool initialSlash = false, bool finalSlash = false) const;
    bool        setVirtualPath(const QString &virtualPath);
    QStringList getCollections(const QString &id_accessor = "", const QString &right = "") const;
    QStringList getFiles(const QString &id_accessor = "", const QString &right = "") const;
};

#endif // TABLECOLLECTIONS_H
