TEMPLATE = lib
CONFIG += plugin
QT += network sql xml
RESOURCES = resources.qrc

INCLUDEPATH += . \
    ../../../api \
    ../../../api/table \
    ../../../api/network \
    ../../../extensions

TARGET = Identifier
DESTDIR = ../../../build/plugins/Extensions/Identifier

HEADERS = Plugin.h \
    Identifier.h
SOURCES = Plugin.cpp \
    Identifier.cpp
OTHER_FILES = Configuration.xml \
    Mime.xml

OBJECTS_DIR = tmp
RCC_DIR = tmp
MOC_DIR = tmp
