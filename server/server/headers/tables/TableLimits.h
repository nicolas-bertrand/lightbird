#ifndef TABLELIMITS_H
# define TABLELIMITS_H

# include "ITableLimits.h"

# include "Table.h"

class TableLimits : virtual public Table,
                    virtual public LightBird::ITableLimits
{
public:
    TableLimits(const QString &id = "");
    TableLimits(const TableLimits &t);
    ~TableLimits();
    TableLimits &operator=(const TableLimits &t);

    bool    add(const QString &name, const QString &value, const QString &id_accessor = "", const QString &id_object = "");

    QString getIdAccessor();
    bool    setIdAccessor(const QString &id_accessor = "");
    QString getIdObject();
    bool    setIdObject(const QString &id_object = "");
    QString getName();
    bool    setName(const QString &name);
    QString getValue();
    bool    setValue(const QString &value);

    /// @brief Allows to test the ITableLimits features.
    /// @return If the tests were successful.
    static bool unitTests();
};

#endif // TABLELIMITS_H
