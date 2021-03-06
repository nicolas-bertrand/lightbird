# Configuration
TEMPLATE = app
CONFIG += \
    qt \
    console \
    thread \
    warn_on
DEPENDPATH += \
    sources \
    headers \
    headers/api \
    ../api \
    ../api/network \
    ../extensions \
    ../library
INCLUDEPATH += \
    headers \
    headers/api \
    ../api \
    ../api/network \
    ../extensions \
    ../library

# Qt Modules
QT += \
    core \
    gui \
    widgets \
    xml \
    network \
    sql

# Build
TARGET = LightBird
DESTDIR = ../build
LIBS += -L../build -lLightBirdLibrary
win32:LIBS += -lWs2_32

# We need to define __linux__ ourselves because MOC can't see built-in defines (QTBUG-34593)
linux:DEFINES += __linux__

# Resources
RESOURCES = resources/resources.qrc

# Icons
RC_FILE = resources/images/logo.rc # Windows
ICON = resources/images/logo.icns  # Mac OS X

# Translations
TRANSLATIONS = \
    resources/languages/en.ts \
    resources/languages/fr.ts

# Temporary directories
OBJECTS_DIR = tmp
RCC_DIR = tmp
MOC_DIR = tmp

# Headers and sources files
HEADERS += \
    headers/Arguments.h \
    headers/Client.h \
    headers/ClientDisconnectTimers.h \
    headers/Clients.h \
    headers/ClientsNetwork.h \
    headers/ClientsNetworkLinux.h \
    headers/ClientsNetworkWindows.h \
    headers/Configuration.h \
    headers/Configurations.h \
    headers/Content.h \
    headers/Context.h \
    headers/Database.h \
    headers/Engine.h \
    headers/EngineClient.h \
    headers/EngineServer.h \
    headers/Events.h \
    headers/Extensions.h \
    headers/Future.hpp \
    headers/IReadWrite.h \
    headers/Log.h \
    headers/Network.h \
    headers/Plugin.hpp \
    headers/Plugins.hpp \
    headers/Port.h \
    headers/PortTcp.h \
    headers/PortUdp.h \
    headers/Request.h \
    headers/Response.h \
    headers/Server.h \
    headers/ServerTcp.h \
    headers/ServerTcpLinux.h \
    headers/ServerTcpWindows.h \
    headers/ServerUdp.h \
    headers/ServerUdpLinux.h \
    headers/ServerUdpWindows.h \
    headers/Session.h \
    headers/Socket.h \
    headers/SocketTcp.h \
    headers/SocketTcpLinux.h \
    headers/SocketTcpWindows.h \
    headers/SocketUdp.h \
    headers/SocketUdpLinux.h \
    headers/SocketUdpWindows.h \
    headers/Thread.h \
    headers/ThreadPool.h \
    headers/Threads.h \
    headers/Timer.h \
    headers/WriteBuffer.h \
    headers/api/Api.h \
    headers/api/ApiConfiguration.h \
    headers/api/ApiDatabase.h \
    headers/api/ApiEvents.h \
    headers/api/ApiGuis.h \
    headers/api/ApiLogs.h \
    headers/api/ApiNetwork.h \
    headers/api/ApiPlugins.h \
    headers/api/ApiSessions.h \
    headers/api/ApiTimers.h
SOURCES += \
    sources/main.cpp \
    sources/Arguments.cpp \
    sources/Client.cpp \
    sources/ClientDisconnectTimers.cpp \
    sources/Clients.cpp \
    sources/ClientsNetwork.cpp \
    sources/ClientsNetworkLinux.cpp \
    sources/ClientsNetworkWindows.cpp \
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
    sources/Log.cpp \
    sources/Network.cpp \
    sources/Plugin.cpp \
    sources/Plugins.cpp \
    sources/Port.cpp \
    sources/PortTcp.cpp \
    sources/PortUdp.cpp \
    sources/Request.cpp \
    sources/Response.cpp \
    sources/Server.cpp \
    sources/ServerTcp.cpp \
    sources/ServerTcpLinux.cpp \
    sources/ServerTcpWindows.cpp \
    sources/ServerUdp.cpp \
    sources/ServerUdpLinux.cpp \
    sources/ServerUdpWindows.cpp \
    sources/Session.cpp \
    sources/Socket.cpp \
    sources/SocketTcp.cpp \
    sources/SocketTcpLinux.cpp \
    sources/SocketTcpWindows.cpp \
    sources/SocketUdp.cpp \
    sources/SocketUdpLinux.cpp \
    sources/SocketUdpWindows.cpp \
    sources/Thread.cpp \
    sources/ThreadPool.cpp \
    sources/Threads.cpp \
    sources/Timer.cpp \
    sources/WriteBuffer.cpp \
    sources/api/Api.cpp \
    sources/api/ApiConfiguration.cpp \
    sources/api/ApiDatabase.cpp \
    sources/api/ApiEvents.cpp \
    sources/api/ApiGuis.cpp \
    sources/api/ApiLogs.cpp \
    sources/api/ApiNetwork.cpp \
    sources/api/ApiPlugins.cpp \
    sources/api/ApiSessions.cpp \
    sources/api/ApiTimers.cpp
OTHER_FILES = \
    resources/configurations/Configuration.xml \
    resources/configurations/FilesExtensions.json \
    resources/databases/queries.xml \
    resources/databases/foreign_keys.sql \
    resources/databases/structure.sql \
    resources/databases/triggers.sql \
    resources/databases/tuple.sql
