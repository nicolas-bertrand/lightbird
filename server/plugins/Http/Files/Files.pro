TEMPLATE = lib
CONFIG += plugin
QT +=  network sql xml
RESOURCES = resources.qrc

INCLUDEPATH += \
    ../../../api \
    ../../../api/network \
    ../../../extensions \
    ../../../library

TARGET = HttpFiles
DESTDIR = ../../../build/plugins/Http/Files
LIBS += -L../../../build -lLightBirdLibrary

HEADERS = \
    Context.h \
    Plugin.h
SOURCES = \
    Context.cpp \
    Plugin.cpp
OTHER_FILES = Configuration.xml

OBJECTS_DIR = tmp
RCC_DIR = tmp
MOC_DIR = tmp
