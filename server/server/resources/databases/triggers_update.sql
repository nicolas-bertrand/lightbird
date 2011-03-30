-- Update the field modified of an updated row
CREATE TRIGGER "update_accounts" AFTER UPDATE ON accounts
BEGIN
    SELECT RAISE(ROLLBACK, '') WHERE NEW.name='' OR NEW.administrator NOT IN (0, 1) OR NEW.active NOT IN (0, 1);
    UPDATE accounts SET modified = (STRFTIME('%Y-%m-%d %H:%M:%S','now', 'localtime')) WHERE new.modified != (STRFTIME('%Y-%m-%d %H:%M:%S','now', 'localtime')) AND rowid = new.rowid;
END;

CREATE TRIGGER "update_accounts_groups" AFTER UPDATE ON accounts_groups
BEGIN
    UPDATE accounts_groups SET modified = (STRFTIME('%Y-%m-%d %H:%M:%S','now', 'localtime')) WHERE new.modified != (STRFTIME('%Y-%m-%d %H:%M:%S','now', 'localtime')) AND rowid = new.rowid;
END;

CREATE TRIGGER "update_accounts_informations" AFTER UPDATE ON accounts_informations
BEGIN
    SELECT RAISE(ROLLBACK, '') WHERE NEW.name='' OR NEW.value='';
    UPDATE accounts_informations SET modified = (STRFTIME('%Y-%m-%d %H:%M:%S','now', 'localtime')) WHERE new.modified != (STRFTIME('%Y-%m-%d %H:%M:%S','now', 'localtime')) AND rowid = new.rowid;
END;

CREATE TRIGGER "update_collections" AFTER UPDATE ON collections
BEGIN
    SELECT RAISE(ROLLBACK, '') WHERE NEW.name='';
    UPDATE collections SET modified = (STRFTIME('%Y-%m-%d %H:%M:%S','now', 'localtime')) WHERE new.modified != (STRFTIME('%Y-%m-%d %H:%M:%S','now', 'localtime')) AND rowid = new.rowid;
END;

CREATE TRIGGER "update_directories" AFTER UPDATE ON directories
BEGIN
    SELECT RAISE(ROLLBACK, '') WHERE NEW.name='';
    UPDATE directories SET modified = (STRFTIME('%Y-%m-%d %H:%M:%S','now', 'localtime')) WHERE new.modified != (STRFTIME('%Y-%m-%d %H:%M:%S','now', 'localtime')) AND rowid = new.rowid;
END;

CREATE TRIGGER "update_events" AFTER UPDATE ON events
BEGIN
    SELECT RAISE(ROLLBACK, '') WHERE NEW.name='';
    UPDATE events SET modified = (STRFTIME('%Y-%m-%d %H:%M:%S','now', 'localtime')) WHERE new.modified != (STRFTIME('%Y-%m-%d %H:%M:%S','now', 'localtime')) AND rowid = new.rowid;
END;

CREATE TRIGGER "update_events_informations" AFTER UPDATE ON events_informations
BEGIN
    SELECT RAISE(ROLLBACK, '') WHERE NEW.name='' OR NEW.value='';
    UPDATE events_informations SET modified = (STRFTIME('%Y-%m-%d %H:%M:%S','now', 'localtime')) WHERE new.modified != (STRFTIME('%Y-%m-%d %H:%M:%S','now', 'localtime')) AND rowid = new.rowid;
END;

CREATE TRIGGER "update_files" AFTER UPDATE ON files
BEGIN
    SELECT RAISE(ROLLBACK, '') WHERE NEW.name='' OR NEW.path='';
    UPDATE files SET modified = (STRFTIME('%Y-%m-%d %H:%M:%S','now', 'localtime')) WHERE new.modified != (STRFTIME('%Y-%m-%d %H:%M:%S','now', 'localtime')) AND rowid = new.rowid;
END;

CREATE TRIGGER "update_files_collections" AFTER UPDATE ON files_collections
BEGIN
    UPDATE files_collections SET modified = (STRFTIME('%Y-%m-%d %H:%M:%S','now', 'localtime')) WHERE new.modified != (STRFTIME('%Y-%m-%d %H:%M:%S','now', 'localtime')) AND rowid = new.rowid;
END;

CREATE TRIGGER "update_files_informations" AFTER UPDATE ON files_informations
BEGIN
    SELECT RAISE(ROLLBACK, '') WHERE NEW.name='' OR NEW.value='';
    UPDATE files_informations SET modified = (STRFTIME('%Y-%m-%d %H:%M:%S','now', 'localtime')) WHERE new.modified != (STRFTIME('%Y-%m-%d %H:%M:%S','now', 'localtime')) AND rowid = new.rowid;
END;

CREATE TRIGGER "update_groups" AFTER UPDATE ON groups
BEGIN
    SELECT RAISE(ROLLBACK, '') WHERE NEW.name='';
    UPDATE groups SET modified = (STRFTIME('%Y-%m-%d %H:%M:%S','now', 'localtime')) WHERE new.modified != (STRFTIME('%Y-%m-%d %H:%M:%S','now', 'localtime')) AND rowid = new.rowid;
END;

CREATE TRIGGER "update_limits" AFTER UPDATE ON limits
BEGIN
    SELECT RAISE(ROLLBACK, '') WHERE NEW.name='' OR NEW.value='';
    UPDATE limits SET modified = (STRFTIME('%Y-%m-%d %H:%M:%S','now', 'localtime')) WHERE new.modified != (STRFTIME('%Y-%m-%d %H:%M:%S','now', 'localtime')) AND rowid = new.rowid;
END;

CREATE TRIGGER "update_permissions" AFTER UPDATE ON permissions
BEGIN
    SELECT RAISE(ROLLBACK, '') WHERE NEW.granted NOT IN (0, 1);
    UPDATE permissions SET modified = (STRFTIME('%Y-%m-%d %H:%M:%S','now', 'localtime')) WHERE new.modified != (STRFTIME('%Y-%m-%d %H:%M:%S','now', 'localtime')) AND rowid = new.rowid;
END;

CREATE TRIGGER "update_tags" AFTER UPDATE ON tags
BEGIN
    SELECT RAISE(ROLLBACK, '') WHERE NEW.name='';
    UPDATE tags SET modified = (STRFTIME('%Y-%m-%d %H:%M:%S','now', 'localtime')) WHERE new.modified != (STRFTIME('%Y-%m-%d %H:%M:%S','now', 'localtime')) AND rowid = new.rowid;
END;
