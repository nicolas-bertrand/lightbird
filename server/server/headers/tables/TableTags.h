#ifndef TABLETAGS_H
# define TABLETAGS_H

# include "ITableTags.h"

# include "Table.h"

class TableTags : virtual public Table,
                  virtual public LightBird::ITableTags
{
public:
    TableTags(const QString &id = "");
    TableTags(const TableTags &t);
    ~TableTags();
    TableTags &operator=(const TableTags &t);

    bool    add(const QString &id_object, const QString &name);

    QString getIdObject();
    bool    setIdObject(const QString &id_object);
    QString getName();
    bool    setName(const QString &name);
};

#endif // TABLETAGS_H
