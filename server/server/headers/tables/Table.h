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

    LightBird::ITableAccessors   *toAccessors();
    LightBird::ITableAccounts    *toAccounts();
    LightBird::ITableCollections *toCollections();
    LightBird::ITableDirectories *toDirectories();
    LightBird::ITableEvents      *toEvents();
    LightBird::ITableFiles       *toFiles();
    LightBird::ITableGroups      *toGroups();
    LightBird::ITableLimits      *toLimits();
    LightBird::ITableObjects     *toObjects();
    LightBird::ITablePermissions *toPermissions();
    LightBird::ITableTags        *toTags();

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
