#ifndef TABLEACCESSORS_H
# define TABLEACCESSORS_H

# include "ITableAccessors.h"

# include "Table.h"

class TableAccessors : virtual public Table,
                       virtual public LightBird::ITableAccessors
{
public:
    QString     getName() const;
    bool        setName(const QString &name);

    bool        isAllowed(const QString &id_object, const QString &right) const;
    bool        getRights(const QString &id_object, QStringList &allowed, QStringList &denied) const;
    QStringList getLimits() const;

    virtual TableAccessors &operator=(const TableAccessors &t);

protected:
    TableAccessors();
    TableAccessors(const TableAccessors &t);
    ~TableAccessors();
};

#endif // TABLEACCESSORS_H
