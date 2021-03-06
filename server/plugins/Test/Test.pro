TEMPLATE = lib
CONFIG += plugin
QT +=  network sql xml
RESOURCES = resources.qrc

INCLUDEPATH += \
    ../../api \
    ../../api/network \
    ../../extensions \
    ../../library

TARGET = Test
DESTDIR = ../../build/plugins/Test
LIBS += -L../../build -lLightBirdLibrary

HEADERS = \
    Configuration.h \
    Database.h \
    Ftp.h \
    ITest.h \
    Library.h \
    Network.h \
    Plugin.h
SOURCES = \
    Configuration.cpp \
    Database.cpp \
    Ftp.cpp \
    Library.cpp \
    Network.cpp \
    Plugin.cpp
OTHER_FILES = Configuration.xml

OBJECTS_DIR = tmp
RCC_DIR = tmp
MOC_DIR = tmp
