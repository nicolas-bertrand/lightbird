#ifndef TABLE_H
# define TABLE_H

# include <QObject>

# include "ITable.h"

class Table : public QObject,
              virtual public LightBird::ITable
{
    Q_OBJECT
    Q_INTERFACES(LightBird::ITable)

public:
    const QString       &getId();
    virtual bool        setId(const QString &id);
    bool                exists(const QString &id = "");
    void                clear();
    bool                remove(const QString &id = "");

    QDateTime           getModified();
    QDateTime           getCreated();

    const QString       &getTableName();
    LightBird::ITable::Tables getTableId();
    bool                isTable(const QString &tableName);
    bool                isTable(LightBird::ITable::Tables tableId);

    LightBird::ITableAccessors   *toTableAccessors();
    LightBird::ITableAccounts    *toTableAccounts();
    LightBird::ITableCollections *toTableCollections();
    LightBird::ITableDirectories *toTableDirectories();
    LightBird::ITableEvents      *toTableEvents();
    LightBird::ITableFiles       *toTableFiles();
    LightBird::ITableGroups      *toTableGroups();
    LightBird::ITableLimits      *toTableLimits();
    LightBird::ITableObjects     *toTableObjects();
    LightBird::ITablePermissions *toTablePermissions();
    LightBird::ITableTags        *toTableTags();

    /// @brief This operator is virtual to allows subclasses to modify its behaviour.
    virtual Table       &operator=(const Table &t);

protected:
    Table();
    Table(const Table &t);
    ~Table();

    QString             id;
    QString             tableName;
    LightBird::ITable::Tables tableId;
};

#endif // TABLE_H
