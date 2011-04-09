#ifndef TABLEACCESSORS_H
# define TABLEACCESSORS_H

# include "ITableAccessors.h"

# include "Table.h"

class TableAccessors : virtual public Table,
                       virtual public LightBird::ITableAccessors
{
public:
    QString     getName();
    bool        setName(const QString &name);

    QStringList getLimits();

    virtual TableAccessors &operator=(const TableAccessors &t);

protected:
    TableAccessors();
    TableAccessors(const TableAccessors &t);
    ~TableAccessors();
};

#endif // TABLEACCESSORS_H
