-- Ces triggers simulent les foreign key de SQLite qui sont disponible a partir de la version 3.6.19

-----------------------------
-- Accounts
-----------------------------
CREATE TRIGGER "fk_delete_accounts" BEFORE DELETE ON accounts
BEGIN
    DELETE FROM accounts_informations WHERE id_account = old.id;
    DELETE FROM accounts_groups WHERE id_account = old.id;
    DELETE FROM limits WHERE id_accessor = old.id;
    DELETE FROM events WHERE id_accessor = old.id;
    DELETE FROM permissions WHERE id_accessor = old.id;
    DELETE FROM directories WHERE id_account = old.id;
    DELETE FROM collections WHERE id_account = old.id;
    DELETE FROM files WHERE id_account = old.id;
END;

CREATE TRIGGER "fk_update_accounts" BEFORE UPDATE ON accounts
BEGIN
    SELECT RAISE(ROLLBACK, '') WHERE NEW."id" != OLD."id";
END;

-----------------------------
-- Accounts_groups
-----------------------------
CREATE TRIGGER "fk_insert_accounts_groups" BEFORE INSERT ON accounts_groups
BEGIN
    SELECT RAISE(ROLLBACK, '') WHERE (SELECT "id" FROM "accounts" WHERE "id" = NEW."id_account") IS NULL;
    SELECT RAISE(ROLLBACK, '') WHERE (SELECT "id" FROM "groups" WHERE "id" = NEW."id_group") IS NULL;
END;

CREATE TRIGGER "fk_update_accounts_groups" BEFORE UPDATE ON accounts_groups
BEGIN
    SELECT RAISE(ROLLBACK, '') WHERE NEW."id" != OLD."id";
END;

-----------------------------
-- Accounts_informations
-----------------------------
CREATE TRIGGER "fk_insert_accounts_informations" BEFORE INSERT ON accounts_informations
BEGIN
    SELECT RAISE(ROLLBACK, 'insert | accounts_informations | id_account') WHERE (SELECT "id" FROM "accounts" WHERE "id" = NEW."id_account") IS NULL;
END;

CREATE TRIGGER "fk_update_accounts_informations" BEFORE UPDATE ON accounts_informations
BEGIN
    SELECT RAISE(ROLLBACK, '') WHERE NEW."id" != OLD."id";
    SELECT RAISE(ROLLBACK, '') WHERE (SELECT "id" FROM "accounts" WHERE "id" = NEW."id_account") IS NULL;
END;

-----------------------------
-- Collections
-----------------------------
CREATE TRIGGER "fk_delete_collections" BEFORE DELETE ON collections
BEGIN
    DELETE FROM files_collections WHERE id_collection = old.id;
    DELETE FROM tags WHERE id_object = old.id;
    DELETE FROM permissions WHERE id_object = old.id;
    DELETE FROM events WHERE id_object = old.id;
    DELETE FROM collections WHERE id_collection = old.id;
END;

CREATE TRIGGER "fk_insert_collections" BEFORE INSERT ON collections
BEGIN
    SELECT RAISE(ROLLBACK, 'insert | collections | id_collection') WHERE NEW."id_collection" != "" AND (SELECT "id" FROM "collections" WHERE "id" = NEW."id_collection") IS NULL;
    SELECT RAISE(ROLLBACK, 'insert | collections | id_account') WHERE NEW."id_account" != "" AND (SELECT "id" FROM "accounts" WHERE "id" = NEW."id_account") IS NULL;
END;

CREATE TRIGGER "fk_update_collections" BEFORE UPDATE ON collections
BEGIN
    SELECT RAISE(ROLLBACK, '') WHERE NEW."id" != OLD."id";
    SELECT RAISE(ROLLBACK, 'update | collections | id_collection') WHERE NEW."id_collection" != "" AND (SELECT "id" FROM "collections" WHERE "id" = NEW."id_collection") IS NULL;
    SELECT RAISE(ROLLBACK, 'update | collections | id_account') WHERE NEW."id_account" != "" AND (SELECT "id" FROM "accounts" WHERE "id" = NEW."id_account") IS NULL;
END;

-----------------------------
-- Directories
-----------------------------
CREATE TRIGGER "fk_delete_directories" BEFORE DELETE ON directories
BEGIN
    DELETE FROM files WHERE id_directory = old.id;
    DELETE FROM tags WHERE id_object = old.id;
    DELETE FROM permissions WHERE id_object = old.id;
    DELETE FROM events WHERE id_object = old.id;
    DELETE FROM directories WHERE id_directory = old.id;
END;

CREATE TRIGGER "fk_insert_directories" BEFORE INSERT ON directories
BEGIN
    SELECT RAISE(ROLLBACK, 'insert | directories | id_directory') WHERE NEW."id_directory" != "" AND (SELECT "id" FROM "directories" WHERE "id" = NEW."id_directory") IS NULL;
    SELECT RAISE(ROLLBACK, 'insert | directories | id_account') WHERE NEW."id_account" != "" AND (SELECT "id" FROM "accounts" WHERE "id" = NEW."id_account") IS NULL;
END;

CREATE TRIGGER "fk_update_directories" BEFORE UPDATE ON directories
BEGIN
    SELECT RAISE(ROLLBACK, '') WHERE NEW."id" != OLD."id";
    SELECT RAISE(ROLLBACK, 'update | directories | id_directory') WHERE NEW."id_directory" != "" AND (SELECT "id" FROM "directories" WHERE "id" = NEW."id_directory") IS NULL;
    SELECT RAISE(ROLLBACK, 'update | directories | id_account') WHERE NEW."id_account" != "" AND (SELECT "id" FROM "accounts" WHERE "id" = NEW."id_account") IS NULL;
END;

-----------------------------
-- Events
-----------------------------
CREATE TRIGGER "fk_delete_events" BEFORE DELETE ON events
BEGIN
    DELETE FROM events_informations WHERE id_event = old.id;
END;

CREATE TRIGGER "fk_insert_events" BEFORE INSERT ON events
BEGIN
    SELECT RAISE(ROLLBACK, 'insert | events | id_accessor') WHERE
        NEW."id_accessor" != "" AND
        (SELECT "id" FROM "accounts" WHERE "id" = NEW."id_accessor") IS NULL AND
        (SELECT "id" FROM "groups" WHERE "id" = NEW."id_accessor") IS NULL;
    SELECT RAISE(ROLLBACK, 'insert | events | id_object') WHERE
        NEW."id_object" != "" AND
        (SELECT "id" FROM "files" WHERE "id" = NEW."id_object") IS NULL AND
        (SELECT "id" FROM "directories" WHERE "id" = NEW."id_object") IS NULL AND
        (SELECT "id" FROM "collections" WHERE "id" = NEW."id_object") IS NULL;
END;

CREATE TRIGGER "fk_update_events" BEFORE UPDATE ON events
BEGIN
    SELECT RAISE(ROLLBACK, '') WHERE NEW."id" != OLD."id";
    SELECT RAISE(ROLLBACK, 'update | events | id_accessor') WHERE
        NEW."id_accessor" != "" AND
        (SELECT "id" FROM "accounts" WHERE "id" = NEW."id_accessor") IS NULL AND
        (SELECT "id" FROM "groups" WHERE "id" = NEW."id_accessor") IS NULL;
    SELECT RAISE(ROLLBACK, 'update | events | id_object') WHERE
        NEW."id_object" != "" AND
        (SELECT "id" FROM "files" WHERE "id" = NEW."id_object") IS NULL AND
        (SELECT "id" FROM "directories" WHERE "id" = NEW."id_object") IS NULL AND
        (SELECT "id" FROM "collections" WHERE "id" = NEW."id_object") IS NULL;
END;

-----------------------------
-- Events_informations
-----------------------------
CREATE TRIGGER "fk_insert_events_informations" BEFORE INSERT ON events_informations
BEGIN
    SELECT RAISE(ROLLBACK, 'insert | events_informations | id_event') WHERE (SELECT "id" FROM "events" WHERE "id" = NEW."id_event") IS NULL;
END;

CREATE TRIGGER "fk_update_events_informations" BEFORE UPDATE ON events_informations
BEGIN
    SELECT RAISE(ROLLBACK, '') WHERE NEW."id" != OLD."id";
    SELECT RAISE(ROLLBACK, 'update | events_informations | id_event') WHERE (SELECT "id" FROM "events" WHERE "id" = NEW."id_event") IS NULL;
END;

-----------------------------
-- Files
-----------------------------
CREATE TRIGGER "fk_delete_files" BEFORE DELETE ON files
BEGIN
    DELETE FROM files_informations WHERE id_file = old.id;
    DELETE FROM files_collections WHERE id_file = old.id;
    DELETE FROM tags WHERE id_object = old.id;
    DELETE FROM permissions	WHERE id_object = old.id;
    DELETE FROM events WHERE id_object = old.id;
END;

CREATE TRIGGER "fk_insert_files" BEFORE INSERT ON files
BEGIN
    SELECT RAISE(ROLLBACK, 'insert | files | id_directory') WHERE NEW."id_directory" != "" AND (SELECT "id" FROM "directories" WHERE "id" = NEW."id_directory") IS NULL;
    SELECT RAISE(ROLLBACK, 'insert | files | id_account') WHERE NEW."id_account" != "" AND (SELECT "id" FROM "accounts" WHERE "id" = NEW."id_account") IS NULL;
END;

CREATE TRIGGER "fk_update_files" BEFORE UPDATE ON files
BEGIN
    SELECT RAISE(ROLLBACK, '') WHERE NEW."id" != OLD."id";
    SELECT RAISE(ROLLBACK, 'update | files | id_directory') WHERE NEW."id_directory" != "" AND (SELECT "id" FROM "directories" WHERE "id" = NEW."id_directory") IS NULL;
    SELECT RAISE(ROLLBACK, 'update | files | id_account') WHERE NEW."id_account" != "" AND (SELECT "id" FROM "accounts" WHERE "id" = NEW."id_account") IS NULL;
END;

-----------------------------
-- Files_collections
-----------------------------
CREATE TRIGGER "fk_insert_files_collections" BEFORE INSERT ON files_collections
BEGIN
    SELECT RAISE(ROLLBACK, 'insert | files_collections | id_file') WHERE (SELECT "id" FROM "files" WHERE "id" = NEW."id_file") IS NULL;
    SELECT RAISE(ROLLBACK, 'insert | files_collections | id_collection') WHERE (SELECT "id" FROM "collections" WHERE "id" = NEW."id_collection") IS NULL;
END;

CREATE TRIGGER "fk_update_files_collections" BEFORE UPDATE ON files_collections
BEGIN
    SELECT RAISE(ROLLBACK, '') WHERE NEW."id" != OLD."id";
    SELECT RAISE(ROLLBACK, 'update | files_collections | id_file') WHERE (SELECT "id" FROM "files" WHERE "id" = NEW."id_file") IS NULL;
    SELECT RAISE(ROLLBACK, 'update | files_collections | id_collection') WHERE (SELECT "id" FROM "collections" WHERE "id" = NEW."id_collection") IS NULL;
END;

-----------------------------
-- Files_informations
-----------------------------
CREATE TRIGGER "fk_insert_files_informations" BEFORE INSERT ON files_informations
BEGIN
    SELECT RAISE(ROLLBACK, 'insert | files_informations | id_file') WHERE (SELECT "id" FROM "files" WHERE "id" = NEW."id_file") IS NULL;
END;

CREATE TRIGGER "fk_update_files_informations" BEFORE UPDATE ON files_informations
BEGIN
    SELECT RAISE(ROLLBACK, '') WHERE NEW."id" != OLD."id";
    SELECT RAISE(ROLLBACK, 'update | files_informations | id_file') WHERE (SELECT "id" FROM "files" WHERE "id" = NEW."id_file") IS NULL;
END;

-----------------------------
-- Groups
-----------------------------
CREATE TRIGGER "fk_delete_groups" BEFORE DELETE ON groups
BEGIN
    DELETE FROM accounts_groups WHERE id_group = old.id;
    DELETE FROM limits WHERE id_accessor = old.id;
    DELETE FROM permissions WHERE id_accessor = old.id;
    DELETE FROM events WHERE id_accessor = old.id;
    DELETE FROM groups WHERE id_group = old.id;
END;

CREATE TRIGGER "fk_insert_groups" BEFORE INSERT ON groups
BEGIN
    SELECT RAISE(ROLLBACK, 'insert | groups | id_group') WHERE NEW."id_group" != "" AND (SELECT "id" FROM "groups" WHERE "id" = NEW."id_group") IS NULL;
END;

CREATE TRIGGER "fk_update_groups" BEFORE UPDATE ON groups
BEGIN
    SELECT RAISE(ROLLBACK, '') WHERE NEW."id" != OLD."id";
    SELECT RAISE(ROLLBACK, 'update | groups | id_group') WHERE NEW."id_group" != "" AND (SELECT "id" FROM "groups" WHERE "id" = NEW."id_group") IS NULL;
END;

-----------------------------
-- Limits
-----------------------------
CREATE TRIGGER "fk_insert_limits" BEFORE INSERT ON limits
BEGIN
    SELECT RAISE(ROLLBACK, 'insert | limits | id_accessor') WHERE
        NEW."id_accessor" != "" AND
        (SELECT "id" FROM "accounts" WHERE "id" = NEW."id_accessor") IS NULL AND
        (SELECT "id" FROM "groups" WHERE "id" = NEW."id_accessor") IS NULL;
END;

CREATE TRIGGER "fk_update_limits" BEFORE UPDATE ON limits
BEGIN
    SELECT RAISE(ROLLBACK, '') WHERE NEW."id" != OLD."id";
    SELECT RAISE(ROLLBACK, 'update | limits | id_accessor') WHERE
        NEW."id_accessor" != "" AND
        (SELECT "id" FROM "accounts" WHERE "id" = NEW."id_accessor") IS NULL AND
        (SELECT "id" FROM "groups" WHERE "id" = NEW."id_accessor") IS NULL;
END;

-----------------------------
-- Permissions
-----------------------------
CREATE TRIGGER "fk_insert_permissions" BEFORE INSERT ON permissions
BEGIN
    SELECT RAISE(ROLLBACK, 'insert | permissions | id_accessor') WHERE
        NEW."id_accessor" != "" AND
        (SELECT "id" FROM "accounts" WHERE "id" = NEW."id_accessor") IS NULL AND
        (SELECT "id" FROM "groups" WHERE "id" = NEW."id_accessor") IS NULL;
    SELECT RAISE(ROLLBACK, 'insert | permissions | id_object') WHERE
        NEW."id_object" != "" AND
        (SELECT "id" FROM "files" WHERE "id" = NEW."id_object") IS NULL AND
        (SELECT "id" FROM "directories" WHERE "id" = NEW."id_object") IS NULL AND
        (SELECT "id" FROM "collections" WHERE "id" = NEW."id_object") IS NULL;
END;

CREATE TRIGGER "fk_update_permissions" BEFORE UPDATE ON permissions
BEGIN
    SELECT RAISE(ROLLBACK, '') WHERE NEW."id" != OLD."id";
    SELECT RAISE(ROLLBACK, 'update | permissions | id_accessor') WHERE
        NEW."id_accessor" != "" AND
        (SELECT "id" FROM "accounts" WHERE "id" = NEW."id_accessor") IS NULL AND
        (SELECT "id" FROM "groups" WHERE "id" = NEW."id_accessor") IS NULL;
    SELECT RAISE(ROLLBACK, 'update | permissions | id_object') WHERE
        NEW."id_object" != "" AND
        (SELECT "id" FROM "files" WHERE "id" = NEW."id_object") IS NULL AND
        (SELECT "id" FROM "directories" WHERE "id" = NEW."id_object") IS NULL AND
        (SELECT "id" FROM "collections" WHERE "id" = NEW."id_object") IS NULL;
END;

-----------------------------
-- Tags
-----------------------------
CREATE TRIGGER "fk_insert_tags" BEFORE INSERT ON tags
BEGIN
    SELECT RAISE(ROLLBACK, 'insert | tags | id_object') WHERE
        (SELECT "id" FROM "files" WHERE "id" = NEW."id_object") IS NULL AND
        (SELECT "id" FROM "directories" WHERE "id" = NEW."id_object") IS NULL AND
        (SELECT "id" FROM "collections" WHERE "id" = NEW."id_object") IS NULL;
END;

CREATE TRIGGER "fk_update_tags" BEFORE UPDATE ON tags
BEGIN
    SELECT RAISE(ROLLBACK, '') WHERE NEW."id" != OLD."id";
    SELECT RAISE(ROLLBACK, 'update | tags | id_object') WHERE
        (SELECT "id" FROM "files" WHERE "id" = NEW."id_object") IS NULL AND
        (SELECT "id" FROM "directories" WHERE "id" = NEW."id_object") IS NULL AND
        (SELECT "id" FROM "collections" WHERE "id" = NEW."id_object") IS NULL;
END;
