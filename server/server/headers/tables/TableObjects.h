#ifndef TABLEOBJECTS_H
# define TABLEOBJECTS_H

# include "ITableObjects.h"

# include "Table.h"

class TableObjects : virtual public Table,
                     virtual public LightBird::ITableObjects
{
public:
    QString     getIdAccount() const;
    bool        setIdAccount(const QString &id_account = "");
    QString     getName() const;
    bool        setName(const QString &name);

    bool        isAllowed(const QString &id_accessor, const QString &right) const;
    bool        getRights(const QString &id_accessor, QStringList &allowed, QStringList &denied) const;
    QStringList getTags() const;
    QStringList getLimits() const;

protected:
    TableObjects();
    ~TableObjects();
    TableObjects(const TableObjects &table);
    TableObjects &operator=(const TableObjects &table);
};

#endif // TABLEOBJECTS_H
