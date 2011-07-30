#ifndef TABLELIMITS_H
# define TABLELIMITS_H

# include "ITableLimits.h"

# include "Table.h"

class TableLimits : virtual public Table,
                    virtual public LightBird::ITableLimits
{
public:
    TableLimits(const QString &id = "");
    ~TableLimits();
    TableLimits(const TableLimits &table);
    TableLimits &operator=(const TableLimits &table);

    bool    add(const QString &name, const QString &value, const QString &id_accessor = "", const QString &id_object = "");

    QString getIdAccessor() const;
    bool    setIdAccessor(const QString &id_accessor = "");
    QString getIdObject() const;
    bool    setIdObject(const QString &id_object = "");
    QString getName() const;
    bool    setName(const QString &name);
    QString getValue() const;
    bool    setValue(const QString &value);
};

#endif // TABLELIMITS_H
