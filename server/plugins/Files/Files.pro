TEMPLATE = lib
CONFIG += plugin
QT +=  network sql xml
RESOURCES = resources.qrc

INCLUDEPATH += \
    ../../api \
    ../../api/network \
    ../../extensions \
    ../../library

TARGET = Files
DESTDIR = ../../build/plugins/Files
LIBS += -L../../build -lLightBirdLibrary

HEADERS = \
    Files.h \
    Plugin.h
SOURCES = \
    Files.cpp \
    Plugin.cpp
OTHER_FILES = Configuration.xml

OBJECTS_DIR = tmp
RCC_DIR = tmp
MOC_DIR = tmp
