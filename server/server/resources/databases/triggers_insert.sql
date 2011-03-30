-- Update the fields created and modified of an inserted row
CREATE TRIGGER "insert_accounts" AFTER INSERT ON accounts
BEGIN
    SELECT RAISE(ROLLBACK, '') WHERE NEW.name='' OR NEW.administrator NOT IN (0, 1) OR NEW.active NOT IN (0, 1);
    UPDATE accounts SET modified = (STRFTIME('%Y-%m-%d %H:%M:%S','now', 'localtime')),  created = (STRFTIME('%Y-%m-%d %H:%M:%S','now', 'localtime')) WHERE rowid = new.rowid;
END;

CREATE TRIGGER "insert_accounts_groups" AFTER INSERT ON accounts_groups
BEGIN
    UPDATE accounts_groups SET modified = (STRFTIME('%Y-%m-%d %H:%M:%S','now', 'localtime')),  created = (STRFTIME('%Y-%m-%d %H:%M:%S','now', 'localtime')) WHERE rowid = new.rowid;
END;

CREATE TRIGGER "insert_accounts_informations" AFTER INSERT ON accounts_informations
BEGIN
    SELECT RAISE(ROLLBACK, '') WHERE NEW.name='' OR NEW.value='';
    UPDATE accounts_informations SET modified = (STRFTIME('%Y-%m-%d %H:%M:%S','now', 'localtime')),  created = (STRFTIME('%Y-%m-%d %H:%M:%S','now', 'localtime')) WHERE rowid = new.rowid;
END;

CREATE TRIGGER "insert_collections" AFTER INSERT ON collections
BEGIN
    SELECT RAISE(ROLLBACK, '') WHERE NEW.name='';
    UPDATE collections SET modified = (STRFTIME('%Y-%m-%d %H:%M:%S','now', 'localtime')),  created = (STRFTIME('%Y-%m-%d %H:%M:%S','now', 'localtime')) WHERE rowid = new.rowid;
END;

CREATE TRIGGER "insert_directories" AFTER INSERT ON directories
BEGIN
    SELECT RAISE(ROLLBACK, '') WHERE NEW.name='';
    UPDATE directories SET modified = (STRFTIME('%Y-%m-%d %H:%M:%S','now', 'localtime')),  created = (STRFTIME('%Y-%m-%d %H:%M:%S','now', 'localtime')) WHERE rowid = new.rowid;
END;

CREATE TRIGGER "insert_events" AFTER INSERT ON events
BEGIN
    SELECT RAISE(ROLLBACK, '') WHERE NEW.name='';
    UPDATE events SET modified = (STRFTIME('%Y-%m-%d %H:%M:%S','now', 'localtime')),  created = (STRFTIME('%Y-%m-%d %H:%M:%S','now', 'localtime')) WHERE rowid = new.rowid;
END;

CREATE TRIGGER "insert_events_informations" AFTER INSERT ON events_informations
BEGIN
    SELECT RAISE(ROLLBACK, '') WHERE NEW.name='' OR NEW.value='';
    UPDATE events_informations SET modified = (STRFTIME('%Y-%m-%d %H:%M:%S','now', 'localtime')),  created = (STRFTIME('%Y-%m-%d %H:%M:%S','now', 'localtime')) WHERE rowid = new.rowid;
END;

CREATE TRIGGER "insert_files" AFTER INSERT ON files
BEGIN
    SELECT RAISE(ROLLBACK, '') WHERE NEW.name='' OR NEW.path='';
    UPDATE files SET modified = (STRFTIME('%Y-%m-%d %H:%M:%S','now', 'localtime')),  created = (STRFTIME('%Y-%m-%d %H:%M:%S','now', 'localtime')) WHERE rowid = new.rowid;
END;

CREATE TRIGGER "insert_files_collections" AFTER INSERT ON files_collections
BEGIN
    UPDATE files_collections SET modified = (STRFTIME('%Y-%m-%d %H:%M:%S','now', 'localtime')),  created = (STRFTIME('%Y-%m-%d %H:%M:%S','now', 'localtime')) WHERE rowid = new.rowid;
END;

CREATE TRIGGER "insert_files_informations" AFTER INSERT ON files_informations
BEGIN
    SELECT RAISE(ROLLBACK, '') WHERE NEW.name='' OR NEW.value='';
    UPDATE files_informations SET modified = (STRFTIME('%Y-%m-%d %H:%M:%S','now', 'localtime')),  created = (STRFTIME('%Y-%m-%d %H:%M:%S','now', 'localtime')) WHERE rowid = new.rowid;
END;

CREATE TRIGGER "insert_groups" AFTER INSERT ON groups
BEGIN
    SELECT RAISE(ROLLBACK, '') WHERE NEW.name='';
    UPDATE groups SET modified = (STRFTIME('%Y-%m-%d %H:%M:%S','now', 'localtime')),  created = (STRFTIME('%Y-%m-%d %H:%M:%S','now', 'localtime')) WHERE rowid = new.rowid;
END;

CREATE TRIGGER "insert_limits" AFTER INSERT ON limits
BEGIN
    SELECT RAISE(ROLLBACK, '') WHERE NEW.name='' OR NEW.value='';
    UPDATE limits SET modified = (STRFTIME('%Y-%m-%d %H:%M:%S','now', 'localtime')),  created = (STRFTIME('%Y-%m-%d %H:%M:%S','now', 'localtime')) WHERE rowid = new.rowid;
END;

CREATE TRIGGER "insert_permissions" AFTER INSERT ON permissions
BEGIN
    SELECT RAISE(ROLLBACK, '') WHERE NEW.granted NOT IN (0, 1);
    UPDATE permissions SET modified = (STRFTIME('%Y-%m-%d %H:%M:%S','now', 'localtime')),  created = (STRFTIME('%Y-%m-%d %H:%M:%S','now', 'localtime')) WHERE rowid = new.rowid;
END;

CREATE TRIGGER "insert_tags" AFTER INSERT ON tags
BEGIN
    SELECT RAISE(ROLLBACK, '') WHERE NEW.name='';
    UPDATE tags SET modified = (STRFTIME('%Y-%m-%d %H:%M:%S','now', 'localtime')),  created = (STRFTIME('%Y-%m-%d %H:%M:%S','now', 'localtime')) WHERE rowid = new.rowid;
END;
