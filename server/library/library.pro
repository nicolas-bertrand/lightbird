TEMPLATE = lib
QT += sql xml

DEPENDPATH += .
INCLUDEPATH += \
    ../api \
    ../extensions

TARGET = LightBirdLibrary
DESTDIR = ../build
DEFINES += LIGHTBIRD_LIBRARY

HEADERS = \
    Configuration.h \
    Defines.h \
    Export.h \
    FilesExtensions.h \
    Identify.h \
    Initialize.h \
    Library.h \
    LightBird.h \
    Mutex.h \
    Preview.h \
    Properties.h \
    Table.h \
    TableAccessors.h \
    TableAccounts.h \
    TableCollections.h \
    TableDirectories.h \
    TableEvents.h \
    TableFiles.h \
    TableGroups.h \
    TableLimits.h \
    TableObjects.h \
    TablePermissions.h \
    TableTags.h \
    Dir.h \
    File.h \
    Node.h
SOURCES = \
    Configuration.cpp \
    FilesExtensions.cpp \
    Identify.cpp \
    Initialize.cpp \
    Library.cpp \
    LightBird.cpp \
    Mutex.cpp \
    Preview.cpp \
    Properties.cpp \
    Table.cpp \
    TableAccessors.cpp \
    TableAccounts.cpp \
    TableCollections.cpp \
    TableDirectories.cpp \
    TableEvents.cpp \
    TableFiles.cpp \
    TableGroups.cpp \
    TableLimits.cpp \
    TableObjects.cpp \
    TablePermissions.cpp \
    TableTags.cpp \
    Dir.cpp \
    File.cpp \
    Node.cpp

OBJECTS_DIR = tmp
RCC_DIR = tmp
MOC_DIR = tmp
