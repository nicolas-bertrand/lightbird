#ifndef TABLEEVENTS_H
# define TABLEEVENTS_H

# include <QMutex>

# include "Table.h"
# include "ITableEvents.h"

class TableEvents : virtual public Table,
                    virtual public Streamit::ITableEvents
{
public:
    TableEvents(const QString &id = "");
    TableEvents(const TableEvents &e);
    ~TableEvents();
    TableEvents   &operator=(const TableEvents &e);

    bool        add(const QString &name, const QMap<QString, QVariant> &informations,
                    const QString &id_accessor = "", const QString &id_object = "");
    bool        add(const QString &name, const QString &id_accessor = "", const QString &id_object = "");

    QString     getName();
    bool        setName(const QString &name);
    QString     getIdAccessor();
    bool        setIdAccessor(const QString &idAccessor = "");
    QString     getIdObject();
    bool        setIdObject(const QString &idObject = "");

    QVariant    getInformation(const QString &name);
    bool        setInformation(const QString &name, const QVariant &value);
    QMap<QString, QVariant> getInformations();
    bool        setInformations(const QMap<QString, QVariant> &informations);
    bool        removeInformation(const QString &name);
    bool        removeInformations(const QStringList &informations);

    QStringList getEvents(const QString &name, const QDateTime &begin = QDateTime(), const QDateTime &end = QDateTime::currentDateTime());

    /// @brief Allows to test the TableEvents features.
    /// @return If the tests were successful.
    static bool unitTests();
};

#endif // TABLEEVENTS_H
