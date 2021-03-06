#include <QSharedPointer>
#include <QSqlQuery>
#include <QStringList>

#include "LightBird.h"
#include "Database.h"

Database::Database(LightBird::IApi &api)
    : ITest(api)
{
}

Database::~Database()
{
}

unsigned int    Database::run()
{
    _log.debug("Running the unit tests of the database...", "Database", "run");
    try
    {
        _accounts();
        _collections();
        _directories();
        _events();
        _files();
        _groups();
        _limits();
        _permissions();
        _sessions();
        _tags();
    }
    catch (unsigned int line)
    {
        _log.debug("Unit tests of the database failed!", Properties("line", line).toMap(), "Database", "run");
        return (line);
    }
    _log.debug("Unit tests of the database successful!", "Database", "run");
    return (0);
}

void            Database::_accounts()
{
    LightBird::TableAccounts a1;
    LightBird::TableAccounts a2;
    LightBird::TableGroups g;
    QSqlQuery   query(_database.getDatabase());
    QString     id1;
    QString     id2;

    _log.trace("Running unit tests of the accounts...", "Database", "_accounts");
    query.prepare("DELETE FROM accounts WHERE name=\"a1\" OR name=\"a2\" OR name=\"a3\"");
    _database.query(query);
    query.prepare("DELETE FROM groups WHERE name=\"g1\" OR name=\"g2\" OR name=\"g3\"");
    _database.query(query);
    try
    {
        ASSERT(a1.add("a1", "p1", true, true));
        ASSERT(!a1.add("a1", "p2"));
        ASSERT(a1.getTableId() == LightBird::Table::Accounts);
        ASSERT(a1.getTableName() == "accounts");
        ASSERT(a1.isTable(a1.getTableId()));
        ASSERT(a1.isTable(a1.getTableName()));
        ASSERT(!a1.getId().isEmpty());
        ASSERT(a1.setId(a1.getId()));
        ASSERT(a2.setId(a1.getId()));
        ASSERT(a1.getId() == a2.getId());
        a2.clear();
        ASSERT(a2.getId().isEmpty());
        ASSERT(a2.add("a2", ""));
        id2 = a2.getId();
        ASSERT(a2.add("a3", ""));
        ASSERT(a2.getIdFromName("a3") == a2.getId());
        id1 = a1.getId();
        ASSERT(a1.setId(a2.getId()));
        ASSERT(a2.remove());
        ASSERT(!a2.exists());
        ASSERT(a2.getId().isEmpty());
        ASSERT(!a1.exists());
        ASSERT(!a1);
        ASSERT(a1.getId().isEmpty());
        ASSERT(a1.setId(id1));
        ASSERT(a1.exists());
        ASSERT(a1);
        ASSERT(a2.remove(id2));
        ASSERT(!a2.remove(id2));
        ASSERT(!a2.remove());
        ASSERT(a1.getModified().isValid());
        ASSERT(a1.getCreated().isValid());
        ASSERT(a1.getName() == "a1");
        ASSERT(a1.setName("a3"));
        ASSERT(a1.getName() == "a3");
        ASSERT(a1.setName("a3"));
        LightBird::TableAccounts a3(a1.getId());
        ASSERT(a3.getName() == "a3");
        ASSERT(a2.add("a2"));
        ASSERT(!a2.setName("a3"));
        ASSERT(!a2.setName(""));
        ASSERT(a1.getIdFromNameAndPassword("a3", "p1") == a1.getId());
        ASSERT(a2.setIdFromNameAndPassword("a3", "p1"));
        ASSERT(a2.getId() == a1.getId());
        ASSERT(a2.setIdFromNameAndPassword("a2", ""));
        ASSERT(a2.setIdFromIdentifiantAndSalt(LightBird::sha256("a3" + LightBird::sha256("p1" + a2.getIdFromName("a3").toLatin1()) + "salt").data(), "salt"));
        ASSERT(a2.setIdFromIdentifiantAndSalt(LightBird::sha256(QByteArray("a3") + a1.getPassword().toLatin1() + "salt").data(), "salt"));
        ASSERT(a2.setIdFromIdentifiantAndSalt(LightBird::sha256(QByteArray("a2") + "salt").data(), "salt"));
        ASSERT(a1.getPassword() == LightBird::sha256("p1" + a2.getIdFromName("a3").toLatin1()));
        ASSERT(a1.setPassword(""));
        ASSERT(a1.getPassword().isEmpty());
        ASSERT(a1.setPassword("p2"));
        ASSERT(a1.getPassword() == LightBird::sha256("p2" + a2.getIdFromName("a3").toLatin1()));
        ASSERT(a1.isActive());
        ASSERT(a1.isActive(false));
        ASSERT(!a1.isActive());
        ASSERT(a1.isActive(true));
        ASSERT(a1.isActive());
        ASSERT(a1.isAdministrator());
        ASSERT(a1.isAdministrator(false));
        ASSERT(!a1.isAdministrator());
        ASSERT(a1.isAdministrator(true));
        ASSERT(a1.isAdministrator());
        ASSERT(a1.setInformation("key1", "value1"));
        ASSERT(a1.getInformation("key1") == "value1");
        ASSERT(a1.setInformation("key1", "value2"));
        ASSERT(a1.getInformation("key1") == "value2");
        ASSERT(a1.setInformation("key2", "value3"));
        ASSERT(a1.getInformation("key2") == "value3");
        QVariantMap i;
        i.insert("key1", "value2");
        i.insert("key2", "value3");
        ASSERT(i == a1.getInformations());
        i.insert("key2", "value4");
        i.insert("key3", "value5");
        ASSERT(a1.setInformations(i).isEmpty());
        ASSERT(i == a1.getInformations());
        ASSERT(i.remove("key2"));
        ASSERT(a1.removeInformation("key3"));
        ASSERT(!a1.removeInformations(i.keys()));
        ASSERT(a1.getInformations().size() == 1);
        ASSERT(a1.getInformation("key2") == "value4");
        ASSERT(g.add("g1"));
        ASSERT(a1.addGroup(g.getId()));
        ASSERT(g.add("g2"));
        ASSERT(a1.addGroup(g.getId()));
        ASSERT(a1.getGroups().size() == 2);
        ASSERT(a1.getGroups().contains(g.getId()));
        ASSERT(g.remove());
        ASSERT(a1.getGroups().size() == 1);
        ASSERT(g.remove(a1.getGroups().first()));
        ASSERT(a1.remove());
        ASSERT(a2.remove());
    }
    catch (unsigned int line)
    {
        _log.trace("Unit tests of the accounts failed!", Properties("line", line).toMap(), "Database", "_accounts");
        throw line;
    }
    _log.trace("Unit tests of the accounts successful!", "Database", "_accounts");
}

void            Database::_collections()
{
    LightBird::TableCollections c1;
    LightBird::TableCollections c2;
    LightBird::TableAccounts a;
    LightBird::TableFiles f;
    LightBird::TablePermissions p;
    QSqlQuery   query(_database.getDatabase());
    QStringList l;

    _log.trace("Running unit tests of the collections...", "Database", "_collections");
    query.prepare("DELETE FROM collections WHERE name IN('videos', 'pictures', 'images', 'egypte', 'spiders')");
    _database.query(query);
    query.prepare("DELETE FROM accounts WHERE name IN('a')");
    _database.query(query);
    query.prepare("DELETE FROM files WHERE name IN('toto.xml', 'titi.xml')");
    _database.query(query);
    _api.configuration().set("permissions/activate", "true");
    _api.configuration().set("permissions/default", "false");
    _api.configuration().set("permissions/inheritance", "true");
    _api.configuration().set("permissions/ownerInheritance", "true");
    try
    {
        ASSERT(c1.add("videos"));
        ASSERT(c1.exists());
        ASSERT(c1.exists(c1.getId()));
        ASSERT(!c1.exists("toto"));
        ASSERT(c1.add("pictures"));
        ASSERT(c1.getName() == "pictures");
        ASSERT(c2.setId(c1.getId()));
        ASSERT(c2.getName() == "pictures");
        ASSERT(c1.setName("images"));
        ASSERT(!c1.setName("."));
        ASSERT(!c1.setName(".."));
        ASSERT(!c1.setName("images/"));
        ASSERT(c2.getName() == "images");
        ASSERT(c1.getIdCollection().isEmpty());
        ASSERT(c2.getIdAccount().isEmpty());
        ASSERT(c2.add("bahrain", c1.getId()));
        ASSERT(c2.getIdCollection() == c1.getId());
        ASSERT(c2.add("france", c1.getId()));
        ASSERT(c2.add("spiders", c2.getId()));
        ASSERT(c2.add("egypte", c2.getIdCollection()));
        ASSERT(c2.getVirtualPath() == "images/france/egypte");
        ASSERT(c2.getVirtualPath(true, true) == "/images/france/egypte/");
        ASSERT(c1.getVirtualPath(true) == "/images");
        ASSERT(c2.setIdCollection(c1.getId()));
        ASSERT(c2.getIdCollection() == c1.getId());
        ASSERT(c2.setIdCollection(c1.getIdCollection()));
        ASSERT(c2.getIdCollection().isEmpty());
        ASSERT(c2.setVirtualPath("////\\\\/images///france/\\\\"));
        ASSERT(c2.getIdCollection() == c1.getIdFromVirtualPath("///\\images///france\\"));
        ASSERT(c2.setVirtualPath("images"));
        ASSERT(c2.getIdCollection() == c1.getId());
        ASSERT(f.add("toto.xml", "toto.xml", ""));
        ASSERT(!f.getCollections().size());
        ASSERT(f.addCollection(c1.getId()));
        ASSERT((l = f.getCollections()).size() == 1);
        ASSERT(l.contains(c1.getId()));
        ASSERT(f.add("titi.xml", "titi.xml", ""));
        ASSERT(f.addCollection(c1.getId()));
        ASSERT((l = c1.getCollections()).size() == 3);
        ASSERT(l.contains(c1.getIdFromVirtualPath("images/france")));
        ASSERT(l.contains(c1.getIdFromVirtualPath("images/egypte")));
        ASSERT(l.contains(c1.getIdFromVirtualPath("images/bahrain")));
        ASSERT((l = c1.getFiles()).size() == 2);
        ASSERT(l.contains(f.getIdFromVirtualPath("toto.xml")));
        ASSERT(l.contains(f.getIdFromVirtualPath("\\///titi.xml")));
        ASSERT(a.add("a"));
        ASSERT(!c2.add("france", c1.getIdFromVirtualPath("images")));
        ASSERT(c2.add("france", c1.getIdFromVirtualPath("images"), a.getId()));
        ASSERT(c1.getIdFromVirtualPath("images/france") != c2.getId());
        ASSERT(c1.getIdFromVirtualPath("images/france", a.getId()) == c2.getId());
        ASSERT(c2.remove());
        ASSERT(c2.setIdFromVirtualPath("images/egypte"));
        ASSERT(c2.getParents() == QStringList() << c2.getIdFromVirtualPath("images"));
        ASSERT(c1.getParents().isEmpty());
        ASSERT(!c2.setIdCollection(c2.getId()));
        ASSERT(c1.getCollections(a.getId(), "read").size() == 0);
        ASSERT(c1.getFiles(a.getId(), "read").size() == 0);
        ASSERT(p.add(a.getId(), c2.getId(), "read"));
        ASSERT(p.add(a.getId(), f.getId(), "read"));
        ASSERT((l = c1.getCollections(a.getId(), "read")).size() == 1);
        ASSERT(l.contains(c2.getId()));
        ASSERT((l = c1.getFiles(a.getId(), "read")).size() == 1);
        ASSERT(l.contains(f.getId()));
        ASSERT(f.removeCollection(c1.getId()));
        ASSERT(c2.remove(c1.getId()));
        ASSERT(!c2.remove());
        ASSERT(c2.remove(c1.getIdFromVirtualPath("videos")));
        ASSERT(f.remove(f.getIdFromVirtualPath("toto.xml")));
        ASSERT(f.remove(f.getIdFromVirtualPath("titi.xml")));
    }
    catch (unsigned int line)
    {
        _log.trace("Unit tests of the collections failed!", Properties("line", line).toMap(), "Database", "_collections");
        throw line;
    }
    _log.trace("Unit tests of the collections successful!", "Database", "_collections");
}

void            Database::_directories()
{
    LightBird::TableDirectories d1;
    LightBird::TableDirectories d2;
    LightBird::TableFiles f;
    LightBird::TableAccounts a;
    LightBird::TablePermissions p;
    QSqlQuery   query(_database.getDatabase());
    QStringList l;
    QStringList m;
    QString     d;

    _log.trace("Running unit tests of the directories...", "Database", "_directories");
    query.prepare("DELETE FROM directories WHERE name IN('videos', 'images..', '...', 'pictures', 'images', 'egypte', 'spiders', 'pictures')");
    _database.query(query);
    query.prepare("DELETE FROM files WHERE name IN('toto.png', 'titi.png', 'file.png')");
    _database.query(query);
    query.prepare("DELETE FROM accounts WHERE name IN('a')");
    _database.query(query);
    _api.configuration().set("permissions/activate", "true");
    _api.configuration().set("permissions/default", "false");
    _api.configuration().set("permissions/inheritance", "true");
    _api.configuration().set("permissions/ownerInheritance", "true");
    try
    {
        ASSERT(d1.add("videos"));
        ASSERT(d1.exists());
        ASSERT(d1.exists(d1.getId()));
        ASSERT(!d1.exists("toto"));
        ASSERT(d1.add("pictures"));
        ASSERT(d1.getName() == "pictures");
        ASSERT(d2.setId(d1.getId()));
        ASSERT(d2.getName() == "pictures");
        ASSERT(!d1.setName("."));
        ASSERT(!d1.setName(".."));
        ASSERT(d1.setName("..."));
        ASSERT(d1.setName("images.."));
        ASSERT(!d1.setName("images/"));
        ASSERT(d1.setName("images"));
        ASSERT(d2.getName() == "images");
        ASSERT(d1.getIdDirectory().isEmpty());
        ASSERT(d2.getIdAccount().isEmpty());
        ASSERT(d2.add("bahrain", d1.getId()));
        ASSERT(d2.getIdDirectory() == d1.getId());
        ASSERT(d2.add("france", d1.getId()));
        ASSERT(!d2.add("/france", d1.getId()));
        ASSERT(!d2.add(".", d1.getId()));
        ASSERT(d2.add("spiders", d2.getId()));
        ASSERT(d2.add("egypte", d2.getIdDirectory()));
        ASSERT(d2.getVirtualPath() == "images/france/egypte");
        ASSERT(d2.getVirtualPath(true, true) == "/images/france/egypte/");
        ASSERT(d1.getVirtualPath(true) == "/images");
        ASSERT(d2.setIdDirectory(d1.getId()));
        ASSERT(d2.getIdDirectory() == d1.getId());
        ASSERT(d2.setIdDirectory(d1.getIdDirectory()));
        ASSERT(d2.getIdDirectory().isEmpty());
        ASSERT(d2.setVirtualPath("////\\\\/images///france/\\\\"));
        ASSERT(d2.getIdDirectory() == d1.getIdFromVirtualPath("///\\images///france\\"));
        ASSERT(d2.setVirtualPath("images"));
        ASSERT(d2.getIdDirectory() == d1.getId());
        ASSERT(f.add("toto.png", "toto.png", "", d1.getId()));
        ASSERT(f.add("titi.png", "titi.png", "", d1.getId()));
        ASSERT(f.add("file.png", "file.png", "", d1.getDirectories().first()));
        ASSERT(d1.getAllFiles().contains(f.getId()) && d1.getAllFiles().contains(f.getIdFromVirtualPath("images/titi.png")));
        ASSERT(!d1.getFiles().contains("file.png"));
        ASSERT(a.add("a"));
        ASSERT((l = d1.getDirectories()).size() == 3);
        ASSERT(l.contains(d1.getIdFromVirtualPath("images/france")));
        ASSERT(l.contains(d1.getIdFromVirtualPath("images/egypte")));
        ASSERT(l.contains(d1.getIdFromVirtualPath("images/bahrain")));
        ASSERT((l = d1.getFiles()).size() == 2);
        ASSERT(l.contains(f.getIdFromVirtualPath("images/toto.png")));
        ASSERT(l.contains(f.getIdFromVirtualPath("images/titi.png")));
        ASSERT(!d1.getDirectories(a.getId(), "read").size());
        ASSERT(!d1.getFiles(a.getId(), "read").size());
        ASSERT(p.add(a.getId(), d1.getId(), "read"));
        ASSERT(d1.getDirectories(a.getId(), "read").size() == 3);
        ASSERT(d1.getFiles(a.getId(), "read").size() == 2);
        ASSERT(d1.isAllowed(a.getId(), "read"));
        ASSERT(d1.getRights(a.getId(), l, m));
        ASSERT(l.contains("read"));
        ASSERT(m.contains(""));
        ASSERT(p.add(a.getId(), d1.getIdFromVirtualPath("images/france"), "read", false));
        ASSERT((l = d1.getDirectories(a.getId(), "read")).size() == 2);
        ASSERT(!l.contains(d1.getIdFromVirtualPath("images/france")));
        ASSERT(l.contains(d1.getIdFromVirtualPath("images/bahrain")));
        ASSERT(p.add(a.getId(), f.getIdFromVirtualPath("images/toto.png"), "read", false));
        ASSERT((l = d1.getFiles(a.getId(), "read")).size() == 1);
        ASSERT(!l.contains(f.getIdFromVirtualPath("images/toto.png")));
        ASSERT(l.contains(f.getIdFromVirtualPath("images/titi.png")));
        ASSERT(d2.remove(d1.getId()));
        ASSERT(!d2.remove());
        ASSERT(d2.remove(d1.getIdFromVirtualPath("videos")));
        ASSERT(!(d = d1.createVirtualPath("////videos////././///d2")).isEmpty());
        ASSERT(d1.setIdFromVirtualPath("videos"));
        d2.clear();
        ASSERT(d2.getDirectory("videos") == d1.getId());
        ASSERT(!d2.setIdFromVirtualPath("videos/d1"));
        ASSERT(d2.setIdFromVirtualPath("/////videos////d2///"));
        ASSERT(d2.getId() == d);
        ASSERT(d1.createVirtualPath("//././///d2") == d2.getId());
        ASSERT(d1.getDirectory("d2") == d2.getId());
        ASSERT(d1.getDirectory("d3").isEmpty());
        ASSERT(!d1.createVirtualPath("/////d2////d3///").isEmpty());
        ASSERT(d1.setIdFromVirtualPath("videos/d2/d3"));
        ASSERT(d1.createVirtualPath("//") == d1.getId());
        ASSERT(d1.createVirtualPath("") == d1.getId());
        ASSERT(d1.cd("/") && !d1.exists());
        ASSERT(d1.cd("/videos/d2/d3") && d1.getVirtualPath() == "videos/d2/d3");
        ASSERT(d1.cd("..") && d1.getVirtualPath() == "videos/d2");
        ASSERT(d1.cd("../d2/d3") && d1.getVirtualPath() == "videos/d2/d3");
        ASSERT(d1.cd("../..") && d1.getVirtualPath() == "videos");
        ASSERT(!d1.cd("../..") && d1.getVirtualPath() == "videos");
        ASSERT(d1.cd("..") && !d1.exists());
        ASSERT(!d1.cd("d2"));
        ASSERT(d1.cd("videos/d2/../d2/../d2/d3") && d1.getVirtualPath() == "videos/d2/d3");
        ASSERT(d1.getParents() == (QStringList() << d1.getIdFromVirtualPath("videos/d2") << d1.getIdFromVirtualPath("videos")));
        ASSERT(d1.cd("/videos/d2") && d1.getVirtualPath() == "videos/d2");
        d1.clear();
        ASSERT(d1.cd("videos/d2") && d1.getVirtualPath() == "videos/d2");
        ASSERT(d1.getParents() == (QStringList() << d1.getIdFromVirtualPath("videos")));
        ASSERT(d1.cd(".."));
        ASSERT(d1.getParents().isEmpty());
        ASSERT(!d1.setIdDirectory(d1.getId()));
        ASSERT(d1.remove(d1.getIdFromVirtualPath("videos")));
        // Rights on the root directory
        ASSERT(!p.isAllowed(a.getId(), "", "add"));
        ASSERT(p.add(a.getId(), "", "add", true));
        ASSERT(p.isAllowed(a.getId(), "", "add"));
        QStringList allowed, denied;
        ASSERT(p.getRights(a.getId(), "", allowed, denied));
        ASSERT(allowed.size() == 1 && allowed[0] == "add");
        ASSERT(denied.size() == 1 && denied[0] == "");
        ASSERT(p.isGranted(false));
        ASSERT(!p.isAllowed(a.getId(), "", "add"));
        ASSERT(p.getRights(a.getId(), "", allowed, denied));
        ASSERT(allowed.isEmpty());
        ASSERT(denied.size() == 1 && denied[0] == "");
        ASSERT(a.remove());
        ASSERT(!p.exists());
    }
    catch (unsigned int line)
    {
        _log.trace("Unit tests of the directories failed!", Properties("line", line).toMap(), "Database", "_directories");
        throw line;
    }
    _log.trace("Unit tests of the directories successful!", "Database", "_directories");
}

void            Database::_events()
{
    LightBird::TableEvents e1;
    LightBird::TableEvents e2;
    QVariantMap informations;
    QSqlQuery   query(_database.getDatabase());
    QStringList events;

    _log.trace("Running unit tests of the events...", "Database", "_events");
    query.prepare("DELETE FROM events WHERE name=\"e1\"");
    _database.query(query);
    try
    {
        ASSERT(e1.getId().isEmpty());
        informations["name1"] = "value1";
        informations["name2"] = "value2";
        informations["name3"] = "value3";
        ASSERT(e1.add("e1", informations, "", ""));
        ASSERT(!e1.getId().isEmpty());
        ASSERT(e1.getName() == "e1");
        ASSERT(e1.getCreated().isValid());
        ASSERT(e1.getIdAccessor().isEmpty());
        ASSERT(e1.getIdObject().isEmpty());
        ASSERT(e1.getInformation("name1") == "value1");
        ASSERT(e1.getInformation("name42").toString().isEmpty());
        ASSERT(e1.getInformations() == informations);
        ASSERT(e2.setId(e1.getId()));
        ASSERT(e2.setName("e2"));
        ASSERT(e2.getName() == "e2");
        ASSERT(e2.setIdAccessor(""));
        ASSERT(e2.getIdAccessor().isEmpty());
        ASSERT(e2.setIdObject(""));
        ASSERT(e2.getIdObject().isEmpty());
        ASSERT(e2.setInformation("name2", "value2new"));
        ASSERT(e2.getInformation("name2") == "value2new");
        ASSERT(e2.setInformation("name4", "value4"));
        ASSERT(e2.getInformation("name4") == "value4");
        informations.clear();
        informations["name1"] = "value1new";
        informations["name3"] = "value3new";
        informations["name5"] = "value5";
        ASSERT(e2.setInformations(informations).isEmpty());
        informations["name2"] = "value2new";
        informations["name4"] = "value4";
        ASSERT(e2.getInformations() == informations);
        ASSERT(e2.removeInformation("name5"));
        informations.remove("name5");
        ASSERT(e2.getInformations() == informations);
        ASSERT((events = e1.getEvents("e2", QDateTime::currentDateTime().addSecs(-40))).size());
        ASSERT(e1.setId(events.front()));
        ASSERT(e1.getName() == "e2");
        ASSERT(e2.remove());
        ASSERT(!e1.remove());
    }
    catch (unsigned int line)
    {
        _log.trace("Unit tests of the events failed!", Properties("line", line).toMap(), "Database", "_events");
        throw line;
    }
    _log.trace("Unit tests of the events successful!", "Database", "_events");
}

void            Database::_files()
{
    LightBird::TableFiles f1;
    LightBird::TableFiles f2;
    LightBird::TableDirectories d1;
    LightBird::TableAccounts a1;
    QSqlQuery   query(_database.getDatabase());

    _log.trace("Running unit tests of the files...", "Database", "_files");
    query.prepare("DELETE FROM files WHERE name IN('f1', 'f2', 'f3')");
    _database.query(query);
    query.prepare("DELETE FROM directories WHERE name=\"d1\"");
    _database.query(query);
    query.prepare("DELETE FROM accounts WHERE name=\"a1\"");
    _database.query(query);
    try
    {
        ASSERT(d1.add("d1"));
        ASSERT(a1.add("a1"));
        ASSERT(f1.add("f3", "f3.xml"));
        ASSERT(f2.add("f8", "f2.xml", "document", d1.getId(), a1.getId()));
        ASSERT(f2.getIdAccount() == a1.getId());
        ASSERT(f2.setIdAccount());
        ASSERT(f2.getIdAccount().isEmpty());
        ASSERT(f2.setIdAccount(a1.getId()));
        ASSERT(f2.getIdAccount() == a1.getId());
        ASSERT(f1.getName() == "f3");
        ASSERT(f2.getName() == "f8");
        ASSERT(f1.setName("f1"));
        ASSERT(f2.setName("f2"));
        ASSERT(!f2.setName("."));
        ASSERT(!f2.setName(".."));
        ASSERT(!f2.setName("f2/"));
        ASSERT(f1.getName() == "f1");
        ASSERT(f2.getName() == "f2");
        ASSERT(f1.getIdFromVirtualPath("d1/f2") == f2.getId());
        ASSERT(f1.getIdFromVirtualPath("\\\\d1\\f2/////") == f2.getId());
        ASSERT(f1.getIdFromVirtualPath("\\\\d3\\f2/////").isEmpty());
        ASSERT(f1.getIdFromVirtualPath("f1") == f1.getId());
        ASSERT(f1.getIdFromVirtualPath("////f1////") == f1.getId());
        ASSERT(f1.getIdFromVirtualPath("").isEmpty());
        ASSERT(f1.getIdFromVirtualPath("/////").isEmpty());
        ASSERT(f1.getPath() == "f3.xml");
        ASSERT(f2.getPath() == "f2.xml");
        ASSERT(f2.getId() == f2.getIdFromPath(f2.getPath()));
        ASSERT(f2.setIdFromPath(f2.getPath()) && f2.getPath() == "f2.xml");
        ASSERT(f2.getIdFromPath("f9.xml").isEmpty());
        ASSERT(f1.setPath("f1.xml"));
        ASSERT(f1.getPath() == "f1.xml");
        ASSERT(f2.setPath("f1"));
        ASSERT(f2.getPath() == "f1");
        ASSERT(f2.setName("f2"));
        ASSERT(f1.getType() == "other");
        ASSERT(f2.getType() == "document");
        ASSERT(f1.setType("image"));
        ASSERT(f1.getType() == "image");
        ASSERT(f2.setType("test"));
        ASSERT(f2.getType() == "other");
        ASSERT(f2.getIdDirectory() == d1.getId());
        ASSERT(f2.setIdDirectory());
        ASSERT(f2.getIdDirectory().isEmpty());
        ASSERT(f2.setIdDirectory(d1.getId()));
        ASSERT(f2.getIdDirectory() == d1.getId());
        ASSERT(f1.getVirtualPath().isEmpty());
        ASSERT(f1.getVirtualPath(true) == "/");
        ASSERT(f1.getVirtualPath(false, true) == f1.getName());
        ASSERT(f2.getVirtualPath() == "d1");
        ASSERT(f2.getVirtualPath(true) == "/d1");
        ASSERT(f2.getVirtualPath(true, true) == "/d1/f2");
        ASSERT(f1.setInformation("k1", "v1"));
        ASSERT(f1.setInformation("k2", "v1"));
        ASSERT(f1.getInformation("k1") == "v1");
        ASSERT(f1.setInformation("k1", "v2"));
        ASSERT(f1.getInformation("k1") == "v2");
        ASSERT(!f1.setInformation("toto", QVariant()));
        QVariantMap i;
        ASSERT((i = f1.getInformations()).size() == 2);
        ASSERT(f2.getInformations().isEmpty());
        i.remove("k1");
        i.insert("k3", "v3");
        i.insert("k4", "v4");
        ASSERT(f1.setInformations(i).isEmpty());
        ASSERT(f1.getInformations().size() == 4);
        ASSERT(f1.removeInformation("k1"));
        ASSERT(f1.getInformations().size() == 3);
        ASSERT(!f1.getInformations().contains("k1"));
        ASSERT(!f1.removeInformation("toto"));
        i.remove("k2");
        ASSERT(f1.removeInformations(i.keys()));
        ASSERT(f1.getInformations().size() == 1);
        ASSERT(f2.remove(f1.getId()));
        ASSERT(f2.remove());
    }
    catch (unsigned int line)
    {
        _log.trace("Unit tests of the files failed!", Properties("line", line).toMap(), "Database", "_files");
        throw line;
    }
    _log.trace("Unit tests of the files successful!", "Database", "_files");
}

void            Database::_groups()
{
    LightBird::TableGroups g1;
    LightBird::TableGroups g2;
    LightBird::TableAccounts a;
    QSqlQuery   query(_database.getDatabase());
    QString     id1;
    QString     id2;

    _log.trace("Running unit tests of the groups...", "Database", "_groups");
    query.prepare("DELETE FROM groups WHERE name=\"b1\" OR name=\"b2\" OR name=\"b3\"");
    _database.query(query);
    query.prepare("DELETE FROM accounts WHERE name=\"a1\" OR name=\"a2\" OR name=\"a3\"");
    _database.query(query);
    try
    {
        ASSERT(g1.add("b1"));
        ASSERT(!g1.add("b1", ""));
        ASSERT(g1.getTableId() == LightBird::Table::Groups);
        ASSERT(g1.getTableName() == "groups");
        ASSERT(g1.isTable(g1.getTableId()));
        ASSERT(g1.isTable(g1.getTableName()));
        ASSERT(!g1.getId().isEmpty());
        ASSERT(g1.setId(g1.getId()));
        ASSERT(g2.setId(g1.getId()));
        ASSERT(g1.getId() == g2.getId());
        g2.clear();
        ASSERT(g2.getId().isEmpty());
        ASSERT(g2.add("b2", ""));
        id2 = g2.getId();
        ASSERT(g2.add("a3", ""));
        id1 = g1.getId();
        ASSERT(g1.setId(g2.getId()));
        ASSERT(g2.remove());
        ASSERT(!g2.exists());
        ASSERT(g2.getId().isEmpty());
        ASSERT(!g1.exists());
        ASSERT(g1.setId(id1));
        ASSERT(g2.remove(id2));
        ASSERT(!g2.remove(id2));
        ASSERT(!g2.remove());
        ASSERT(g1.getModified().isValid());
        ASSERT(g1.getCreated().isValid());
        ASSERT(g1.getName() == "b1");
        ASSERT(g1.setName("a3"));
        ASSERT(g1.getName() == "a3");
        ASSERT(g1.setName("a3"));
        ASSERT(g2.add("b2"));
        ASSERT(!g2.setName("a3"));
        ASSERT(!g2.setName(""));
        ASSERT(g1.remove());
        ASSERT(g2.remove());
        ASSERT(g1.add("b1", ""));
        ASSERT(!g2.add("b2", "toto"));
        ASSERT(g2.add("b2", g1.getId()));
        ASSERT(!g2.add("b2", g2.getIdGroup()));
        ASSERT(g2.add("b3", g2.getIdGroup()));
        ASSERT(g2.getParents() == QStringList() << g1.getId());
        ASSERT(g1.getParents().isEmpty());
        ASSERT(!g2.setIdGroup(g2.getId()));
        ASSERT(!g2.setName("b2"));
        ASSERT(g2.setName("b1"));
        ASSERT(g1.getIdGroup().isEmpty());
        ASSERT(!g2.setIdGroup(g1.getIdGroup()));
        ASSERT(g2.getIdGroup() == g1.getId());
        ASSERT(a.add("a1"));
        ASSERT(g1.addAccount(a.getId()));
        ASSERT(a.add("a2"));
        ASSERT(g1.addAccount(a.getId()));
        ASSERT(g1.getAccounts().size() == 2);
        ASSERT(g1.getAccounts().contains(a.getId()));
        ASSERT(a.remove());
        ASSERT(g1.getAccounts().size() == 1);
        ASSERT(a.remove(g1.getAccounts().first()));
        ASSERT(g1.remove());
    }
    catch (unsigned int line)
    {
        _log.trace("Unit tests of the groups failed!", Properties("line", line).toMap(), "Database", "_groups");
        throw line;
    }
    _log.trace("Unit tests of the groups successful!", "Database", "_groups");
}

void            Database::_limits()
{
    LightBird::TableLimits l;
    LightBird::TableGroups g1;
    LightBird::TableGroups g2;
    LightBird::TableDirectories d1;
    QSqlQuery   query(_database.getDatabase());

    _log.trace("Running unit tests of the limits...", "Database", "_limits");
    query.prepare("DELETE FROM limits WHERE name IN('l1', 'l2')");
    _database.query(query);
    query.prepare("DELETE FROM groups WHERE name IN('g1', 'g2')");
    _database.query(query);
    query.prepare("DELETE FROM directories WHERE name IN('d1')");
    _database.query(query);
    try
    {
        ASSERT(g1.add("g1"));
        ASSERT(g2.add("g2"));
        ASSERT(d1.add("d1"));
        ASSERT(l.add("l1", "v1", g2.getId(), d1.getId()));
        ASSERT(g2.getLimits().size() == 1);
        ASSERT(g2.getLimits().contains(l.getId()));
        ASSERT(l.getName() == "l1");
        ASSERT(l.setName("l2"));
        ASSERT(l.getName() == "l2");
        ASSERT(l.setName("l3"));
        ASSERT(l.getName() == "l3");
        ASSERT(l.getValue() == "v1");
        ASSERT(l.setValue("v2"));
        ASSERT(l.getValue() == "v2");
        ASSERT(l.setValue("l4"));
        ASSERT(l.getValue() == "l4");
        ASSERT(l.getIdAccessor() == g2.getId());
        ASSERT(l.setIdAccessor(""));
        ASSERT(l.getIdAccessor().isEmpty());
        ASSERT(l.setIdAccessor(g1.getId()));
        ASSERT(l.getIdAccessor() == g1.getId());
        ASSERT(l.getIdObject() == d1.getId());
        ASSERT(l.setIdObject(""));
        ASSERT(l.getIdObject().isEmpty());
        ASSERT(l.setIdObject(d1.getId()));
        ASSERT(l.getIdObject() == d1.getId());
        ASSERT(g2.remove());
        ASSERT(l.exists());
        ASSERT(g1.remove());
        ASSERT(!l.exists());
        ASSERT(d1.remove());
    }
    catch (unsigned int line)
    {
        _log.trace("Unit tests of the limits failed!", Properties("line", line).toMap(), "Database", "_limits");
        throw line;
    }
    _log.trace("Unit tests of the limits successful!", "Database", "_limits");
}

void            Database::_permissions()
{
    LightBird::TablePermissions p;
    LightBird::TableAccounts a;
    LightBird::TableGroups g1;
    LightBird::TableGroups g2;
    LightBird::TableDirectories d1;
    LightBird::TableDirectories d2;
    LightBird::TableCollections c;
    LightBird::TableFiles f;
    QSqlQuery   query(_database.getDatabase());
    QString     id;
    QString     id_object;
    QStringList allowed;
    QStringList denied;

    _log.trace("Running unit tests of the permissions...", "Database", "_permissions");
    query.prepare("DELETE FROM directories WHERE name IN('d', 'Directory1', 'Directory6', 'Directory7')");
    _database.query(query);
    query.prepare("DELETE FROM accounts WHERE name IN('a')");
    _database.query(query);
    query.prepare("DELETE FROM groups WHERE name IN('g1', 'g2')");
    _database.query(query);
    query.prepare("DELETE FROM permissions WHERE id_accessor='' AND id_object=''");
    _database.query(query);
    _api.configuration().set("permissions/activate", "true");
    _api.configuration().set("permissions/default", "false");
    _api.configuration().set("permissions/inheritance", "true");
    _api.configuration().set("permissions/ownerInheritance", "true");
    _api.configuration().set("permissions/groupInheritance", "true");
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
        ASSERT(!p.getRights(id, "42", allowed, denied));
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
        _api.configuration().set("permissions/activate", "false");
        ASSERT(p.isAllowed(id, d1.getIdFromVirtualPath("Directory1/Directory2"), "read"));
        ASSERT(p.isAllowed(id, d1.getIdFromVirtualPath("Directory1/Directory2/Directory3"), "read"));
        ASSERT(p.isAllowed(id, f.getIdFromVirtualPath("Directory1/Directory2/Directory3/File1"), "read"));
        ASSERT(p.isAllowed(id, f.getIdFromVirtualPath("Directory1/Directory2/Directory3/File2"), "read"));
        ASSERT(p.isAllowed(id, d1.getIdFromVirtualPath("Directory1/Directory2/Directory4"), "modify"));
        ASSERT(p.isAllowed(id, f.getIdFromVirtualPath("Directory1/Directory2/File4"), "delete"));
        _api.configuration().set("permissions/activate", "true");
        ASSERT(f.setIdFromVirtualPath("Directory1/Directory2/Directory3/File2"));
        ASSERT(!f.isAllowed(a.getId(), "read"));
        ASSERT(f.setIdAccount(a.getId()));
        ASSERT(f.isAllowed(a.getId(), "read"));
        ASSERT(f.setIdAccount());
        ASSERT(!f.isAllowed(a.getId(), "read"));
        ASSERT(d1.setIdFromVirtualPath("Directory1/Directory2/Directory3"));
        ASSERT(d1.setIdAccount(a.getId()));
        ASSERT(f.isAllowed(a.getId(), "read"));
        _api.configuration().set("permissions/ownerInheritance", "false");
        ASSERT(!f.isAllowed(a.getId(), "read"));
        _api.configuration().set("permissions/ownerInheritance", "true");
        ASSERT(d1.setIdAccount());
        ASSERT(f.setIdFromVirtualPath("Directory1/Directory2/File4"));
        ASSERT(d2.setIdFromVirtualPath("Directory1/Directory2/Directory4"));
        ASSERT(d1.setIdFromVirtualPath("Directory1/Directory2"));
        ASSERT(f.isAllowed(a.getId(), "read"));
        ASSERT(d2.isAllowed(a.getId(), "read"));
        _api.configuration().set("permissions/inheritance", "false");
        ASSERT(!f.isAllowed(a.getId(), "read"));
        ASSERT(!d2.isAllowed(a.getId(), "read"));
        ASSERT(d1.isAllowed(a.getId(), "read"));
        ASSERT(d1.setIdFromVirtualPath("Directory1"));
        ASSERT(d2.setIdFromVirtualPath("Directory6"));
        ASSERT(!d1.isAllowed(a.getId(), "read"));
        ASSERT(d2.isAllowed(a.getId(), "read"));
        _api.configuration().set("permissions/default", "true");
        ASSERT(d1.isAllowed(a.getId(), "read"));
        ASSERT(d2.isAllowed(a.getId(), "read"));
        ASSERT(p.setId(a.getId(), d2.getId(), "read"));
        ASSERT(p.isGranted(false));
        ASSERT(!d2.isAllowed(a.getId(), "read"));
        ASSERT(f.setIdFromVirtualPath("Directory7/File9"));
        ASSERT(f.isAllowed(a.getId(), "read"));
        _api.configuration().set("permissions/default", "false");
        ASSERT(!f.isAllowed(a.getId(), "read"));
        _api.configuration().set("permissions/inheritance", "true");
        ASSERT(p.add(a.getId(), "", "read", false));
        ASSERT(!d1.isAllowed(a.getId(), "read"));
        ASSERT(p.isGranted(true));
        ASSERT(d1.isAllowed(a.getId(), "read"));
        ASSERT(f.setIdFromVirtualPath("Directory7/Directory8/File8"));
        ASSERT(d1.setIdFromVirtualPath("Directory7/Directory8"));
        ASSERT(p.setId(a.getId(), d1.getId(), "read"));
        ASSERT(p.setRight("modify"));
        _api.configuration().set("permissions/inheritance", "false");
        ASSERT(!f.isAllowed(a.getId(), "read"));
        ASSERT(!f.isAllowed(a.getId(), "modify"));
        _api.configuration().set("permissions/inheritance", "true");
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
        _api.configuration().set("permissions/inheritance", "false");
        ASSERT(a.getRights(f.getId(), allowed, denied));
        ASSERT(allowed.contains("read") && denied.contains("") && allowed.size() == 1 && denied.size() == 1);
        _api.configuration().set("permissions/inheritance", "true");
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
        _api.configuration().set("permissions/default", "false");
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
        _api.configuration().set("permissions/default", "true");
        ASSERT(g2.isAllowed(f.getId(), "read"));
        _api.configuration().set("permissions/groupInheritance", "false");
        ASSERT(a.isAllowed(f.getId(), "read"));
        ASSERT(g1.isAllowed(f.getId(), "read"));
        ASSERT(d1.remove(d1.getIdFromVirtualPath("Directory1")));
        ASSERT(d1.remove(d1.getIdFromVirtualPath("Directory6")));
        ASSERT(d1.remove(d1.getIdFromVirtualPath("Directory7")));
        ASSERT(a.remove());
        ASSERT(g1.remove(g2.getIdFromName("g1").first()));
        ASSERT(g1.remove(g2.getIdFromName("g2").first()));
    }
    catch (unsigned int line)
    {
        _log.trace("Unit tests of the permissions failed!", Properties("line", line).toMap(), "Database", "_permissions");
        throw line;
    }
    _log.trace("Unit tests of the permissions successful!", "Database", "_permissions");
}

void            Database::_sessions()
{
    LightBird::TableAccounts a;
    QSqlQuery   query(_database.getDatabase());
    QString     id1;
    QString     id2;
    QVariantMap i;
    QStringList l;

    _log.trace("Running unit tests of the sessions...", "Database", "_sessions");
    query.prepare("DELETE FROM accounts WHERE name=\"a1\"");
    _database.query(query);
    try
    {
        ASSERT(a.add("a1", "p1"));
        i["key1"] = "value1";
        i["key2"] = 42;
        i["key3"] = "value3";
        id1 = _api.sessions().create()->getId();
        id2 = _api.sessions().create(QDateTime::currentDateTime().addSecs(10), a.getId(), QStringList() << "client1" << "client2", i)->getId();
        ASSERT(!id1.isEmpty());
        ASSERT(!id2.isEmpty());
        LightBird::Session s1(_api.sessions().getSession(id1));
        LightBird::Session s2(_api.sessions().getSession(id2));
        ASSERT(s1->getClients().isEmpty());
        ASSERT(s2->getClients() == (QStringList() << "client1" << "client2"));
        ASSERT(s2->setClient("client3"));
        ASSERT(s2->setClient("client1"));
        ASSERT(s2->getClients() == (QStringList() << "client1" << "client2" << "client3"));
        ASSERT(s2->removeClient("client2"));
        ASSERT(s2->getClients() == (QStringList() << "client1" << "client3"));
        ASSERT(s2->setClients(QStringList() << "client4" << "client5" << "client1"));
        ASSERT(s2->getClients() == (QStringList() << "client1" << "client3" << "client4" << "client5"));
        ASSERT(s2->removeClients(QStringList() << "client1" << "client4"));
        ASSERT(s2->getClients() == (QStringList() << "client3" << "client5"));
        ASSERT(s2->removeClients());
        ASSERT(s2->getClients().isEmpty());
        ASSERT(s2->setClients(QStringList() << "client3" << "client5"));
        ASSERT(!s2->setAccount("test"));
        ASSERT(s1->setAccount(s2->getAccount()));
        ASSERT(s2->setAccount());
        ASSERT(s2->getAccount().isEmpty());
        ASSERT(s1->getAccount() == a.getId());
        ASSERT(s2->setAccount(a.getId()));
        ASSERT(s1->getCreation() < QDateTime::currentDateTime().addSecs(10));
        ASSERT(s1->getCreation() > QDateTime::currentDateTime().addSecs(-10));
        ASSERT(s2->getInformation("key2") == 42);
        ASSERT(s2->getInformation("key1") == "value1");
        ASSERT(s2->setInformation("key1", "value2"));
        ASSERT(s2->getInformation("key1") == "value2");
        ASSERT(s2->setInformation("key2", "value3"));
        ASSERT(s2->getInformation("key2") == "value3");
        i.insert("key1", "value2");
        i.insert("key2", "value3");
        ASSERT(i == s2->getInformations());
        i.insert("key2", "value4");
        i.insert("key3", "value5");
        ASSERT(s2->setInformations(i));
        ASSERT(i == s2->getInformations());
        ASSERT(i.remove("key2"));
        ASSERT(s2->removeInformation("key3"));
        ASSERT(!s2->removeInformations(i.keys()));
        ASSERT(s2->getInformations().size() == 1);
        ASSERT(s2->getInformation("key2") == "value4");
        ASSERT(!s1->isExpired());
        ASSERT(!s2->isExpired());
        ASSERT(_api.sessions().getSession(s1->getId()).data() == s1.data());
        ASSERT(_api.sessions().destroy(s1->getId()));
        ASSERT(s1->isExpired());
        ASSERT(_api.sessions().getSession(s1->getId()).isNull());
        s1 = _api.sessions().create(QDateTime::currentDateTime().addSecs(10), a.getId(), QStringList() << "client1" << "client3", i);
        ASSERT(s1->getInformations().size() == i.size());
        ASSERT(s1->removeInformations());
        ASSERT(s1->getInformations().isEmpty());
        ASSERT(!s1->isExpired());
        ASSERT((l = _api.sessions().getSessions()).contains(s1->getId()) && l.contains(s2->getId()));
        ASSERT((l = _api.sessions().getSessions(a.getId())).contains(s1->getId()) && l.contains(s2->getId()));
        s1->setAccount();
        ASSERT((l = _api.sessions().getSessions(a.getId())).contains(s2->getId()) && !l.contains(s1->getId()));
        ASSERT((l = _api.sessions().getSessions("", "client5")).contains(s2->getId()) && !l.contains(s1->getId()));
        ASSERT((l = _api.sessions().getSessions("", "client3")).contains(s2->getId()) && l.contains(s1->getId()));
        ASSERT((l = _api.sessions().getSessions(a.getId(), "client3")).contains(s2->getId()) && !l.contains(s1->getId()));
        ASSERT(_api.sessions().getSessions(a.getId(), "client").isEmpty());
        ASSERT(s2->setExpiration(s1->getExpiration().addSecs(10)));
        ASSERT(!s2->isExpired());
        ASSERT(s2->setExpiration());
        ASSERT(!s2->isExpired());
        ASSERT(s2->setExpiration(QDateTime::currentDateTime().addSecs(-10)));
        ASSERT(s2->isExpired());
        ASSERT(_api.sessions().destroy(s1->getId()));
        ASSERT(s1->isExpired());
        ASSERT(a.remove());
    }
    catch (unsigned int line)
    {
        _log.trace("Unit tests of the sessions failed!", Properties("line", line).toMap(), "Database", "_sessions");
        throw line;
    }
    _log.trace("Unit tests of the sessions successful!", "Database", "_sessions");
}

void            Database::_tags()
{
    LightBird::TableTags t;
    LightBird::TableDirectories d1;
    LightBird::TableDirectories d2;
    QSqlQuery   query(_database.getDatabase());

    _log.trace("Running unit tests of the tags...", "Database", "_tags");
    query.prepare("DELETE FROM tags WHERE name IN('t1', 't2')");
    _database.query(query);
    query.prepare("DELETE FROM directories WHERE name IN('d1', 'd2')");
    _database.query(query);
    try
    {
        ASSERT(d1.add("d1"));
        ASSERT(d2.add("d2"));
        ASSERT(t.add(d2.getId(), "t1"));
        ASSERT(d2.getTags().size() == 1);
        ASSERT(d2.getTags().contains(t.getId()));
        ASSERT(t.getIdObject() == d2.getId());
        ASSERT(t.setIdObject(d1.getId()));
        ASSERT(t.getIdObject() == d1.getId());
        ASSERT(t.getName() == "t1");
        ASSERT(t.setName("t2"));
        ASSERT(t.getName() == "t2");
        ASSERT(d2.remove());
        ASSERT(t.exists());
        ASSERT(d1.remove());
        ASSERT(!t.exists());
    }
    catch (unsigned int line)
    {
        _log.trace("Unit tests of the tags failed!", Properties("line", line).toMap(), "Database", "_tags");
        throw line;
    }
    _log.trace("Unit tests of the tags successful!", "Database", "_tags");
}
