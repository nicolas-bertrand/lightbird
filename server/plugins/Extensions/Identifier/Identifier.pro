TEMPLATE = lib
CONFIG += plugin
QT += network sql xml
RESOURCES = resources.qrc

INCLUDEPATH += . \
    ../../../api \
    ../../../api/network \
    ../../../extensions \
    ../../../library

TARGET = Identifier
DESTDIR = ../../../build/plugins/Extensions/Identifier
LIBS += -L../../../build -lLightBird

HEADERS = Plugin.h \
    Identifier.h
SOURCES = Plugin.cpp \
    Identifier.cpp
OTHER_FILES = Configuration.xml \
    Mime.xml

OBJECTS_DIR = tmp
RCC_DIR = tmp
MOC_DIR = tmp
