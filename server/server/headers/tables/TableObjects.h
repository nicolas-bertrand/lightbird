#ifndef TABLEOBJECTS_H
# define TABLEOBJECTS_H

# include "ITableObjects.h"

# include "Table.h"

class TableObjects : virtual public Table,
                     virtual public LightBird::ITableObjects
{
public:
    QString     getIdAccount();
    bool        setIdAccount(const QString &id_account = "");
    QString     getName();
    bool        setName(const QString &name);

    bool        isAllowed(const QString &id_accessor, const QString &right);
    QStringList getRights(const QString &id_accessor);
    QStringList getTags();

    virtual TableObjects &operator=(const TableObjects &t);

protected:
    TableObjects();
    TableObjects(const TableObjects &t);
    ~TableObjects();
};

#endif // TABLEOBJECTS_H
