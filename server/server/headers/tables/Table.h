#ifndef TABLE_H
# define TABLE_H

# include <QObject>

# include "ITable.h"

class Table : public QObject,
              virtual public Streamit::ITable
{
    Q_OBJECT
    Q_INTERFACES(Streamit::ITable)

public:
    const QString       &getId();
    virtual bool        setId(const QString &id);
    bool                exists(const QString &id = "");
    void                clear();
    bool                remove(const QString &id = "");

    QDateTime           getModified();
    QDateTime           getCreated();

    const QString       &getTableName();
    Streamit::ITable::Tables getTableId();
    bool                isTable(const QString &tableName);
    bool                isTable(Streamit::ITable::Tables tableId);

    Streamit::ITableAccessors   *toTableAccessors();
    Streamit::ITableAccounts    *toTableAccounts();
    Streamit::ITableCollections *toTableCollections();
    Streamit::ITableDirectories *toTableDirectories();
    Streamit::ITableEvents      *toTableEvents();
    Streamit::ITableFiles       *toTableFiles();
    Streamit::ITableGroups      *toTableGroups();
    Streamit::ITableLimits      *toTableLimits();
    Streamit::ITableObjects     *toTableObjects();
    Streamit::ITablePermissions *toTablePermissions();
    Streamit::ITableTags        *toTableTags();

    /// @brief This operator is virtual to allows subclasses to modify its behaviour.
    virtual Table       &operator=(const Table &t);

protected:
    Table();
    Table(const Table &t);
    ~Table();

    QString             id;
    QString             tableName;
    Streamit::ITable::Tables tableId;
};

#endif // TABLE_H
