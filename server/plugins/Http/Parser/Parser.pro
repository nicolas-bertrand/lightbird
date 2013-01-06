TEMPLATE = lib
CONFIG += plugin
QT += network sql xml
RESOURCES = resources.qrc

INCLUDEPATH += \
    ../../../api \
    ../../../api/network \
    ../../../extensions \
    ../../../library

TARGET = HttpParser
DESTDIR = ../../../build/plugins/Http/Parser
LIBS += -L../../../build -lLightBirdLibrary

HEADERS = \
    Parser.h \
    ParserClient.h \
    ParserServer.h \
    Plugin.h
SOURCES = \
    Parser.cpp \
    ParserClient.cpp \
    ParserServer.cpp \
    Plugin.cpp
OTHER_FILES = Configuration.xml

OBJECTS_DIR = tmp
RCC_DIR = tmp
MOC_DIR = tmp
