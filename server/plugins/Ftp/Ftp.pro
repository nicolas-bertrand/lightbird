TEMPLATE = lib
CONFIG += plugin
QT += network sql xml
RESOURCES = resources.qrc

INCLUDEPATH += \
    ../../api \
    ../../api/network \
    ../../extensions \
    ../../library

TARGET = Ftp
DESTDIR = ../../build/plugins/Ftp
LIBS += -L../../build -lLightBird

HEADERS = \
    Commands.h \
    Control.h \
    Data.h \
    Ftp.h \
    Parser.h \
    ParserControl.h \
    ParserData.h \
    Plugin.h \
    Timer.h
SOURCES = \
    Commands.cpp \
    Control.cpp \
    Data.cpp \
    Parser.cpp \
    ParserControl.cpp \
    ParserData.cpp \
    Plugin.cpp \
    Timer.cpp
OTHER_FILES = Configuration.xml

OBJECTS_DIR = tmp
RCC_DIR = tmp
MOC_DIR = tmp
