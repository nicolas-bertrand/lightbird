CREATE TABLE accounts (
  "id" VARCHAR(36) NOT NULL,
  "name" VARCHAR(255) NOT NULL,
  "password" VARCHAR(255) DEFAULT "" NOT NULL,
  "administrator" BOOLEAN DEFAULT 0 NOT NULL,
  "active" BOOLEAN DEFAULT 1 NOT NULL,
  "modified" DATETIME DEFAULT (STRFTIME('%Y-%m-%d %H:%M:%S', 'now', 'localtime')) NOT NULL,
  "created" DATETIME DEFAULT (STRFTIME('%Y-%m-%d %H:%M:%S', 'now', 'localtime')) NOT NULL,
  PRIMARY KEY(id),
  UNIQUE(name)
);
CREATE TABLE accounts_groups (
  "id" VARCHAR(36) NOT NULL,
  "id_account" VARCHAR(36) NOT NULL,
  "id_group" VARCHAR(36) NOT NULL,
  "modified" DATETIME DEFAULT (STRFTIME('%Y-%m-%d %H:%M:%S', 'now', 'localtime')) NOT NULL,
  "created" DATETIME DEFAULT (STRFTIME('%Y-%m-%d %H:%M:%S', 'now', 'localtime')) NOT NULL,
  PRIMARY KEY(id),
  UNIQUE(id_account, id_group)
);
CREATE TABLE accounts_informations (
  "id" VARCHAR(36) NOT NULL,
  "id_account" VARCHAR(36) NOT NULL,
  "name" VARCHAR(255) NOT NULL,
  "value" VARCHAR(255) NOT NULL,
  "modified" DATETIME DEFAULT (STRFTIME('%Y-%m-%d %H:%M:%S', 'now', 'localtime')) NOT NULL,
  "created" DATETIME DEFAULT (STRFTIME('%Y-%m-%d %H:%M:%S', 'now', 'localtime')) NOT NULL,
  PRIMARY KEY(id)
);
CREATE TABLE collections (
  "id" VARCHAR(36) NOT NULL,
  "name" VARCHAR(255) NOT NULL,
  "id_collection" VARCHAR(36) DEFAULT "" NOT NULL,
  "id_account" VARCHAR(36) DEFAULT "" NOT NULL,
  "modified" DATETIME DEFAULT (STRFTIME('%Y-%m-%d %H:%M:%S', 'now', 'localtime')) NOT NULL,
  "created" DATETIME DEFAULT (STRFTIME('%Y-%m-%d %H:%M:%S', 'now', 'localtime')) NOT NULL,
  PRIMARY KEY(id)
);
CREATE TABLE deleted (
  "table" VARCHAR(255) NOT NULL,
  "id" VARCHAR(255) NOT NULL,
  "date" DATETIME DEFAULT (STRFTIME('%Y-%m-%d %H:%M:%S', 'now', 'localtime')) NOT NULL,
  UNIQUE(id)
 );
CREATE TABLE directories (
  "id" VARCHAR(36) NOT NULL,
  "name" VARCHAR(255) NOT NULL,
  "id_directory" VARCHAR(36) DEFAULT "" NOT NULL,
  "id_account" VARCHAR(36) DEFAULT "" NOT NULL,
  "modified" DATETIME DEFAULT (STRFTIME('%Y-%m-%d %H:%M:%S', 'now', 'localtime')) NOT NULL,
  "created" DATETIME DEFAULT (STRFTIME('%Y-%m-%d %H:%M:%S', 'now', 'localtime')) NOT NULL,
  PRIMARY KEY(id),
  UNIQUE(name, id_directory)
);
CREATE TABLE events (
  "id" VARCHAR(36) NOT NULL,
  "name" VARCHAR(255) NOT NULL,
  "id_accessor" VARCHAR(36) DEFAULT "" NOT NULL,
  "id_object" VARCHAR(36) DEFAULT "" NOT NULL,
  "modified" DATETIME DEFAULT (STRFTIME('%Y-%m-%d %H:%M:%S', 'now', 'localtime')) NOT NULL,
  "created" DATETIME DEFAULT (STRFTIME('%Y-%m-%d %H:%M:%S', 'now', 'localtime')) NOT NULL,
  PRIMARY KEY(id)
);
CREATE TABLE events_informations (
  "id" VARCHAR(36) NOT NULL,
  "id_event" VARCHAR(36) NOT NULL,
  "name" VARCHAR(255) NOT NULL,
  "value" VARCHAR(255) NOT NULL,
  "modified" DATETIME DEFAULT (STRFTIME('%Y-%m-%d %H:%M:%S', 'now', 'localtime')) NOT NULL,
  "created" DATETIME DEFAULT (STRFTIME('%Y-%m-%d %H:%M:%S', 'now', 'localtime')) NOT NULL,
  PRIMARY KEY(id)
);
CREATE TABLE files (
  "id" VARCHAR(36) NOT NULL,
  "name" VARCHAR(255) NOT NULL,
  "path" TEXT NOT NULL,
  "type" VARCHAR(255) DEFAULT "" NOT NULL,
  "id_directory" VARCHAR(36) DEFAULT "" NOT NULL,
  "id_account" VARCHAR(36) DEFAULT "" NOT NULL,
  "modified" DATETIME DEFAULT (STRFTIME('%Y-%m-%d %H:%M:%S', 'now', 'localtime')) NOT NULL,
  "created" DATETIME DEFAULT (STRFTIME('%Y-%m-%d %H:%M:%S', 'now', 'localtime')) NOT NULL,
  PRIMARY KEY(id),
  UNIQUE(name, id_directory)
);
CREATE TABLE files_collections (
  "id" VARCHAR(36) NOT NULL,
  "id_file" VARCHAR(36) NOT NULL,
  "id_collection" VARCHAR(36) NOT NULL,
  "modified" DATETIME DEFAULT (STRFTIME('%Y-%m-%d %H:%M:%S', 'now', 'localtime')) NOT NULL,
  "created" DATETIME DEFAULT (STRFTIME('%Y-%m-%d %H:%M:%S', 'now', 'localtime')) NOT NULL,
  PRIMARY KEY(id),
  UNIQUE(id_file, id_collection)
);
CREATE TABLE files_informations (
  "id" VARCHAR(36) NOT NULL,
  "id_file" VARCHAR(36) NOT NULL,
  "name" VARCHAR(255) NOT NULL,
  "value" VARCHAR(255) NOT NULL,
  "modified" DATETIME DEFAULT (STRFTIME('%Y-%m-%d %H:%M:%S', 'now', 'localtime')) NOT NULL,
  "created" DATETIME DEFAULT (STRFTIME('%Y-%m-%d %H:%M:%S', 'now', 'localtime')) NOT NULL,
  PRIMARY KEY(id)
);
CREATE TABLE groups (
  "id" VARCHAR(36) NOT NULL,
  "name" VARCHAR(255) NOT NULL,
  "id_group" VARCHAR(36) DEFAULT "" NOT NULL,
  "modified" DATETIME DEFAULT (STRFTIME('%Y-%m-%d %H:%M:%S', 'now', 'localtime')) NOT NULL,
  "created" DATETIME DEFAULT (STRFTIME('%Y-%m-%d %H:%M:%S', 'now', 'localtime')) NOT NULL,
  PRIMARY KEY(id),
  UNIQUE(name, id_group)
);
CREATE TABLE limits (
  "id" VARCHAR(36) NOT NULL,
  "id_accessor" VARCHAR(36) DEFAULT "" NOT NULL,
  "id_object" VARCHAR(36) DEFAULT "" NOT NULL,
  "name" VARCHAR(255) NOT NULL,
  "value" VARCHAR(255) NOT NULL,
  "modified" DATETIME DEFAULT (STRFTIME('%Y-%m-%d %H:%M:%S', 'now', 'localtime')) NOT NULL,
  "created" DATETIME DEFAULT (STRFTIME('%Y-%m-%d %H:%M:%S', 'now', 'localtime')) NOT NULL,
  PRIMARY KEY(id),
  UNIQUE(id_accessor, id_object, name)
);
CREATE TABLE permissions (
  "id" VARCHAR(36) NOT NULL,
  "id_accessor" VARCHAR(36) DEFAULT "" NOT NULL,
  "id_object" VARCHAR(36) DEFAULT "" NOT NULL,
  "right" VARCHAR(255) DEFAULT "" NOT NULL,
  "granted" BOOLEAN DEFAULT 1 NOT NULL,
  "modified" DATETIME DEFAULT (STRFTIME('%Y-%m-%d %H:%M:%S', 'now', 'localtime')) NOT NULL,
  "created" DATETIME DEFAULT (STRFTIME('%Y-%m-%d %H:%M:%S', 'now', 'localtime')) NOT NULL,
  PRIMARY KEY(id),
  UNIQUE(id_accessor, id_object, right)
);
CREATE TABLE sessions (
  "id" VARCHAR(36) NOT NULL,
  "expiration" DATETIME DEFAULT "" NOT NULL,
  "id_account" VARCHAR(36) DEFAULT "" NOT NULL,
  "modified" DATETIME DEFAULT (STRFTIME('%Y-%m-%d %H:%M:%S', 'now', 'localtime')) NOT NULL,
  "created" DATETIME DEFAULT (STRFTIME('%Y-%m-%d %H:%M:%S', 'now', 'localtime')) NOT NULL,
  PRIMARY KEY(id)
);
CREATE TABLE sessions_informations (
  "id" VARCHAR(36) NOT NULL,
  "id_session" VARCHAR(36) NOT NULL,
  "name" VARCHAR(255) NOT NULL,
  "value" VARCHAR(255) NOT NULL,
  "modified" DATETIME DEFAULT (STRFTIME('%Y-%m-%d %H:%M:%S', 'now', 'localtime')) NOT NULL,
  "created" DATETIME DEFAULT (STRFTIME('%Y-%m-%d %H:%M:%S', 'now', 'localtime')) NOT NULL,
  PRIMARY KEY(id)
);
CREATE TABLE tags (
  "id" VARCHAR(36) NOT NULL,
  "id_object" VARCHAR(36) NOT NULL,
  "name" VARCHAR(255) NOT NULL,
  "modified" DATETIME DEFAULT (STRFTIME('%Y-%m-%d %H:%M:%S', 'now', 'localtime')) NOT NULL,
  "created" DATETIME DEFAULT (STRFTIME('%Y-%m-%d %H:%M:%S', 'now', 'localtime')) NOT NULL,
  PRIMARY KEY(id),
  UNIQUE(id_object, name)
);
