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

HEADERS = Plugin.h \
    Parser.h \
    ParserControl.h \
    ParserData.h \
    ClientHandler.h \
    Execute.h
SOURCES = Plugin.cpp \
    Parser.cpp \
    ParserControl.cpp \
    ParserData.cpp \
    ClientHandler.cpp \
    Execute.cpp
OTHER_FILES = Configuration.xml \
    Queries.xml

OBJECTS_DIR = tmp
RCC_DIR = tmp
MOC_DIR = tmp
