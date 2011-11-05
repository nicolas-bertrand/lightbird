# Configuration
TEMPLATE = app
CONFIG += qt \
    console \
    thread \
    sql \
    warn_on
DEPENDPATH += sources \
    headers \
    headers/api \
    headers/tables
INCLUDEPATH += headers \
    headers/api \
    headers/tables \
    ../api \
    ../api/table \
    ../api/network

# Qt Modules
QT += xml \
    network \
    sql

# Build
TARGET = LightBird
DESTDIR = ../build

# Resources
RESOURCES = resources/resources.qrc

# Icons
RC_FILE = resources/images/logo.rc  # Windows
ICON = resources/images/logo.icns   # Mac OS X

# Translations
TRANSLATIONS = resources/languages/server_en.ts \
    resources/languages/server_fr.ts

# Temporary directories
OBJECTS_DIR = tmp
RCC_DIR = tmp
MOC_DIR = tmp

# Headers and sources files
HEADERS += headers/Arguments.h \
    headers/Client.h \
    headers/Clients.h \
    headers/Configuration.h \
    headers/Configurations.h \
    headers/Content.h \
    headers/Context.h \
    headers/Database.h \
    headers/Defines.h \
    headers/Engine.h \
    headers/EngineClient.h \
    headers/EngineServer.h \
    headers/Events.h \
    headers/Extensions.h \
    headers/Future.hpp \
    headers/Initialize.h \
    headers/IReadWrite.h \
    headers/Log.h \
    headers/Network.h \
    headers/Plugin.hpp \
    headers/Plugins.hpp \
    headers/Port.h \
    headers/PortTcp.h \
    headers/PortUdp.h \
    headers/Properties.h \
    headers/Request.h \
    headers/Response.h \
    headers/Server.h \
    headers/SmartMutex.h \
    headers/Session.h \
    headers/Thread.h \
    headers/ThreadPool.h \
    headers/Threads.h \
    headers/Timer.h \
    headers/Tools.h \
    headers/api/Api.h \
    headers/api/ApiConfiguration.h \
    headers/api/ApiDatabase.h \
    headers/api/ApiEvents.h \
    headers/api/ApiGuis.h \
    headers/api/ApiLogs.h \
    headers/api/ApiNetwork.h \
    headers/api/ApiPlugins.h \
    headers/api/ApiSessions.h \
    headers/api/ApiTimers.h \
    headers/tables/Table.h \
    headers/tables/TableAccessors.h \
    headers/tables/TableAccounts.h \
    headers/tables/TableCollections.h \
    headers/tables/TableDirectories.h \
    headers/tables/TableEvents.h \
    headers/tables/TableFiles.h \
    headers/tables/TableGroups.h \
    headers/tables/TableLimits.h \
    headers/tables/TableObjects.h \
    headers/tables/TablePermissions.h \
    headers/tables/TableTags.h
SOURCES += sources/main.cpp \
    sources/Arguments.cpp \
    sources/Client.cpp \
    sources/Clients.cpp \
    sources/Configuration.cpp \
    sources/Configurations.cpp \
    sources/Content.cpp \
    sources/Context.cpp \
    sources/Database.cpp \
    sources/Engine.cpp \
    sources/EngineClient.cpp \
    sources/EngineServer.cpp \
    sources/Events.cpp \
    sources/Extensions.cpp \
    sources/Initialize.cpp \
    sources/Log.cpp \
    sources/Network.cpp \
    sources/Plugin.cpp \
    sources/Plugins.cpp \
    sources/Port.cpp \
    sources/PortTcp.cpp \
    sources/PortUdp.cpp \
    sources/Properties.cpp \
    sources/Request.cpp \
    sources/Response.cpp \
    sources/Server.cpp \
    sources/SmartMutex.cpp \
    sources/Session.cpp \
    sources/Thread.cpp \
    sources/ThreadPool.cpp \
    sources/Threads.cpp \
    sources/Timer.cpp \
    sources/Tools.cpp \
    sources/api/Api.cpp \
    sources/api/ApiConfiguration.cpp \
    sources/api/ApiDatabase.cpp \
    sources/api/ApiEvents.cpp \
    sources/api/ApiGuis.cpp \
    sources/api/ApiLogs.cpp \
    sources/api/ApiNetwork.cpp \
    sources/api/ApiPlugins.cpp \
    sources/api/ApiSessions.cpp \
    sources/api/ApiTimers.cpp \
    sources/tables/Table.cpp \
    sources/tables/TableAccessors.cpp \
    sources/tables/TableAccounts.cpp \
    sources/tables/TableCollections.cpp \
    sources/tables/TableDirectories.cpp \
    sources/tables/TableEvents.cpp \
    sources/tables/TableFiles.cpp \
    sources/tables/TableGroups.cpp \
    sources/tables/TableLimits.cpp \
    sources/tables/TableObjects.cpp \
    sources/tables/TablePermissions.cpp \
    sources/tables/TableTags.cpp
OTHER_FILES = resources/configurations/Configuration.xml \
    resources/databases/queries.xml \
    resources/databases/foreign_keys.sql \
    resources/databases/structure.sql \
    resources/databases/triggers.sql \
    resources/databases/tuple.sql
