TEMPLATE = lib
CONFIG += plugin
QT += network sql xml
RESOURCES = resources.qrc

INCLUDEPATH += . \
    ../../../api \
    ../../../api/network \
    ../../../extensions \
    ../../../library

TARGET = Mime
DESTDIR = ../../../build/plugins/Extensions/Mime
LIBS += -L../../../build -lLightBird

HEADERS = Plugin.h
SOURCES = Plugin.cpp
OTHER_FILES = Configuration.xml \
    Mime.xml

OBJECTS_DIR = tmp
RCC_DIR = tmp
MOC_DIR = tmp
