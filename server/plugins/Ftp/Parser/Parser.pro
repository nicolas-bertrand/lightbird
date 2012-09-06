TEMPLATE = lib
CONFIG += plugin
QT += network sql xml
RESOURCES = resources.qrc

INCLUDEPATH += . \
    ../../../api \
    ../../../api/table \
    ../../../api/network \
    ../../../extensions \
    ../../../library

TARGET = Parser
DESTDIR = ../../../build/plugins/Ftp/Parser
LIBS += -L../../../build -lLightBird

HEADERS = Plugin.h \
    Parser.h \
    ControlParser.h \
    DataParser.h
SOURCES = Plugin.cpp \
    Parser.cpp \
    ControlParser.cpp \
    DataParser.cpp
OTHER_FILES = Configuration.xml

OBJECTS_DIR = tmp
RCC_DIR = tmp
MOC_DIR = tmp
