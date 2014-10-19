#ifndef LIGHTBIRD_DEFINES_H
# define LIGHTBIRD_DEFINES_H

/// This header defines the macro used by the server.

# define VERSION                              "11.11"               ///< The version of the server
# define DATE_FORMAT                          "yyyy-MM-dd hh:mm:ss" ///< The database date format, used through QDateTime.
# define MAXTRYLOCK                           -1                    ///< Maximum time to wait in a tryLock in milliseconds. After that, an error should be returned.
# define PLUGINS_RESOURCES_PATH               ":plugins"            ///< The resources path of a plugin is this macro followed by its id.
# define FILES_EXTENSIONS_RESOURCES_PATH      ":filesExtensions"    ///< The path to the resource that stores the files extensions.
# define DEFAULT_THREADS_NUMBER               10                    ///< The default number of threads in the thread pool.

# define DEFAULT_CONFIGURATION_DIRECTORY      "configurations"      ///< The path to the configuration file of the server.
# define DEFAULT_CONFIGURATION_FILE           "Configuration.xml"   ///< The default name of the XML configuration file.
# define DEFAULT_CONFIGURATION_RESSOURCE      ":configuration"      ///< The ressource alias of the default configuration of the server.

# define DEFAULT_DATABASE_TYPE                "QSQLITE"             ///< The default Qt SQL driver type.
# define DEFAULT_DATABASE_PATH                "databases"           ///< The name of the default directory where the database is stored.
# define DEFAULT_DATABASE_FILE                "database.sqlite"     ///< The default file name of the database.
# define DEFAULT_DATABASE_RESOURCE            ":database"           ///< The name of the database file in the resources of the server.

# define DEFAULT_PERMISSIONS_ACTIVATE         "false"               ///< If the permissions system is activated.
# define DEFAULT_PERMISSIONS_DEFAULT          "false"               ///< The default permissions.
# define DEFAULT_PERMISSIONS_INHERITANCE      "true"                ///< If the inheritance of the rights is activated.
# define DEFAULT_PERMISSIONS_OWNERINHERITANCE "true"                ///< If the inheritance of the owner rights is activated.
# define DEFAULT_PERMISSIONS_GROUPINHERITANCE "true"                ///< If the inheritance of the group rights is activated.

# define DEFAULT_PLUGINS_PATH                 "plugins"             ///< The name of the default plugins directory.
# define DEFAULT_QT_PLUGINS_PATH              "QtPlugins"           ///< The default path to the plugins of Qt. Used to distrubute the server.
# define DEFAULT_FILES_PATH                   "files"               ///< The name of the default directory of the files.
# define DEFAULT_TEMPORARY_PATH               "tmp"                 ///< The name of the default temporary directory.
# define DEFAULT_CLEAN_TEMPORARY_PATH         "true"                ///< Defines if the temporary directory must be emptied every time the server is started.
# define DEFAULT_LANGUAGES_PATH               "languages"           ///< The name of the default languages directory.

#endif // LIGHTBIRD_DEFINES_H
