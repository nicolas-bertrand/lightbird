TEMPLATE = lib
CONFIG += plugin
QT += network sql xml
RESOURCES = resources.qrc

INCLUDEPATH += \
    ../../../api \
    ../../../api/network \
    ../../../extensions \
    ../../../library \
    ./include

DEFINES += TAGLIB_STATIC

TARGET = TagLib
DESTDIR = ../../../build/plugins/Extensions/TagLib
LIBS += -L../../../build -lLightBirdLibrary
LIBS += -L../../../../server/plugins/Extensions/TagLib -lTagLib

HEADERS = \
    Identify.h \
    Plugin.h \
    Preview.h
SOURCES = \
    Identify.cpp \
    Plugin.cpp \
    Preview.cpp
OTHER_FILES = Configuration.xml

OBJECTS_DIR = tmp
RCC_DIR = tmp
MOC_DIR = tmp
