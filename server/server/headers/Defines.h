#ifndef DEFINES_H
# define DEFINES_H

/// This header defines the macro used in the server.

# define VERSION                                "0.0.1"                 ///< The version of the server
# define DATE_FORMAT                            "yyyy-MM-dd hh:mm:ss"   ///< The database date format, used through QDateTime.
# define MAXTRYLOCK                             1000                    ///< Maximum time to wait in a tryLock in milliseconds. After that, an error should be returned.
# define PLUGINS_RESOURCES_PATH                 ":plugins"              ///< The resources path of a plugin is this macro followed by its id.

# define DEFAULT_CONFIGURATION_DIRECTORY        "configurations"        ///< The path to the configuration file of the server.
# define DEFAULT_CONFIGURATION_FILE             "Configuration.xml"     ///< The default name of the XML configuration file.
# define DEFAULT_CONFIGURATION_RESSOURCE        ":configuration"        ///< The ressource alias of the default configuration of the server.

# define DEFAULT_DATABASE_TYPE                  "QSQLITE"               ///< The default Qt SQL driver type.
# define DEFAULT_DATABASE_PATH                  "databases"             ///< The name of the default directory where the database is stored.
# define DEFAULT_DATABASE_FILE                  "database.sqlite"       ///< The default file name of the database.
# define DEFAULT_DATABASE_RESOURCE              ":database"             ///< Nom de la base de données dans les ressources du serveur.

# define DEFAULT_PERMISSIONS_ACTIVATE           "false"                 ///< If the permissions system is activated.
# define DEFAULT_PERMISSIONS_DEFAULT            "false"                 ///< The default permissions.
# define DEFAULT_PERMISSIONS_INHERITANCE        "true"                  ///< If the inheritance of the rights is activated.
# define DEFAULT_PERMISSIONS_OWNERINHERITANCE   "true"                  ///< If the inheritance of the owner rights is activated.
# define DEFAULT_PERMISSIONS_GROUPINHERITANCE   "true"                  ///< If the inheritance of the owner rights is activated.

# define DEFAULT_PLUGINS_PATH                   "plugins"               ///< The name of the default plugins directory.
# define DEFAULT_QT_PLUGINS_PATH                "plugins"               ///< The default path to the plugins of Qt. Used to distrubute the server.
# define DEFAULT_FILES_PATH                     "files"                 ///< The name of the default directory of the files.
# define DEFAULT_TEMPORARY_PATH                 "tmp"                   ///< The name of the default temporary directory.
# define DEFAULT_CLEAN_TEMPORARY_PATH           "true"                  ///< Defines if the temporary directory must be emptied every time the server is started.
# define DEFAULT_LANGUAGES_PATH                 "languages"             ///< The name of the default languages directory.

#endif // DEFINES_H
