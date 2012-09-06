TEMPLATE = lib
CONFIG += plugin debug
QT += network sql xml
RESOURCES = resources.qrc

INCLUDEPATH += . \
    ../../../api \
    ../../../api/table \
    ../../../api/network \
    ../../../extensions \
    ../../../library

TARGET = Execute
DESTDIR = ../../../build/plugins/Ftp/Execute
LIBS += -L../../../build -lLightBird

HEADERS = Plugin.h \
        ClientHandler.h \
        Execute.h

SOURCES = Plugin.cpp \
        ClientHandler.cpp \
        Execute.cpp

OTHER_FILES = Configuration.xml

OBJECTS_DIR = tmp
RCC_DIR = tmp
MOC_DIR = tmp
