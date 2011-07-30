#ifndef TABLETAGS_H
# define TABLETAGS_H

# include "ITableTags.h"

# include "Table.h"

class TableTags : virtual public Table,
                  virtual public LightBird::ITableTags
{
public:
    TableTags(const QString &id = "");
    ~TableTags();
    TableTags(const TableTags &table);
    TableTags &operator=(const TableTags &table);

    bool    add(const QString &id_object, const QString &name);

    QString getIdObject() const;
    bool    setIdObject(const QString &id_object);
    QString getName() const;
    bool    setName(const QString &name);
};

#endif // TABLETAGS_H
