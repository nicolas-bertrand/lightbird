#include <QUuid>

#include "Configurations.h"
#include "Database.h"
#include "Defines.h"
#include "Log.h"
#include "TableAccounts.h"
#include "TableGroups.h"
#include "TableCollections.h"
#include "TableDirectories.h"
#include "TableFiles.h"
#include "TablePermissions.h"

TablePermissions::TablePermissions(const QString &id)
{
    this->tableName = "permissions";
    this->tableId = Streamit::ITable::Permissions;
    if (!id.isEmpty())
        Table::setId(id);
}

TablePermissions::~TablePermissions()
{
}

TablePermissions::TablePermissions(const TablePermissions &t) : Table()
{
    *this = t;
}

TablePermissions &TablePermissions::operator=(const TablePermissions &t)
{
    if (this != &t)
    {
        this->id = t.id;
        this->tableId = t.tableId;
        this->tableName = t.tableName;
    }
    return (*this);
}

QString TablePermissions::getId(const QString &id_accessor, const QString &id_object, const QString &right)
{
    QSqlQuery                           query;
    QVector<QMap<QString, QVariant> >   result;

    query.prepare(Database::instance()->getQuery("TablePermissions", "getId"));
    query.bindValue(":id_accessor", id_accessor);
    query.bindValue(":id_object", id_object);
    query.bindValue(":right", right);
    if (Database::instance()->query(query, result) && result.size() > 0)
        return (result[0]["id"].toString());
    return ("");
}

bool        TablePermissions::setId(const QString &id_accessor, const QString &id_object, const QString &right)
{
    QString id;

    if ((id = this->getId(id_accessor, id_object, right)).isEmpty())
        return (false);
    this->id = id;
    return (true);
}

bool    TablePermissions::add(const QString &id_accessor, const QString &id_object, const QString &right, bool granted)
{
    QSqlQuery   query;
    QString     id;

    id = QUuid::createUuid().toString().remove(0, 1).remove(36, 1);
    query.prepare(Database::instance()->getQuery("TablePermissions", "add"));
    query.bindValue(":id", id);
    query.bindValue(":id_accessor", id_accessor);
    query.bindValue(":id_object", id_object);
    query.bindValue(":right", right);
    query.bindValue(":granted", QString::number(granted));
    if (!Database::instance()->query(query) || query.numRowsAffected() == 0)
        return (false);
    this->id = id;
    return (true);
}

QString TablePermissions::getIdAccessor()
{
    QSqlQuery                           query;
    QVector<QMap<QString, QVariant> >   result;

    query.prepare(Database::instance()->getQuery("TablePermissions", "getIdAccessor"));
    query.bindValue(":id", this->id);
    if (Database::instance()->query(query, result) && result.size() > 0)
        return (result[0]["id_accessor"].toString());
    return ("");
}

bool    TablePermissions::setIdAccessor(const QString &id_accessor)
{
    QSqlQuery   query;

    query.prepare(Database::instance()->getQuery("TablePermissions", "setIdAccessor"));
    query.bindValue(":id", this->id);
    query.bindValue(":id_accessor", id_accessor);
    return (Database::instance()->query(query));
}

QString TablePermissions::getIdObject()
{
    QSqlQuery                           query;
    QVector<QMap<QString, QVariant> >   result;

    query.prepare(Database::instance()->getQuery("TablePermissions", "getIdObject"));
    query.bindValue(":id", this->id);
    if (Database::instance()->query(query, result) && result.size() > 0)
        return (result[0]["id_object"].toString());
    return ("");
}

bool    TablePermissions::setIdObject(const QString &id_object)
{
    QSqlQuery   query;

    query.prepare(Database::instance()->getQuery("TablePermissions", "setIdObject"));
    query.bindValue(":id", this->id);
    query.bindValue(":id_object", id_object);
    return (Database::instance()->query(query));
}

QString TablePermissions::getRight()
{
    QSqlQuery                           query;
    QVector<QMap<QString, QVariant> >   result;

    query.prepare(Database::instance()->getQuery("TablePermissions", "getRight"));
    query.bindValue(":id", this->id);
    if (Database::instance()->query(query, result) && result.size() > 0)
        return (result[0]["right"].toString());
    return ("");
}

bool    TablePermissions::setRight(const QString &right)
{
    QSqlQuery   query;

    query.prepare(Database::instance()->getQuery("TablePermissions", "setRight"));
    query.bindValue(":id", this->id);
    query.bindValue(":right", right);
    return (Database::instance()->query(query));
}

bool    TablePermissions::isGranted()
{
    QSqlQuery                           query;
    QVector<QMap<QString, QVariant> >   result;

    query.prepare(Database::instance()->getQuery("TablePermissions", "getGranted"));
    query.bindValue(":id", this->id);
    if (Database::instance()->query(query, result) && result.size() > 0)
        return (result[0]["granted"].toBool());
    return (false);
}

bool    TablePermissions::isGranted(bool granted)
{
    QSqlQuery   query;

    query.prepare(Database::instance()->getQuery("TablePermissions", "setGranted"));
    query.bindValue(":id", this->id);
    query.bindValue(":granted", QString::number(granted));
    return (Database::instance()->query(query));
}

bool    TablePermissions::isAllowed(const QString &id_accessor, const QString &id_object, const QString &right)
{
    Streamit::ITableAccounts    *account = NULL;
    Streamit::ITableFiles       *file = NULL;
    Streamit::ITableDirectories *directory = NULL;
    Streamit::ITableCollections *collection = NULL;
    bool                        inheritance = false;
    bool                        ownerInheritance = false;
    bool                        checked = false;
    int                         granted = 0;
    QString                     id;
    TableDirectories            dir;
    TableCollections            col;
    QStringList                 accessors;

    // If the permissions system is not activated, we don't need to check the rights
    if (Configurations::instance()->get("permissions/activate") != "true")
        return (true);
    if (Configurations::instance()->get("permissions/ownerInheritance") == "true")
        ownerInheritance = true;
    if (Configurations::instance()->get("permissions/inheritance") == "true")
        inheritance = true;
    QSharedPointer<Streamit::ITableAccessors> accessor(dynamic_cast<Streamit::ITableAccessors *>(Database::instance()->getTable(Streamit::ITable::Accessor, id_accessor)));
    if (accessor.isNull())
        return (false);
    if (accessor->isTable(Streamit::ITable::Accounts))
        account = dynamic_cast<Streamit::ITableAccounts *>(accessor.data());
    // If the accessor is an account, and the account is administrator, he has all the rights on all the objects
    if (account && account->isAdministrator())
        return (true);
    QSharedPointer<Streamit::ITableObjects> object(dynamic_cast<Streamit::ITableObjects *>(Database::instance()->getTable(Streamit::ITable::Object, id_object)));
    if (object.isNull())
        return (false);
    // If the accessor is an account, and the account is the owner of the object, he has all the rights on it
    if (account && account->getId() == object->getIdAccount())
        return (true);
    if (account)
        accessors = account->getGroups();
    accessors.push_front(accessor->getId());
    if (object->isTable(Streamit::ITable::Files))
    {
        file = dynamic_cast<Streamit::ITableFiles *>(object.data());
        if (!granted)
            if ((granted = this->_checkPermission(accessors, file->getId(), right)) == 2)
                return (true);
        id = file->getIdDirectory();
        checked = true;
    }
    if (object->isTable(Streamit::ITable::Directories))
    {
        directory = dynamic_cast<Streamit::ITableDirectories *>(object.data());
        id = directory->getId();
    }
    if (object->isTable(Streamit::ITable::Collections))
    {
        collection = dynamic_cast<Streamit::ITableCollections *>(object.data());
        id = collection->getId();
    }
    // Run through the objects hierarchy to find the permissions
    if (file || directory)
        while (!id.isEmpty())
        {
            if (!granted && (inheritance || !checked) && (granted = this->_checkPermission(accessors, id, right)) == 2)
                return (true);
            dir.setId(id);
            if (account && ownerInheritance && account->getId() == dir.getIdAccount())
                return (true);
            id = dir.getIdDirectory();
            checked = true;
        }
    else if (collection)
        while (!id.isEmpty())
        {
            if (!granted && (inheritance || !checked) && (granted = this->_checkPermission(accessors, id, right)) == 2)
                return (true);
            col.setId(id);
            if (account && ownerInheritance && account->getId() == col.getIdAccount())
                return (true);
            id = col.getIdCollection();
            checked = true;
        }
    // The permission has been granted
    if (granted == 2)
        return (true);
    // Check if there is a permission at the root of the server
    if (!granted && inheritance && (granted = this->_checkPermission(accessors, "", right)) == 2)
        return (true);
    // If there no permissions for the accessor on the object, the default is used
    if (granted == 0 && Configurations::instance()->get("permissions/default") == "true")
        return (true);
    return (false);
}

QStringList TablePermissions::getRights(const QString &id_accessor, const QString &id_object)
{
    Streamit::ITableAccounts            *account = NULL;
    Streamit::ITableFiles               *file = NULL;
    Streamit::ITableDirectories         *directory = NULL;
    Streamit::ITableCollections         *collection = NULL;
    QString                             id;
    TableDirectories                    dir;
    TableCollections                    col;
    QStringList                         allowed;
    QStringList                         denied;
    QSqlQuery                           query;
    QVector<QMap<QString, QVariant> >   result;
    int                                 i;
    int                                 s;

    QSharedPointer<Streamit::ITableAccessors> accessor(dynamic_cast<Streamit::ITableAccessors *>(Database::instance()->getTable(Streamit::ITable::Accessor, id_accessor)));
    if (accessor.isNull())
        return (allowed);
    if (accessor->isTable(Streamit::ITable::Accounts))
        account = dynamic_cast<Streamit::ITableAccounts *>(accessor.data());
    QSharedPointer<Streamit::ITableObjects> object(dynamic_cast<Streamit::ITableObjects *>(Database::instance()->getTable(Streamit::ITable::Object, id_object)));
    if (object.isNull())
        return (allowed);
    if (object->isTable(Streamit::ITable::Files))
    {
        file = dynamic_cast<Streamit::ITableFiles *>(object.data());
        id = file->getId();
    }
    if (object->isTable(Streamit::ITable::Directories))
    {
        directory = dynamic_cast<Streamit::ITableDirectories *>(object.data());
        id = directory->getId();
    }
    if (object->isTable(Streamit::ITable::Collections))
    {
        collection = dynamic_cast<Streamit::ITableCollections *>(object.data());
        id = collection->getId();
    }
    while (!id.isEmpty())
    {
        query.prepare(Database::instance()->getQuery("TablePermissions", "getRights"));
        query.bindValue(":id_accessor", id_accessor);
        query.bindValue(":id_object", id);
        if (Database::instance()->query(query, result))
            for (i = 0, s = result.size(); i < s; ++i)
                if (result[i]["granted"].toBool() && !denied.contains(result[i]["right"].toString()))
                    allowed << result[i]["right"].toString();
                else if (!result[i]["granted"].toBool())
                {
                    denied << result[i]["right"].toString();
                    allowed.removeAll(result[i]["right"].toString());
                }
        if (file || directory)
        {
            if (!dir.setId(id) && file)
                id = file->getIdDirectory();
            else
                id = dir.getIdDirectory();
        }
        else
        {
            col.setId(id);
            id = col.getIdCollection();
        }
    }
    return (allowed);
}

int     TablePermissions::_checkPermission(const QStringList &accessors, const QString &idObject, const QString &right)
{
    QSqlQuery                           query;
    QVector<QMap<QString, QVariant> >   result;
    QStringList                         rights;
    int                                 granted = 0;
    bool                                sorted = false;
    int                                 i;
    int                                 s;

    rights.push_back(right);
    // Someone who can modify can read
    if (rights.contains("read"))
        rights.push_back("modify");
    // Someone who can delete can modify
    if (rights.contains("modify"))
        rights.push_back("delete");
    query.prepare(Database::instance()->getQuery("TablePermissions", "_checkPermission")
                  .replace(":accessors", "'" + accessors.join("','") + "'").replace(":rights", "'" + rights.join("','") + "'"));
    query.bindValue(":id_object", idObject);
    if (!Database::instance()->query(query, result) || result.size() <= 0)
        return (0);
    // Removes the useless rights
    for (i = 0, s = result.size(); i < s && !sorted; ++i)
        if (result[i]["right"] == right)
        {
            QMutableVectorIterator<QMap<QString, QVariant> > it(result);
            while (it.hasNext())
                if (it.next().value("right") != right)
                    it.remove();
            sorted = true;
        }
    if (right == "read")
        for (i = 0, s = result.size(); i < s && !sorted; ++i)
            if (result[i]["right"] == "modify")
            {
                QMutableVectorIterator<QMap<QString, QVariant> > it(result);
                while (it.hasNext())
                    if (it.next().value("right") != "modify")
                        it.remove();
                sorted = true;
            }
    if (right == "read" || right == "modify")
        for (i = 0, s = result.size(); i < s && !sorted; ++i)
            if (result[i]["right"] == "delete")
            {
                QMutableVectorIterator<QMap<QString, QVariant> > it(result);
                while (it.hasNext())
                    if (it.next().value("right") != "delete")
                        it.remove();
                sorted = true;
            }
    for (i = 0, s = result.size(); i < s && !sorted; ++i)
        if (!result[i]["right"].toString().isEmpty())
        {
            QMutableVectorIterator<QMap<QString, QVariant> > it(result);
            while (it.hasNext())
                if (it.next().value("right").toString().isEmpty())
                    it.remove();
            sorted = true;
        }
    // Check the right of the accessor
    for (i = 0, s = result.size(); i < s; ++i)
        if (result[i]["id_accessor"] == accessors.front())
            return (result[i]["granted"].toBool() + 1);
    for (i = 0, s = result.size(); i < s && granted != 2; ++i)
        if (!result[i]["id_accessor"].toString().isEmpty())
            granted = result[i]["granted"].toBool() + 1;
    if (granted == 0)
        for (i = 0, s = result.size(); i < s && granted != 2; ++i)
            if (result[i]["id_accessor"].toString().isEmpty())
                granted = result[i]["granted"].toBool() + 1;
    return (granted);
}

bool                    TablePermissions::unitTests()
{
    TablePermissions    p;
    TableAccounts       a;
    TableGroups         g1;
    TableGroups         g2;
    TableDirectories    d1;
    TableDirectories    d2;
    TableCollections    c;
    TableFiles          f;
    QSqlQuery           query;
    QString             id;
    QStringList         rights;
    QString             id_object;

    Log::instance()->debug("Running unit tests...", "TablePermissions", "unitTests");
    query.prepare("DELETE FROM directories WHERE name IN('d', 'Dossier1', 'Dossier6', 'Dossier7')");
    Database::instance()->query(query);
    query.prepare("DELETE FROM accounts WHERE name IN('a')");
    Database::instance()->query(query);
    query.prepare("DELETE FROM groups WHERE name IN('g1', 'g2')");
    Database::instance()->query(query);
    Configurations::instance()->set("permissions/activate", "true");
    Configurations::instance()->set("permissions/default", "false");
    Configurations::instance()->set("permissions/inheritance", "true");
    Configurations::instance()->set("permissions/ownerInheritance", "true");
    try
    {
        ASSERT(d1.add("d"));
        ASSERT(a.add("a"));
        ASSERT(p.add(a.getId(), d1.getId(), "read", true));
        ASSERT(!p.add(a.getId(), d1.getId(), "read", false));
        ASSERT(p.getIdAccessor() == a.getId());
        ASSERT(p.setIdAccessor(a.getId()));
        ASSERT(p.getIdAccessor() == a.getId());
        ASSERT(p.getIdObject() == d1.getId());
        ASSERT(p.setIdObject(d1.getId()));
        ASSERT(p.getIdObject() == d1.getId());
        ASSERT(!p.setIdObject(a.getId()));
        ASSERT(p.setIdObject(""));
        ASSERT(p.setIdObject(d1.getId()));
        ASSERT(p.getRight() == "read");
        ASSERT(p.setRight("write"));
        ASSERT(p.getRight() == "write");
        ASSERT(p.isGranted());
        ASSERT(p.isGranted());
        ASSERT(p.isGranted(false));
        ASSERT(!p.isGranted());
        ASSERT(p.isGranted(true));
        ASSERT(p.isGranted());
        ASSERT(p.exists());
        ASSERT(a.remove());
        ASSERT(d1.remove());
        ASSERT(!p.exists());
        ASSERT(a.add("a"));
        id = a.getId();
        ASSERT(d1.add("Dossier6"));
        ASSERT(p.add(id, d1.getId(), "read", true));
        ASSERT(f.add("Fichier7", "Fichier7", "", d1.getId()));
        ASSERT(d1.add("Dossier7"));
        ASSERT(f.add("Fichier9", "Fichier9", "", d1.getId()));
        ASSERT(d1.add("Dossier8", d1.getId()));
        ASSERT(p.add(id, d1.getId(), "read", true));
        ASSERT(f.add("Fichier8", "Fichier8", "", d1.getId()));
        ASSERT(d1.add("Dossier1"));
        ASSERT(d2.add("Dossier5", d1.getId()));
        ASSERT(d2.add("Dossier2", d1.getId()));
        ASSERT(p.add(id, d2.getId(), "read", true));
        ASSERT(f.add("Fichier5", "Fichier5", "", d1.getId()));
        ASSERT(f.add("Fichier6", "Fichier6", "", d1.getId()));
        ASSERT(p.add(id, f.getId(), "read", true));
        ASSERT(d1.add("Dossier3", d2.getId()));
        ASSERT(p.add(id, d1.getId(), "read", false));
        ASSERT(f.add("Fichier1", "Fichier1", "", d1.getId()));
        ASSERT(f.add("Fichier2", "Fichier2", "", d1.getId()));
        ASSERT(p.add(id, f.getId(), "read", false));
        ASSERT(d1.add("Dossier4", d2.getId()));
        ASSERT(f.add("Fichier3", "Fichier3", "", d1.getId()));
        ASSERT(p.add(id, f.getId(), "read", false));
        ASSERT(f.add("Fichier4", "Fichier4", "", d2.getId()));
        ASSERT(!p.isAllowed(id, d1.getIdFromVirtualPath("Dossier1"), "read"));
        ASSERT(p.isAllowed(id, d1.getIdFromVirtualPath("Dossier1/Dossier2"), "read"));
        ASSERT(!p.isAllowed(id, d1.getIdFromVirtualPath("Dossier1/Dossier2/Dossier3"), "read"));
        ASSERT(!p.isAllowed(id, f.getIdFromVirtualPath("Dossier1/Dossier2/Dossier3/Fichier1"), "read"));
        ASSERT(!p.isAllowed(id, f.getIdFromVirtualPath("Dossier1/Dossier2/Dossier3/Fichier2"), "read"));
        ASSERT(p.isAllowed(id, d1.getIdFromVirtualPath("Dossier1/Dossier2/Dossier4"), "read"));
        ASSERT(p.isAllowed(id, f.getIdFromVirtualPath("Dossier1/Dossier2/Fichier4"), "read"));
        ASSERT(!p.isAllowed(id, d1.getIdFromVirtualPath("Dossier1/Dossier5"), "read"));
        ASSERT(!p.isAllowed(id, f.getIdFromVirtualPath("Dossier1/Fichier5"), "read"));
        ASSERT(p.isAllowed(id, f.getIdFromVirtualPath("Dossier1/Fichier6"), "read"));
        ASSERT(p.isAllowed(id, d1.getIdFromVirtualPath("Dossier6"), "read"));
        ASSERT(p.isAllowed(id, f.getIdFromVirtualPath("Dossier6/Fichier7"), "read"));
        ASSERT(!p.isAllowed(id, d1.getIdFromVirtualPath("Dossier7"), "read"));
        ASSERT(p.isAllowed(id, d1.getIdFromVirtualPath("Dossier7/Dossier8"), "read"));
        ASSERT(p.isAllowed(id, f.getIdFromVirtualPath("Dossier7/Dossier8/Fichier8"), "read"));
        ASSERT(!p.isAllowed(id, f.getIdFromVirtualPath("Dossier7/Fichier9"), "read"));
        ASSERT(c.add("Collection1"));
        ASSERT(!p.isAllowed(id, c.getId(), "read"));
        ASSERT(p.add(id, c.getId(), "read"));
        ASSERT(p.isAllowed(id, c.getId(), "read"));
        ASSERT(p.remove());
        ASSERT(!p.isAllowed(id, c.getId(), "read"));
        ASSERT(a.isAdministrator(true));
        ASSERT(p.isAllowed(id, c.getId(), "read"));
        ASSERT(a.isAdministrator(false));
        ASSERT(!p.isAllowed(id, c.getId(), "read"));
        ASSERT(c.setIdAccount(id));
        ASSERT(p.isAllowed(id, c.getId(), "read"));
        id_object = f.getIdFromVirtualPath("Dossier1/Dossier2/Fichier4");
        ASSERT(p.getRights(id, id_object).contains("read"));
        ASSERT(p.add(id, id_object, "write"));
        ASSERT((rights = p.getRights(id, id_object)).size() == 2);
        ASSERT(rights.contains("read"));
        ASSERT(rights.contains("write"));
        ASSERT(!p.getRights(id, d1.getIdFromVirtualPath("Dossier1/Dossier5")).size());
        ASSERT(p.getRights(id, d1.getIdFromVirtualPath("Dossier7/Dossier8")).contains("read"));
        id_object = f.getIdFromVirtualPath("Dossier1/Dossier2/Dossier3/Fichier2");
        ASSERT(!p.getRights(id, id_object).size());
        ASSERT(p.add(id, d1.getIdFromVirtualPath("Dossier1/Dossier2"), "add"));
        ASSERT(p.getRights(id, id_object).contains("add"));
        ASSERT(p.add(id, d1.getIdFromVirtualPath("Dossier1/Dossier2/Dossier3"), "add", false));
        ASSERT(!p.getRights(id, id_object).size());
        ASSERT(p.add(id, d1.getIdFromVirtualPath("Dossier1/Dossier2/Dossier3"), "write", true));
        ASSERT(p.getRights(id, id_object).contains("write"));
        ASSERT(p.add(id, c.getId(), "read"));
        ASSERT(p.getRights(id, c.getId()).contains("read"));
        // Advanced tests on permissions
        Configurations::instance()->set("permissions/activate", "false");
        ASSERT(p.isAllowed(id, d1.getIdFromVirtualPath("Dossier1/Dossier2"), "read"));
        ASSERT(p.isAllowed(id, d1.getIdFromVirtualPath("Dossier1/Dossier2/Dossier3"), "read"));
        ASSERT(p.isAllowed(id, f.getIdFromVirtualPath("Dossier1/Dossier2/Dossier3/Fichier1"), "read"));
        ASSERT(p.isAllowed(id, f.getIdFromVirtualPath("Dossier1/Dossier2/Dossier3/Fichier2"), "read"));
        ASSERT(p.isAllowed(id, d1.getIdFromVirtualPath("Dossier1/Dossier2/Dossier4"), "modify"));
        ASSERT(p.isAllowed(id, f.getIdFromVirtualPath("Dossier1/Dossier2/Fichier4"), "delete"));
        Configurations::instance()->set("permissions/activate", "true");
        ASSERT(f.setIdFromVirtualPath("Dossier1/Dossier2/Dossier3/Fichier2"));
        ASSERT(!f.isAllowed(a.getId(), "read"));
        ASSERT(f.setIdAccount(a.getId()));
        ASSERT(f.isAllowed(a.getId(), "read"));
        ASSERT(f.setIdAccount());
        ASSERT(!f.isAllowed(a.getId(), "read"));
        ASSERT(d1.setIdFromVirtualPath("Dossier1/Dossier2/Dossier3"));
        ASSERT(d1.setIdAccount(a.getId()));
        ASSERT(f.isAllowed(a.getId(), "read"));
        Configurations::instance()->set("permissions/ownerInheritance", "false");
        ASSERT(!f.isAllowed(a.getId(), "read"));
        Configurations::instance()->set("permissions/ownerInheritance", "true");
        ASSERT(d1.setIdAccount());
        ASSERT(f.setIdFromVirtualPath("Dossier1/Dossier2/Fichier4"));
        ASSERT(d2.setIdFromVirtualPath("Dossier1/Dossier2/Dossier4"));
        ASSERT(d1.setIdFromVirtualPath("Dossier1/Dossier2"));
        ASSERT(f.isAllowed(a.getId(), "read"));
        ASSERT(d2.isAllowed(a.getId(), "read"));
        Configurations::instance()->set("permissions/inheritance", "false");
        ASSERT(!f.isAllowed(a.getId(), "read"));
        ASSERT(!d2.isAllowed(a.getId(), "read"));
        ASSERT(d1.isAllowed(a.getId(), "read"));
        ASSERT(d1.setIdFromVirtualPath("Dossier1"));
        ASSERT(d2.setIdFromVirtualPath("Dossier6"));
        ASSERT(!d1.isAllowed(a.getId(), "read"));
        ASSERT(d2.isAllowed(a.getId(), "read"));
        Configurations::instance()->set("permissions/default", "true");
        ASSERT(d1.isAllowed(a.getId(), "read"));
        ASSERT(d2.isAllowed(a.getId(), "read"));
        ASSERT(p.setId(a.getId(), d2.getId(), "read"));
        ASSERT(p.isGranted(false));
        ASSERT(!d2.isAllowed(a.getId(), "read"));
        ASSERT(f.setIdFromVirtualPath("Dossier7/Fichier9"));
        ASSERT(f.isAllowed(a.getId(), "read"));
        Configurations::instance()->set("permissions/default", "false");
        ASSERT(!f.isAllowed(a.getId(), "read"));
        Configurations::instance()->set("permissions/inheritance", "true");
        ASSERT(p.add(a.getId(), "", "read", false));
        ASSERT(!d1.isAllowed(a.getId(), "read"));
        ASSERT(p.isGranted(true));
        ASSERT(d1.isAllowed(a.getId(), "read"));
        ASSERT(f.setIdFromVirtualPath("Dossier7/Dossier8/Fichier8"));
        ASSERT(d1.setIdFromVirtualPath("Dossier7/Dossier8"));
        ASSERT(p.setId(a.getId(), d1.getId(), "read"));
        ASSERT(p.setRight("modify"));
        Configurations::instance()->set("permissions/inheritance", "false");
        ASSERT(!f.isAllowed(a.getId(), "read"));
        ASSERT(!f.isAllowed(a.getId(), "modify"));
        Configurations::instance()->set("permissions/inheritance", "true");
        ASSERT(f.isAllowed(a.getId(), "read"));
        ASSERT(f.isAllowed(a.getId(), "modify"));
        ASSERT(!f.isAllowed(a.getId(), "delete"));
        ASSERT(!f.isAllowed(a.getId(), "add"));
        ASSERT(p.add(a.getId(), d1.getId(), "read", false));
        ASSERT(!f.isAllowed(a.getId(), "read"));
        ASSERT(f.isAllowed(a.getId(), "modify"));
        ASSERT(p.add(a.getId(), f.getId(), "read", true));
        ASSERT(p.add(a.getId(), f.getId(), "delete", false));
        ASSERT(!f.isAllowed(a.getId(), "modify"));
        ASSERT(!f.isAllowed(a.getId(), "delete"));
        ASSERT(f.isAllowed(a.getId(), "read"));
        ASSERT(p.setIdObject(d1.getId()));
        ASSERT(p.isGranted(true));
        ASSERT(f.isAllowed(a.getId(), "modify"));
        ASSERT(f.isAllowed(a.getId(), "delete"));
        ASSERT(f.isAllowed(a.getId(), "read"));
        ASSERT(!f.isAllowed(a.getId(), "add"));
        ASSERT(p.add(a.getId(), f.getId(), "", false));
        ASSERT(!f.isAllowed(a.getId(), "modify"));
        ASSERT(!f.isAllowed(a.getId(), "delete"));
        ASSERT(f.isAllowed(a.getId(), "read"));
        ASSERT(g1.add("g1"));
        ASSERT(g2.add("g2", g1.getId()));
        ASSERT(a.addGroup(g1.getId()));
        ASSERT(a.addGroup(g2.getId()));
        ASSERT(f.setIdFromVirtualPath("Dossier1/Fichier5"));
        ASSERT(f.isAllowed(a.getId(), "read"));
        ASSERT(p.remove(p.getId(a.getId(), "", "read")));
        ASSERT(!f.isAllowed(a.getId(), "read"));
        ASSERT(p.add(g1.getId(), f.getId(), "read", false));
        ASSERT(p.add(g2.getId(), f.getId(), "read", false));
        ASSERT(p.add(a.getId(), f.getId(), "read", true));
        ASSERT(f.isAllowed(a.getId(), "read"));
        ASSERT(!f.isAllowed(g1.getId(), "read"));
        ASSERT(!f.isAllowed(g2.getId(), "read"));
        ASSERT(p.remove());
        ASSERT(p.add("", f.getId(), "read", true));
        ASSERT(!f.isAllowed(a.getId(), "read"));
        ASSERT(p.setId(g2.getId(), f.getId(), "read"));
        ASSERT(p.isGranted(true));
        ASSERT(f.isAllowed(a.getId(), "read"));
        ASSERT(p.remove());
        ASSERT(!f.isAllowed(a.getId(), "read"));
        ASSERT(p.remove(p.getId(g1.getId(), f.getId(), "read")));
        ASSERT(f.isAllowed(a.getId(), "read"));
        ASSERT(d1.remove(d1.getIdFromVirtualPath("Dossier1")));
        ASSERT(d1.remove(d1.getIdFromVirtualPath("Dossier6")));
        ASSERT(d1.remove(d1.getIdFromVirtualPath("Dossier7")));
        ASSERT(a.remove());
        ASSERT(g1.remove());
    }
    catch (Properties properties)
    {
        Log::instance()->error("Unit test failed", properties, "TablePermissions", "unitTests");
        return (false);
    }
    Log::instance()->debug("Unit tests successful!", "TablePermissions", "unitTests");
    return (true);
}
