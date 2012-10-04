TEMPLATE = lib
CONFIG += plugin
QT += network sql xml
RESOURCES = resources.qrc

INCLUDEPATH += . \
    ../../api \
    ../../api/table \
    ../../api/network \
    ../../extensions \
    ../../library

TARGET = Ftp
DESTDIR = ../../build/plugins/Ftp
LIBS += -L../../build -lLightBird

HEADERS =  ClientHandler.h \
    Commands.h \
    Parser.h \
    ParserControl.h \
    ParserData.h \
    Plugin.h \
    Timer.h
SOURCES = ClientHandler.cpp \
    Commands.cpp \
    Parser.cpp \
    ParserControl.cpp \
    ParserData.cpp \
    Plugin.cpp \
    Timer.cpp
OTHER_FILES = Configuration.xml \
    Queries.xml

OBJECTS_DIR = tmp
RCC_DIR = tmp
MOC_DIR = tmp
