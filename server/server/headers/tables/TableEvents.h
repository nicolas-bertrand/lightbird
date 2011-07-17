#ifndef TABLEEVENTS_H
# define TABLEEVENTS_H

# include "ITableEvents.h"

# include "Table.h"

class TableEvents : virtual public Table,
                    virtual public LightBird::ITableEvents
{
public:
    TableEvents(const QString &id = "");
    TableEvents(const TableEvents &e);
    ~TableEvents();
    TableEvents &operator=(const TableEvents &e);

    QStringList getEvents(const QString &name, const QDateTime &begin = QDateTime(), const QDateTime &end = QDateTime::currentDateTime());
    bool        add(const QString &name, const QMap<QString, QVariant> &informations,
                    const QString &id_accessor = "", const QString &id_object = "");
    bool        add(const QString &name, const QString &id_accessor = "", const QString &id_object = "");

    QString     getName();
    bool        setName(const QString &name);
    QString     getIdAccessor();
    bool        setIdAccessor(const QString &id_accessor = "");
    QString     getIdObject();
    bool        setIdObject(const QString &id_object = "");

    QVariant    getInformation(const QString &name);
    QMap<QString, QVariant> getInformations();
    bool        setInformation(const QString &name, const QVariant &value);
    bool        setInformations(const QMap<QString, QVariant> &informations);
    bool        removeInformation(const QString &name);
    bool        removeInformations(const QStringList &informations);
};

#endif // TABLEEVENTS_H
