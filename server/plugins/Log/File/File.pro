TEMPLATE = lib
CONFIG += plugin
QT +=  network sql xml
RESOURCES = resources.qrc

INCLUDEPATH += . \
    ../../../api \
    ../../../api/network \
    ../../../extensions \
    ../../../library

TARGET = LogFile
DESTDIR = ../../../build/plugins/Log/File
LIBS += -L../../../build -lLightBird

HEADERS = File.h
SOURCES = File.cpp
OTHER_FILES = Configuration.xml

OBJECTS_DIR = tmp
RCC_DIR = tmp
MOC_DIR = tmp
