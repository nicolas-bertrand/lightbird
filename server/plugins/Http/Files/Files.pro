TEMPLATE = lib
CONFIG += plugin
QT +=  network sql xml
RESOURCES = resources.qrc

INCLUDEPATH += . \
    ../../../api \
    ../../../api/network \
    ../../../extensions \
    ../../../library

TARGET = Plugin
DESTDIR = ../../../build/plugins/Http/Files
LIBS += -L../../../build -lLightBird

HEADERS = Plugin.h
SOURCES = Plugin.cpp
OTHER_FILES = Configuration.xml

OBJECTS_DIR = tmp
RCC_DIR = tmp
MOC_DIR = tmp
