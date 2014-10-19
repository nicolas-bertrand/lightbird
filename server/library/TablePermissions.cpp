#include "Library.h"
#include "LightBird.h"

using namespace LightBird;

TablePermissions::TablePermissions(const QString &id)
{
    this->tableName = "permissions";
    this->tableId = Table::Permissions;
    Table::setId(id);
}

TablePermissions::~TablePermissions()
{
}

TablePermissions::TablePermissions(const TablePermissions &table)
    : Table()
{
    *this = table;
}

TablePermissions &TablePermissions::operator=(const TablePermissions &table)
{
    Table::operator=(table);
    return (*this);
}

QString TablePermissions::getId(const QString &id_accessor, const QString &id_object, const QString &right) const
{
    QSqlQuery            query(Library::database().getDatabase());
    QVector<QVariantMap> result;

    query.prepare(Library::database().getQuery("TablePermissions", "getId"));
    query.bindValue(":id_accessor", id_accessor);
    query.bindValue(":id_object", id_object);
    query.bindValue(":right", right);
    if (Library::database().query(query, result) && result.size() > 0)
        return (result[0]["id"].toString());
    return ("");
}

bool    TablePermissions::setId(const QString &id_accessor, const QString &id_object, const QString &right)
{
    QString id;

    if ((id = this->getId(id_accessor, id_object, right)).isEmpty())
        return (false);
    this->id = id;
    return (true);
}

bool    TablePermissions::add(const QString &id_accessor, const QString &id_object, const QString &right, bool granted)
{
    QSqlQuery   query(Library::database().getDatabase());
    QString     id;

    id = createUuid();
    query.prepare(Library::database().getQuery("TablePermissions", "add"));
    query.bindValue(":id", id);
    query.bindValue(":id_accessor", id_accessor);
    query.bindValue(":id_object", id_object);
    query.bindValue(":right", right);
    query.bindValue(":granted", QString::number(granted));
    if (!Library::database().query(query) || query.numRowsAffected() == 0)
        return (false);
    this->id = id;
    return (true);
}

QString TablePermissions::getIdAccessor() const
{
    QSqlQuery            query(Library::database().getDatabase());
    QVector<QVariantMap> result;

    query.prepare(Library::database().getQuery("TablePermissions", "getIdAccessor"));
    query.bindValue(":id", this->id);
    if (Library::database().query(query, result) && result.size() > 0)
        return (result[0]["id_accessor"].toString());
    return ("");
}

bool    TablePermissions::setIdAccessor(const QString &id_accessor)
{
    QSqlQuery   query(Library::database().getDatabase());

    query.prepare(Library::database().getQuery("TablePermissions", "setIdAccessor"));
    query.bindValue(":id", this->id);
    query.bindValue(":id_accessor", id_accessor);
    return (Library::database().query(query));
}

QString TablePermissions::getIdObject() const
{
    QSqlQuery            query(Library::database().getDatabase());
    QVector<QVariantMap> result;

    query.prepare(Library::database().getQuery("TablePermissions", "getIdObject"));
    query.bindValue(":id", this->id);
    if (Library::database().query(query, result) && result.size() > 0)
        return (result[0]["id_object"].toString());
    return ("");
}

bool    TablePermissions::setIdObject(const QString &id_object)
{
    QSqlQuery   query(Library::database().getDatabase());

    query.prepare(Library::database().getQuery("TablePermissions", "setIdObject"));
    query.bindValue(":id", this->id);
    query.bindValue(":id_object", id_object);
    return (Library::database().query(query));
}

QString TablePermissions::getRight() const
{
    QSqlQuery            query(Library::database().getDatabase());
    QVector<QVariantMap> result;

    query.prepare(Library::database().getQuery("TablePermissions", "getRight"));
    query.bindValue(":id", this->id);
    if (Library::database().query(query, result) && result.size() > 0)
        return (result[0]["right"].toString());
    return ("");
}

bool    TablePermissions::setRight(const QString &right)
{
    QSqlQuery   query(Library::database().getDatabase());

    query.prepare(Library::database().getQuery("TablePermissions", "setRight"));
    query.bindValue(":id", this->id);
    query.bindValue(":right", right);
    return (Library::database().query(query));
}

bool    TablePermissions::isGranted() const
{
    QSqlQuery            query(Library::database().getDatabase());
    QVector<QVariantMap> result;

    query.prepare(Library::database().getQuery("TablePermissions", "getGranted"));
    query.bindValue(":id", this->id);
    if (Library::database().query(query, result) && result.size() > 0)
        return (result[0]["granted"].toBool());
    return (false);
}

bool    TablePermissions::isGranted(bool granted)
{
    QSqlQuery   query(Library::database().getDatabase());

    query.prepare(Library::database().getQuery("TablePermissions", "setGranted"));
    query.bindValue(":id", this->id);
    query.bindValue(":granted", QString::number(granted));
    return (Library::database().query(query));
}

bool    TablePermissions::isAllowed(const QString &id_accessor, const QString &id_object, const QString &right) const
{
    TableAccounts       *account = NULL;
    TableFiles          *file = NULL;
    TableDirectories    *directory = NULL;
    TableCollections    *collection = NULL;
    TableGroups         group;
    bool                inheritance;
    bool                ownerInheritance;
    bool                checked = false;
    int                 granted = 0;
    QString             id;
    TableDirectories    dir;
    TableCollections    col;
    QStringList         accessors;
    QList<QStringList>  groups;

    // If the permissions system is not activated, we don't need to check the rights
    if (!LightBird::c().permissions.activate)
        return (true);
    ownerInheritance = LightBird::c().permissions.ownerInheritance;
    inheritance = LightBird::c().permissions.inheritance;
    QSharedPointer<TableAccessors> accessor(dynamic_cast<TableAccessors *>(Library::database().getTable(Table::Accessor, id_accessor)));
    if (accessor.isNull())
        return (false);
    if (accessor->isTable(Table::Accounts))
        account = dynamic_cast<TableAccounts *>(accessor.data());
    // If the accessor is an account, and the account is administrator, he has all the rights on all the objects
    if (account && account->isAdministrator())
        return (true);
    QSharedPointer<TableObjects> object(dynamic_cast<TableObjects *>(Library::database().getTable(Table::Object, id_object)));
    // An empty id_object is the root directory
    if (!object && !id_object.isEmpty())
        return (false);
    // If the accessor is an account, and the account is the owner of the object, he has all the rights on it
    if (account && object && account->getId() == object->getIdAccount())
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
        if (!LightBird::c().permissions.groupInheritance)
            groups.clear();
    }
    accessors.push_front(accessor->getId());
    // Get the id of the first object in the hierarchy
    if (!object)
        ;
    else if (object->isTable(Table::Files))
    {
        // Check the permission of the file
        file = dynamic_cast<TableFiles *>(object.data());
        if ((granted = this->_idAllowed(accessors, groups, file->getId(), right)) == 2)
            return (true);
        id = file->getIdDirectory();
        checked = true;
    }
    else if (object->isTable(Table::Directories))
    {
        directory = dynamic_cast<TableDirectories *>(object.data());
        id = directory->getId();
    }
    else if (object->isTable(Table::Collections))
    {
        collection = dynamic_cast<TableCollections *>(object.data());
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
    // The root directory of the server
    else if (!object && this->_idAllowed(accessors, groups, "", right) == 2)
        return (true);

    // The permission has been granted
    if (granted == 2)
        return (true);
    // Check if there is a permission at the root of the server
    if (!granted && inheritance && (granted = this->_idAllowed(accessors, groups, "", right)) == 2)
        return (true);
    // If there is no permissions for the accessor on the object, the default is applied
    if (!granted && LightBird::c().permissions.default_)
        return (true);
    return (false);
}

bool    TablePermissions::getRights(const QString &id_accessor, const QString &id_object, QStringList &allowed, QStringList &denied) const
{
    TableAccounts       *account = NULL;
    TableFiles          *file = NULL;
    TableDirectories    *directory = NULL;
    TableCollections    *collection = NULL;
    TableGroups         group;
    bool                inheritance;
    bool                ownerInheritance;
    bool                checked = false;
    QString             id;
    TableDirectories    dir;
    TableCollections    col;
    QStringList         accessors;
    QList<QStringList>  groups;

    allowed.clear();
    denied.clear();
    // If the permissions system is not activated, all the rights are granted
    if (!LightBird::c().permissions.activate)
    {
        allowed.push_back("");
        return (true);
    }
    ownerInheritance = LightBird::c().permissions.ownerInheritance;
    inheritance = LightBird::c().permissions.inheritance;
    QSharedPointer<TableAccessors> accessor(dynamic_cast<TableAccessors *>(Library::database().getTable(Table::Accessor, id_accessor)));
    if (accessor.isNull())
        return (false);
    if (accessor->isTable(Table::Accounts))
        account = dynamic_cast<TableAccounts *>(accessor.data());
    // If the accessor is an account, and the account is administrator, he has all the rights on all the objects
    if (account && account->isAdministrator())
    {
        allowed.push_back("");
        return (true);
    }
    QSharedPointer<TableObjects> object(dynamic_cast<TableObjects *>(Library::database().getTable(Table::Object, id_object)));
    // An empty id_object is the root directory
    if (!object && !id_object.isEmpty())
        return (false);
    // If the accessor is an account, and the account is the owner of the object, he has all the rights on it
    if (account && object && account->getId() == object->getIdAccount())
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
        if (!LightBird::c().permissions.groupInheritance)
            groups.clear();
    }
    accessors.push_front(accessor->getId());
    // Get the id of the first object in the hierarchy
    if (!object)
        ;
    else if (object->isTable(Table::Files))
    {
        file = dynamic_cast<TableFiles *>(object.data());
        id = file->getIdDirectory();
        this->_getRights(accessors, groups, file->getId(), allowed, denied);
        checked = true;
    }
    else if (object->isTable(Table::Directories))
    {
        directory = dynamic_cast<TableDirectories *>(object.data());
        id = directory->getId();
    }
    else if (object->isTable(Table::Collections))
    {
        collection = dynamic_cast<TableCollections *>(object.data());
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
    // The root directory of the server
    else if (!object)
        this->_getRights(accessors, groups, "", allowed, denied);
    // Check if there is a permission at the root of the server
    if (inheritance && object)
        this->_getRights(accessors, groups, "", allowed, denied);
    // If there is no global permission, the default is applied
    if (!allowed.contains("") && !denied.contains(""))
    {
        if (LightBird::c().permissions.default_)
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

unsigned int    TablePermissions::_idAllowed(const QStringList &accessors, const QList<QStringList> &groups, const QString &id_object, const QString &right) const
{
    QSqlQuery            query(Library::database().getDatabase());
    QVector<QVariantMap> result;
    QStringList          rights;
    unsigned int         granted;

    rights.push_back(right);
    // Someone who can modify can read
    if (rights.contains("read"))
        rights.push_back("modify");
    // Someone who can delete can modify
    if (rights.contains("modify"))
        rights.push_back("delete");
    query.prepare(Library::database().getQuery("TablePermissions", "_idAllowed")
                  .replace(":accessors", "'" + accessors.join("','") + "'")
                  .replace(":rights", "'" + rights.join("','") + "'"));
    query.bindValue(":id_object", id_object);
    if (!Library::database().query(query, result) || result.size() <= 0)
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

unsigned int    TablePermissions::_checkRights(const QVector<QVariantMap> &rights, const QStringList &accessors, const QString &right) const
{
    int          i;
    int          s;
    unsigned int granted = 0;
    QList<int>   index;

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

void    TablePermissions::_getRights(const QStringList &accessors, const QList<QStringList> &groups, const QString &id_object, QStringList &allowed, QStringList &denied) const
{
    QSqlQuery            query(Library::database().getDatabase());
    QVector<QVariantMap> result;
    int                  i;
    int                  s;
    QStringList          allowedTmp;
    QStringList          deniedTmp;
    QString              right;

    query.prepare(Library::database().getQuery("TablePermissions", "_getRights")
                  .replace(":accessors", "'" + accessors.join("','") + "'"));
    query.bindValue(":id_object", id_object);
    if (!Library::database().query(query, result) || result.size() <= 0)
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

void    TablePermissions::_mergeRights(QStringList &allowedSrc, QStringList &deniedSrc, QStringList &allowedDest, QStringList &deniedDest) const
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
