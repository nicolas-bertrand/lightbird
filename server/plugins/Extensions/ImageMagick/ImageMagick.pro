TEMPLATE = lib
CONFIG += plugin
QT += network sql xml
RESOURCES = resources.qrc

INCLUDEPATH += . \
    ../../../api \
    ../../../api/table \
    ../../../api/network \
    ../../../extensions \
    ../../../library \
    ./include

TARGET = ImageMagick
DESTDIR = ../../../build/plugins/Extensions/ImageMagick
LIBS += -L../../../build -lLightBird

HEADERS = Identify.h \
    Image.h \
    Plugin.h
SOURCES = Identify.cpp \
    Image.cpp \
    Plugin.cpp
OTHER_FILES = Configuration.xml \
    Readme.txt

OBJECTS_DIR = tmp
RCC_DIR = tmp
MOC_DIR = tmp
