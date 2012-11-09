TEMPLATE = lib
CONFIG += plugin
QT += network sql xml
RESOURCES = resources.qrc

INCLUDEPATH += . \
    ../../../api \
    ../../../api/network \
    ../../../extensions \
    ../../../library

TARGET = Parser
DESTDIR = ../../../build/plugins/Http/Parser
LIBS += -L../../../build -lLightBird

HEADERS = Plugin.h \
    Parser.h \
    ParserClient.h \
    ParserServer.h
SOURCES = Plugin.cpp \
    Parser.cpp \
    ParserClient.cpp \
    ParserServer.cpp
OTHER_FILES = Configuration.xml

OBJECTS_DIR = tmp
RCC_DIR = tmp
MOC_DIR = tmp
