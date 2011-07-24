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
    const QString       &getId() const;
    virtual bool        setId(const QString &id);
    bool                exists(const QString &id = "");
    void                clear();
    bool                remove(const QString &id = "");

    QDateTime           getModified() const;
    QDateTime           getCreated() const;

    const QString       &getTableName() const;
    LightBird::ITable::Table getTableId() const;
    bool                isTable(const QString &tableName) const;
    bool                isTable(LightBird::ITable::Table tableId) const;

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
    LightBird::ITable::Table tableId;
};

#endif // TABLE_H
