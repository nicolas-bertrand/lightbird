TEMPLATE = lib
CONFIG += plugin
QT +=  network sql xml
RESOURCES = resources.qrc

INCLUDEPATH += . \
    ../../api \
    ../../api/table \
    ../../api/network \
    ../../extensions \
    ../../library

TARGET = Test
DESTDIR = ../../build/plugins/Test
LIBS += -L../../build -lLightBird

HEADERS = Plugin.h \
    UnitTests.h
SOURCES = Plugin.cpp \
    UnitTests.cpp
OTHER_FILES = Configuration.xml

OBJECTS_DIR = tmp
RCC_DIR = tmp
MOC_DIR = tmp
