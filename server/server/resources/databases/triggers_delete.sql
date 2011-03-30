CREATE TRIGGER "delete_accounts" BEFORE DELETE ON accounts
BEGIN
	INSERT INTO deleted ('table', 'id') VALUES ('accounts', old.id);
END;

CREATE TRIGGER "delete_accounts_groups" BEFORE DELETE ON accounts_groups
BEGIN
	INSERT INTO deleted ('table', 'id') VALUES ('accounts_groups', old.id);
END;

CREATE TRIGGER "delete_accounts_informations" BEFORE DELETE ON accounts_informations
BEGIN
	INSERT INTO deleted ('table', 'id') VALUES ('accounts_informations', old.id);
END;

CREATE TRIGGER "delete_collections" BEFORE DELETE ON collections
BEGIN
	INSERT INTO deleted ('table', 'id') VALUES ('collections', old.id);
END;

CREATE TRIGGER "delete_directories" BEFORE DELETE ON directories
BEGIN
	INSERT INTO deleted ('table', 'id') VALUES ('directories', old.id);
END;

CREATE TRIGGER "delete_events" BEFORE DELETE ON events
BEGIN
	INSERT INTO deleted ('table', 'id') VALUES ('events', old.id);
END;

CREATE TRIGGER "delete_events_informations" BEFORE DELETE ON events_informations
BEGIN
	INSERT INTO deleted ('table', 'id') VALUES ('events_informations', old.id);
END;

CREATE TRIGGER "delete_files" BEFORE DELETE ON files
BEGIN
	INSERT INTO deleted ('table', 'id') VALUES ('files', old.id);
END;

CREATE TRIGGER "delete_files_collections" BEFORE DELETE ON files_collections
BEGIN
	INSERT INTO deleted ('table', 'id') VALUES ('files_collections', old.id);
END;

CREATE TRIGGER "delete_files_informations" BEFORE DELETE ON files_informations
BEGIN
	INSERT INTO deleted ('table', 'id') VALUES ('files_informations', old.id);
END;

CREATE TRIGGER "delete_groups" BEFORE DELETE ON groups
BEGIN
	INSERT INTO deleted ('table', 'id') VALUES ('groups', old.id);
END;

CREATE TRIGGER "delete_limits" BEFORE DELETE ON limits
BEGIN
	INSERT INTO deleted ('table', 'id') VALUES ('limits', old.id);
END;

CREATE TRIGGER "delete_permissions" BEFORE DELETE ON permissions
BEGIN
	INSERT INTO deleted ('table', 'id') VALUES ('permissions', old.id);
END;

CREATE TRIGGER "delete_tags" BEFORE DELETE ON tags
BEGIN
	INSERT INTO deleted ('table', 'id') VALUES ('tags', old.id);
END;
