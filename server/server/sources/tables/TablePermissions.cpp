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
    this->tableId = LightBird::ITable::Permissions;
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
    LightBird::ITableAccounts   *account = NULL;
    LightBird::ITableFiles      *file = NULL;
    LightBird::ITableDirectories *directory = NULL;
    LightBird::ITableCollections *collection = NULL;
    TableGroups                 group;
    bool                        inheritance = false;
    bool                        ownerInheritance = false;
    bool                        checked = false;
    int                         granted = 0;
    QString                     id;
    TableDirectories            dir;
    TableCollections            col;
    QStringList                 accessors;
    QList<QStringList>          groups;

    // If the permissions system is not activated, we don't need to check the rights
    if (Configurations::instance()->get("permissions/activate") != "true")
        return (true);
    if (Configurations::instance()->get("permissions/ownerInheritance") == "true")
        ownerInheritance = true;
    if (Configurations::instance()->get("permissions/inheritance") == "true")
        inheritance = true;
    QSharedPointer<LightBird::ITableAccessors> accessor(dynamic_cast<LightBird::ITableAccessors *>(Database::instance()->getTable(LightBird::ITable::Accessor, id_accessor)));
    if (accessor.isNull())
        return (false);
    if (accessor->isTable(LightBird::ITable::Accounts))
        account = dynamic_cast<LightBird::ITableAccounts *>(accessor.data());
    // If the accessor is an account, and the account is administrator, he has all the rights on all the objects
    if (account && account->isAdministrator())
        return (true);
    QSharedPointer<LightBird::ITableObjects> object(dynamic_cast<LightBird::ITableObjects *>(Database::instance()->getTable(LightBird::ITable::Object, id_object)));
    if (object.isNull())
        return (false);
    // If the accessor is an account, and the account is the owner of the object, he has all the rights on it
    if (account && account->getId() == object->getIdAccount())
        return (true);
    // Get all the accessors concerned by the permission
    if (account)
        accessors = account->getGroups();
    else if (group.setId(accessor->getId()) && !(id = group.getIdGroup()).isEmpty())
        accessors.push_back(id);
    // Get all the parent groups of the accessor
    if (!accessors.isEmpty())
    {
        groups.push_back(accessors);
        while (true)
        {
            QStringListIterator it(groups.back());
            QStringList parents;
            while (it.hasNext())
            {
                // A parent has been found
                if (group.setId(it.peekNext()) && !(id = group.getIdGroup()).isEmpty())
                {
                    parents.push_back(id);
                    accessors.push_back(id);
                }
                it.next();
            }
            // No parent remaining. We are at the root of the tree.
            if (parents.isEmpty())
                break;
            parents.removeDuplicates();
            groups.push_back(parents);
        }
        accessors.removeDuplicates();
        // If groupInheritance is disables, we don't need to keep the groups hierarchy
        if (Configurations::instance()->get("permissions/groupInheritance") != "true")
            groups.clear();
    }
    accessors.push_front(accessor->getId());
    // Get the id of the first object in the hierarchy
    if (object->isTable(LightBird::ITable::Files))
    {
        // Check the permission of the file
        file = dynamic_cast<LightBird::ITableFiles *>(object.data());
        if ((granted = this->_idAllowed(accessors, groups, file->getId(), right)) == 2)
            return (true);
        id = file->getIdDirectory();
        checked = true;
    }
    if (object->isTable(LightBird::ITable::Directories))
    {
        directory = dynamic_cast<LightBird::ITableDirectories *>(object.data());
        id = directory->getId();
    }
    if (object->isTable(LightBird::ITable::Collections))
    {
        collection = dynamic_cast<LightBird::ITableCollections *>(object.data());
        id = collection->getId();
    }
    // Run through the objects hierarchy to find the permissions
    if (file || directory)
        while (!id.isEmpty())
        {
            if (!granted && (inheritance || !checked) && (granted = this->_idAllowed(accessors, groups, id, right)) == 2)
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
            if (!granted && (inheritance || !checked) && (granted = this->_idAllowed(accessors, groups, id, right)) == 2)
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
    if (!granted && inheritance && (granted = this->_idAllowed(accessors, groups, "", right)) == 2)
        return (true);
    // If there is no permissions for the accessor on the object, the default is applied
    if (!granted && Configurations::instance()->get("permissions/default") == "true")
        return (true);
    return (false);
}

bool    TablePermissions::getRights(const QString &id_accessor, const QString &id_object, QStringList &allowed, QStringList &denied)
{
    LightBird::ITableAccounts   *account = NULL;
    LightBird::ITableFiles      *file = NULL;
    LightBird::ITableDirectories *directory = NULL;
    LightBird::ITableCollections *collection = NULL;
    TableGroups                 group;
    bool                        inheritance = false;
    bool                        ownerInheritance = false;
    bool                        checked = false;
    QString                     id;
    TableDirectories            dir;
    TableCollections            col;
    QStringList                 accessors;
    QList<QStringList>          groups;

    allowed.clear();
    denied.clear();
    // If the permissions system is not activated, all the rights are granted
    if (Configurations::instance()->get("permissions/activate") != "true")
    {
        allowed.push_back("");
        return (true);
    }
    if (Configurations::instance()->get("permissions/ownerInheritance") == "true")
        ownerInheritance = true;
    if (Configurations::instance()->get("permissions/inheritance") == "true")
        inheritance = true;
    QSharedPointer<LightBird::ITableAccessors> accessor(dynamic_cast<LightBird::ITableAccessors *>(Database::instance()->getTable(LightBird::ITable::Accessor, id_accessor)));
    if (accessor.isNull())
        return (false);
    if (accessor->isTable(LightBird::ITable::Accounts))
        account = dynamic_cast<LightBird::ITableAccounts *>(accessor.data());
    // If the accessor is an account, and the account is administrator, he has all the rights on all the objects
    if (account && account->isAdministrator())
    {
        allowed.push_back("");
        return (true);
    }
    QSharedPointer<LightBird::ITableObjects> object(dynamic_cast<LightBird::ITableObjects *>(Database::instance()->getTable(LightBird::ITable::Object, id_object)));
    if (object.isNull())
        return (false);
    // If the accessor is an account, and the account is the owner of the object, he has all the rights on it
    if (account && account->getId() == object->getIdAccount())
    {
        allowed.push_back("");
        return (true);
    }
    // Get all the accessors concerned by the permission
    if (account)
        accessors = account->getGroups();
    else if (group.setId(accessor->getId()) && !(id = group.getIdGroup()).isEmpty())
        accessors.push_back(id);
    // Get all the parent groups of the accessor
    if (!accessors.isEmpty())
    {
        groups.push_back(accessors);
        while (true)
        {
            QStringListIterator it(groups.back());
            QStringList parents;
            while (it.hasNext())
            {
                // A parent has been found
                if (group.setId(it.peekNext()) && !(id = group.getIdGroup()).isEmpty())
                {
                    parents.push_back(id);
                    accessors.push_back(id);
                }
                it.next();
            }
            // No parent remaining. We are at the root of the tree.
            if (parents.isEmpty())
                break;
            parents.removeDuplicates();
            groups.push_back(parents);
        }
        accessors.removeDuplicates();
        // If groupInheritance is disables, we don't need to keep the groups hierarchy
        if (Configurations::instance()->get("permissions/groupInheritance") != "true")
            groups.clear();
    }
    accessors.push_front(accessor->getId());
    // Get the id of the first object in the hierarchy
    if (object->isTable(LightBird::ITable::Files))
    {
        file = dynamic_cast<LightBird::ITableFiles *>(object.data());
        id = file->getIdDirectory();
        this->_getRights(accessors, groups, file->getId(), allowed, denied);
        checked = true;
    }
    if (object->isTable(LightBird::ITable::Directories))
    {
        directory = dynamic_cast<LightBird::ITableDirectories *>(object.data());
        id = directory->getId();
    }
    if (object->isTable(LightBird::ITable::Collections))
    {
        collection = dynamic_cast<LightBird::ITableCollections *>(object.data());
        id = collection->getId();
    }
    // Run through the objects hierarchy to find the permissions
    if (file || directory)
        while (!id.isEmpty())
        {
            if (inheritance || !checked)
                this->_getRights(accessors, groups, id, allowed, denied);
            dir.setId(id);
            if (account && ownerInheritance && account->getId() == dir.getIdAccount())
            {
                allowed.clear();
                denied.clear();
                allowed.push_back("");
                return (true);
            }
            id = dir.getIdDirectory();
            checked = true;
        }
    else if (collection)
        while (!id.isEmpty())
        {
            if (inheritance || !checked)
                this->_getRights(accessors, groups, id, allowed, denied);
            col.setId(id);
            if (account && ownerInheritance && account->getId() == col.getIdAccount())
            {
                allowed.clear();
                denied.clear();
                allowed.push_back("");
                return (true);
            }
            id = col.getIdCollection();
            checked = true;
        }
    // Check if there is a permission at the root of the server
    if (inheritance)
        this->_getRights(accessors, groups, "", allowed, denied);
    // If there is no global permission, the default is applied
    if (!allowed.contains("") && !denied.contains(""))
    {
        if (Configurations::instance()->get("permissions/default") == "true")
        {
            allowed.clear();
            allowed.push_back("");
        }
        else
        {
            denied.clear();
            denied.push_back("");
        }
    }
    return (true);
}

unsigned    TablePermissions::_idAllowed(const QStringList &accessors, const QList<QStringList> &groups, const QString &idObject, const QString &right)
{
    QSqlQuery                           query;
    QVector<QMap<QString, QVariant> >   result;
    QStringList                         rights;
    unsigned                            granted;

    rights.push_back(right);
    // Someone who can modify can read
    if (rights.contains("read"))
        rights.push_back("modify");
    // Someone who can delete can modify
    if (rights.contains("modify"))
        rights.push_back("delete");
    query.prepare(Database::instance()->getQuery("TablePermissions", "_idAllowed")
                  .replace(":accessors", "'" + accessors.join("','") + "'")
                  .replace(":rights", "'" + rights.join("','") + "'"));
    query.bindValue(":id_object", idObject);
    if (!Database::instance()->query(query, result) || result.size() <= 0)
        return (0);
    // Check the right of the accessor
    if ((granted = this->_checkRights(result, QStringList() << accessors.front(), right)))
        return (granted);
    // If the permission is allowed by at least one group, it is granted
    if (groups.isEmpty() && (granted = this->_checkRights(result, accessors, right)))
        return (granted);
    // Check the group inheritance
    else
    {
        QListIterator<QStringList> it(groups);
        while (it.hasNext() && !granted)
        {
            if ((granted = this->_checkRights(result, it.peekNext(), right)))
                return (granted);
            it.next();
        }
    }
    return (this->_checkRights(result, QStringList() << "", right));
}

unsigned        TablePermissions::_checkRights(const QVector<QMap<QString, QVariant> > &rights, const QStringList &accessors, const QString &right)
{
    int         i;
    int         s;
    unsigned    granted = 0;
    QList<int>  index;

    // Get the indexes of the accessors we are interested in
    for (i = 0, s = rights.size(); i < s; ++i)
        if (accessors.contains(rights[i]["id_accessor"].toString()))
            index.push_back(i);
    // Check the right directly
    QListIterator<int> it(index);
    while (it.hasNext())
    {
        if (rights[it.peekNext()]["right"] == right
            && (granted = (rights[it.peekNext()]["granted"].toBool() + 1)) == 2)
            return (granted);
        it.next();
    }
    if (granted)
        return (granted);
    it.toFront();
    // Check the read and modify cases
    if (right == "read" || right == "modify")
    {
        while (it.hasNext())
        {
            if ((rights[it.peekNext()]["right"] == "delete" || rights[it.peekNext()]["right"] == "modify")
                && (granted = (rights[it.peekNext()]["granted"].toBool() + 1)) == 2)
                return (granted);
            it.next();
        }
        if (granted)
            return (granted);
        it.toFront();
    }
    // Check the global right
    while (it.hasNext())
    {
        if (rights[it.peekNext()]["right"].toString().isEmpty()
            && (granted = (rights[it.peekNext()]["granted"].toBool() + 1)) == 2)
            return (granted);
        it.next();
    }
    return (granted);
}

void    TablePermissions::_getRights(const QStringList &accessors, const QList<QStringList> &groups, const QString &idObject, QStringList &allowed, QStringList &denied)
{
    QSqlQuery                           query;
    QVector<QMap<QString, QVariant> >   result;
    int                                 i;
    int                                 s;
    QStringList                         allowedTmp;
    QStringList                         deniedTmp;
    QString                             right;

    query.prepare(Database::instance()->getQuery("TablePermissions", "_getRights")
                  .replace(":accessors", "'" + accessors.join("','") + "'"));
    query.bindValue(":id_object", idObject);
    if (!Database::instance()->query(query, result) || result.size() <= 0)
        return ;
    // Check the right of the accessor
    for (i = 0, s = result.size(); i < s; ++i)
        if (result[i]["id_accessor"] == accessors.front())
        {
            if (result[i]["granted"].toBool())
                allowedTmp.push_back(result[i]["right"].toString());
            else
                deniedTmp.push_back(result[i]["right"].toString());
        }
    this->_mergeRights(allowedTmp, deniedTmp, allowed, denied);
    // Get the right of the groups of the accessor, if goupInheritance is disabled
    if (groups.isEmpty())
    {
        for (i = 0, s = result.size(); i < s; ++i)
            if (!result[i]["id_accessor"].toString().isEmpty())
            {
                right = result[i]["right"].toString();
                // If at least one group allowed the right, it is granted
                if (result[i]["granted"].toBool())
                {
                    if (deniedTmp.contains(right))
                        deniedTmp.removeAll(right);
                    if (!allowedTmp.contains(right))
                        allowedTmp.push_back(right);
                }
                else if (!allowedTmp.contains(right) && !deniedTmp.contains(right))
                    deniedTmp.push_back(right);
            }
        this->_mergeRights(allowedTmp, deniedTmp, allowed, denied);
    }
    // Get the rights using the group inheritance
    else
    {
        // Run through the hierarchy
        QListIterator<QStringList> it(groups);
        while (it.hasNext())
        {
            for (i = 0, s = result.size(); i < s; ++i)
            {
                // The right is in the current hierarchy level
                if (it.peekNext().contains(result[i]["id_accessor"].toString()))
                {
                    right = result[i]["right"].toString();
                    // If at least one group allowed the right in the current level, it is granted
                    if (result[i]["granted"].toBool())
                    {
                        if (deniedTmp.contains(right))
                            deniedTmp.removeAll(right);
                        if (!allowedTmp.contains(right))
                            allowedTmp.push_back(right);
                    }
                    else if (!allowedTmp.contains(right) && !deniedTmp.contains(right))
                        deniedTmp.push_back(right);
                }
            }
            this->_mergeRights(allowedTmp, deniedTmp, allowed, denied);
            it.next();
        }
    }
    // Get the rights of all the accessors
    for (i = 0, s = result.size(); i < s; ++i)
        if (result[i]["id_accessor"].toString().isEmpty())
        {
            right = result[i]["right"].toString();
            if (!allowedTmp.contains(right) && !deniedTmp.contains(right))
            {
                if (result[i]["granted"].toBool())
                    allowed.push_back(right);
                else
                    denied.push_back(right);
            }
        }
}

void    TablePermissions::_mergeRights(QStringList &allowedSrc, QStringList &deniedSrc, QStringList &allowedDest, QStringList &deniedDest)
{
    // Add the new rights to the allowed list
    if (!allowedDest.contains("") && !deniedDest.contains(""))
    {
        QStringListIterator it(allowedSrc);
        while (it.hasNext())
        {
            if (!allowedDest.contains(it.peekNext()) && !deniedDest.contains(it.peekNext()))
                allowedDest.push_back(it.peekNext());
            it.next();
        }
        // All the rights has been allowed
        if (allowedDest.contains(""))
        {
            allowedDest.clear();
            allowedDest.push_back("");
        }
    }
    // Add the new rights to the denied list
    if (!allowedDest.contains("") && !deniedDest.contains(""))
    {
        QStringListIterator it(deniedSrc);
        while (it.hasNext())
        {
            if (!allowedDest.contains(it.peekNext()) && !deniedDest.contains(it.peekNext()))
                deniedDest.push_back(it.peekNext());
            it.next();
        }
        // All the rights has been denied
        if (deniedDest.contains(""))
        {
            deniedDest.clear();
            deniedDest.push_back("");
        }
    }
    // A delete right implies a modify, and a modify implies a read
    if (allowedDest.contains("delete") && !allowedDest.contains("modify") && !deniedDest.contains("modify") && !deniedDest.contains(""))
        allowedDest.push_back("modify");
    if (allowedDest.contains("modify") && !allowedDest.contains("read") && !deniedDest.contains("read") && !deniedDest.contains(""))
        allowedDest.push_back("read");
    if (deniedDest.contains("delete") && !deniedDest.contains("modify") && !allowedDest.contains("modify") && !allowedDest.contains(""))
        deniedDest.push_back("modify");
    if (deniedDest.contains("modify") && !deniedDest.contains("read") && !allowedDest.contains("read") && !allowedDest.contains(""))
        deniedDest.push_back("read");
    allowedSrc.clear();
    deniedSrc.clear();
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
    QString             id_object;
    QStringList         allowed;
    QStringList         denied;

    Log::instance()->debug("Running unit tests...", "TablePermissions", "unitTests");
    query.prepare("DELETE FROM directories WHERE name IN('d', 'Directory1', 'Directory6', 'Directory7')");
    Database::instance()->query(query);
    query.prepare("DELETE FROM accounts WHERE name IN('a')");
    Database::instance()->query(query);
    query.prepare("DELETE FROM groups WHERE name IN('g1', 'g2')");
    Database::instance()->query(query);
    query.prepare("DELETE FROM permissions WHERE id_accessor='' AND id_object=''");
    Database::instance()->query(query);
    Configurations::instance()->set("permissions/activate", "true");
    Configurations::instance()->set("permissions/default", "false");
    Configurations::instance()->set("permissions/inheritance", "true");
    Configurations::instance()->set("permissions/ownerInheritance", "true");
    Configurations::instance()->set("permissions/groupInheritance", "true");
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
        // Create a file/directory hierarchy
        ASSERT(d1.add("Directory6"));
        ASSERT(p.add(id, d1.getId(), "read", true));
        ASSERT(f.add("File7", "File7", "", d1.getId()));
        ASSERT(d1.add("Directory7"));
        ASSERT(f.add("File9", "File9", "", d1.getId()));
        ASSERT(d1.add("Directory8", d1.getId()));
        ASSERT(p.add(id, d1.getId(), "read", true));
        ASSERT(f.add("File8", "File8", "", d1.getId()));
        ASSERT(d1.add("Directory1"));
        ASSERT(d2.add("Directory5", d1.getId()));
        ASSERT(d2.add("Directory2", d1.getId()));
        ASSERT(p.add(id, d2.getId(), "read", true));
        ASSERT(f.add("File5", "File5", "", d1.getId()));
        ASSERT(f.add("File6", "File6", "", d1.getId()));
        ASSERT(p.add(id, f.getId(), "read", true));
        ASSERT(d1.add("Directory3", d2.getId()));
        ASSERT(p.add(id, d1.getId(), "read", false));
        ASSERT(f.add("File1", "File1", "", d1.getId()));
        ASSERT(f.add("File2", "File2", "", d1.getId()));
        ASSERT(p.add(id, f.getId(), "read", false));
        ASSERT(d1.add("Directory4", d2.getId()));
        ASSERT(f.add("File3", "File3", "", d1.getId()));
        ASSERT(p.add(id, f.getId(), "read", false));
        ASSERT(f.add("File4", "File4", "", d2.getId()));
        // Some simple tests
        ASSERT(!p.isAllowed(id, d1.getIdFromVirtualPath("Directory1"), "read"));
        ASSERT(p.isAllowed(id, d1.getIdFromVirtualPath("Directory1/Directory2"), "read"));
        ASSERT(!p.isAllowed(id, d1.getIdFromVirtualPath("Directory1/Directory2/Directory3"), "read"));
        ASSERT(!p.isAllowed(id, f.getIdFromVirtualPath("Directory1/Directory2/Directory3/File1"), "read"));
        ASSERT(!p.isAllowed(id, f.getIdFromVirtualPath("Directory1/Directory2/Directory3/File2"), "read"));
        ASSERT(p.isAllowed(id, d1.getIdFromVirtualPath("Directory1/Directory2/Directory4"), "read"));
        ASSERT(p.isAllowed(id, f.getIdFromVirtualPath("Directory1/Directory2/File4"), "read"));
        ASSERT(!p.isAllowed(id, d1.getIdFromVirtualPath("Directory1/Directory5"), "read"));
        ASSERT(!p.isAllowed(id, f.getIdFromVirtualPath("Directory1/File5"), "read"));
        ASSERT(p.isAllowed(id, f.getIdFromVirtualPath("Directory1/File6"), "read"));
        ASSERT(p.isAllowed(id, d1.getIdFromVirtualPath("Directory6"), "read"));
        ASSERT(p.isAllowed(id, f.getIdFromVirtualPath("Directory6/File7"), "read"));
        ASSERT(!p.isAllowed(id, d1.getIdFromVirtualPath("Directory7"), "read"));
        ASSERT(p.isAllowed(id, d1.getIdFromVirtualPath("Directory7/Directory8"), "read"));
        ASSERT(p.isAllowed(id, f.getIdFromVirtualPath("Directory7/Directory8/File8"), "read"));
        ASSERT(!p.isAllowed(id, f.getIdFromVirtualPath("Directory7/File9"), "read"));
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
        id_object = f.getIdFromVirtualPath("Directory1/Directory2/File4");
        // Test getRights
        ASSERT(!p.getRights("", id_object, allowed, denied));
        ASSERT(!p.getRights(id, "", allowed, denied));
        ASSERT(!p.getRights("42", id_object, allowed, denied));
        ASSERT(p.getRights(id, id_object, allowed, denied) && allowed.contains("read"));
        ASSERT(denied.contains(""));
        ASSERT(p.add(id, id_object, "write"));
        ASSERT(p.getRights(id, id_object, allowed, denied));
        ASSERT(allowed.contains("read"));
        ASSERT(allowed.contains("write"));
        ASSERT(denied.contains(""));
        ASSERT(p.getRights(id, d1.getIdFromVirtualPath("Directory1/Directory5"), allowed, denied) && allowed.isEmpty() && denied.contains(""));
        ASSERT(p.getRights(id, d1.getIdFromVirtualPath("Directory7/Directory8"), allowed, denied) && allowed.contains("read"));
        id_object = f.getIdFromVirtualPath("Directory1/Directory2/Directory3/File2");
        ASSERT(p.getRights(id, id_object, allowed, denied) && allowed.isEmpty() && denied.contains(""));
        ASSERT(p.add(id, d1.getIdFromVirtualPath("Directory1/Directory2"), "add"));
        ASSERT(p.getRights(id, id_object, allowed, denied) && allowed.contains("add"));
        ASSERT(p.add(id, d1.getIdFromVirtualPath("Directory1/Directory2/Directory3"), "add", false));
        ASSERT(p.getRights(id, id_object, allowed, denied) && allowed.isEmpty());
        ASSERT(p.add(id, d1.getIdFromVirtualPath("Directory1/Directory2/Directory3"), "write", true));
        ASSERT(p.getRights(id, id_object, allowed, denied) && allowed.contains("write"));
        ASSERT(p.add(id, c.getId(), "read"));
        ASSERT(p.getRights(id, c.getId(), allowed, denied) && allowed.contains("") && denied.isEmpty());
        // Advanced tests on permissions
        Configurations::instance()->set("permissions/activate", "false");
        ASSERT(p.isAllowed(id, d1.getIdFromVirtualPath("Directory1/Directory2"), "read"));
        ASSERT(p.isAllowed(id, d1.getIdFromVirtualPath("Directory1/Directory2/Directory3"), "read"));
        ASSERT(p.isAllowed(id, f.getIdFromVirtualPath("Directory1/Directory2/Directory3/File1"), "read"));
        ASSERT(p.isAllowed(id, f.getIdFromVirtualPath("Directory1/Directory2/Directory3/File2"), "read"));
        ASSERT(p.isAllowed(id, d1.getIdFromVirtualPath("Directory1/Directory2/Directory4"), "modify"));
        ASSERT(p.isAllowed(id, f.getIdFromVirtualPath("Directory1/Directory2/File4"), "delete"));
        Configurations::instance()->set("permissions/activate", "true");
        ASSERT(f.setIdFromVirtualPath("Directory1/Directory2/Directory3/File2"));
        ASSERT(!f.isAllowed(a.getId(), "read"));
        ASSERT(f.setIdAccount(a.getId()));
        ASSERT(f.isAllowed(a.getId(), "read"));
        ASSERT(f.setIdAccount());
        ASSERT(!f.isAllowed(a.getId(), "read"));
        ASSERT(d1.setIdFromVirtualPath("Directory1/Directory2/Directory3"));
        ASSERT(d1.setIdAccount(a.getId()));
        ASSERT(f.isAllowed(a.getId(), "read"));
        Configurations::instance()->set("permissions/ownerInheritance", "false");
        ASSERT(!f.isAllowed(a.getId(), "read"));
        Configurations::instance()->set("permissions/ownerInheritance", "true");
        ASSERT(d1.setIdAccount());
        ASSERT(f.setIdFromVirtualPath("Directory1/Directory2/File4"));
        ASSERT(d2.setIdFromVirtualPath("Directory1/Directory2/Directory4"));
        ASSERT(d1.setIdFromVirtualPath("Directory1/Directory2"));
        ASSERT(f.isAllowed(a.getId(), "read"));
        ASSERT(d2.isAllowed(a.getId(), "read"));
        Configurations::instance()->set("permissions/inheritance", "false");
        ASSERT(!f.isAllowed(a.getId(), "read"));
        ASSERT(!d2.isAllowed(a.getId(), "read"));
        ASSERT(d1.isAllowed(a.getId(), "read"));
        ASSERT(d1.setIdFromVirtualPath("Directory1"));
        ASSERT(d2.setIdFromVirtualPath("Directory6"));
        ASSERT(!d1.isAllowed(a.getId(), "read"));
        ASSERT(d2.isAllowed(a.getId(), "read"));
        Configurations::instance()->set("permissions/default", "true");
        ASSERT(d1.isAllowed(a.getId(), "read"));
        ASSERT(d2.isAllowed(a.getId(), "read"));
        ASSERT(p.setId(a.getId(), d2.getId(), "read"));
        ASSERT(p.isGranted(false));
        ASSERT(!d2.isAllowed(a.getId(), "read"));
        ASSERT(f.setIdFromVirtualPath("Directory7/File9"));
        ASSERT(f.isAllowed(a.getId(), "read"));
        Configurations::instance()->set("permissions/default", "false");
        ASSERT(!f.isAllowed(a.getId(), "read"));
        Configurations::instance()->set("permissions/inheritance", "true");
        ASSERT(p.add(a.getId(), "", "read", false));
        ASSERT(!d1.isAllowed(a.getId(), "read"));
        ASSERT(p.isGranted(true));
        ASSERT(d1.isAllowed(a.getId(), "read"));
        ASSERT(f.setIdFromVirtualPath("Directory7/Directory8/File8"));
        ASSERT(d1.setIdFromVirtualPath("Directory7/Directory8"));
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
        ASSERT(f.setIdFromVirtualPath("Directory1/File5"));
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
        ASSERT(p.remove(p.getId("", f.getId(), "read")));
        // Check the rights conflict
        ASSERT(!f.isAllowed(a.getId(), "read"));
        ASSERT(p.add(a.getId(), f.getId(), "modify", true));
        ASSERT(f.isAllowed(a.getId(), "read"));
        ASSERT(p.add(a.getId(), f.getId(), "delete", false));
        ASSERT(f.isAllowed(a.getId(), "read"));
        ASSERT(!f.isAllowed(a.getId(), "delete"));
        ASSERT(f.isAllowed(a.getId(), "modify"));
        ASSERT(p.setId(a.getId(), f.getId(), "modify"));
        ASSERT(p.isGranted(false));
        ASSERT(!f.isAllowed(a.getId(), "modify"));
        ASSERT(!f.isAllowed(a.getId(), "read"));
        ASSERT(!f.isAllowed(a.getId(), "add"));
        ASSERT(p.add(a.getId(), f.getId(), "", true));
        ASSERT(f.isAllowed(a.getId(), "add"));
        ASSERT(p.add(a.getId(), f.getId(), "add", true));
        ASSERT(!f.isAllowed(a.getId(), "read"));
        ASSERT(f.isAllowed(a.getId(), "add"));
        ASSERT(p.setId(a.getId(), f.getId(), "add"));
        ASSERT(p.isGranted(false));
        ASSERT(!f.isAllowed(a.getId(), "add"));
        ASSERT(p.remove());
        ASSERT(f.isAllowed(a.getId(), "add"));
        ASSERT(p.setId(a.getId(), f.getId(), "delete"));
        ASSERT(p.isGranted(true));
        ASSERT(f.isAllowed(a.getId(), "delete"));
        ASSERT(f.isAllowed(a.getId(), "read"));
        ASSERT(p.remove(p.getId(a.getId(), f.getId(), "delete")));
        ASSERT(!f.isAllowed(a.getId(), "read"));
        ASSERT(p.remove(p.getId(a.getId(), f.getId(), "modify")));
        ASSERT(f.isAllowed(a.getId(), "read"));
        ASSERT(p.remove(p.getId(a.getId(), f.getId(), "")));
        ASSERT(!f.isAllowed(a.getId(), "read"));
        // Advanced tests on getRights
        ASSERT(a.getRights(f.getId(), allowed, denied));
        ASSERT(allowed.isEmpty() && denied.contains(""));
        ASSERT(p.add(a.getId(), f.getId(), "read", true));
        ASSERT(a.getRights(f.getId(), allowed, denied));
        ASSERT(allowed.contains("read") && denied.contains("") && allowed.size() == 1 && denied.size() == 1);
        ASSERT(d1.setIdFromVirtualPath("Directory1"));
        ASSERT(p.add(a.getId(), d1.getId(), "delete", true));
        ASSERT(f.isAllowed(a.getId(), "modify"));
        ASSERT(a.getRights(f.getId(), allowed, denied));
        ASSERT(allowed.contains("read") && allowed.contains("modify") && allowed.contains("delete") && denied.contains("") && allowed.size() == 3);
        Configurations::instance()->set("permissions/inheritance", "false");
        ASSERT(a.getRights(f.getId(), allowed, denied));
        ASSERT(allowed.contains("read") && denied.contains("") && allowed.size() == 1 && denied.size() == 1);
        Configurations::instance()->set("permissions/inheritance", "true");
        ASSERT(a.isAllowed(f.getId(), "delete"));
        ASSERT(p.add(a.getId(), f.getId(), "", false));
        ASSERT(!a.isAllowed(f.getId(), "delete"));
        ASSERT(a.getRights(f.getId(), allowed, denied));
        ASSERT(allowed.contains("read") && denied.contains("") && allowed.size() == 1 && denied.size() == 1);
        ASSERT(p.add("", "", "add", true));
        ASSERT(a.getRights(f.getId(), allowed, denied));
        ASSERT(allowed.contains("add") && allowed.contains("read") && denied.contains("") && allowed.size() == 2);
        ASSERT(p.remove(p.getId(a.getId(), d1.getId(), "delete")));
        ASSERT(p.remove(p.getId(a.getId(), f.getId(), "read")));
        ASSERT(p.remove(p.getId(a.getId(), f.getId(), "")));
        ASSERT(p.remove(p.getId("", "", "add")));
        Configurations::instance()->set("permissions/default", "false");
        // Check the permissions on the groups hierarchy
        ASSERT(g2.setIdGroup(""));
        ASSERT(g2.add("g3", g2.getIdFromName("g1").first()));
        ASSERT(g2.add("g4", g2.getIdFromName("g3").first()));
        ASSERT(g2.add("g5", g2.getIdFromName("g1").first()));
        ASSERT(g2.add("g6", g2.getIdFromName("g5").first()));
        ASSERT(g2.add("g7", g2.getIdFromName("g5").first()));
        ASSERT(g2.add("g8", g2.getIdFromName("g1").first()));
        ASSERT(g2.add("g9", g2.getIdFromName("g2").first()));
        ASSERT(g1.removeAccount(a.getId()));
        ASSERT(g2.setId(g2.getIdFromName("g2").first()));
        ASSERT(g2.removeAccount(a.getId()));
        ASSERT(!a.isAllowed(f.getId(), "read"));
        ASSERT(g2.setId(g2.getIdFromName("g6").first()));
        ASSERT(a.addGroup(g2.getIdFromName("g5").first()));
        ASSERT(a.addGroup(g2.getId()));
        ASSERT(p.add(g1.getId(), f.getId(), "read", true));
        ASSERT(a.isAllowed(f.getId(), "read"));
        ASSERT(p.add(g2.getId(), f.getId(), "read", false));
        ASSERT(!a.isAllowed(f.getId(), "read"));
        ASSERT(g1.isAllowed(f.getId(), "read"));
        ASSERT(g1.setId(g1.getIdFromName("g4").first()));
        ASSERT(g1.isAllowed(f.getId(), "read"));
        ASSERT(p.add(g1.getIdFromName("g3").first(), f.getId(), "read", false));
        ASSERT(!g1.isAllowed(f.getId(), "read"));
        ASSERT(g2.setId(g2.getIdFromName("g9").first()));
        ASSERT(!g2.isAllowed(f.getId(), "read"));
        Configurations::instance()->set("permissions/default", "true");
        ASSERT(g2.isAllowed(f.getId(), "read"));
        Configurations::instance()->set("permissions/groupInheritance", "false");
        ASSERT(a.isAllowed(f.getId(), "read"));
        ASSERT(g1.isAllowed(f.getId(), "read"));
        ASSERT(d1.remove(d1.getIdFromVirtualPath("Directory1")));
        ASSERT(d1.remove(d1.getIdFromVirtualPath("Directory6")));
        ASSERT(d1.remove(d1.getIdFromVirtualPath("Directory7")));
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
