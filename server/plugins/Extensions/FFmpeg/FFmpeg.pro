TEMPLATE = lib
CONFIG += plugin
QT += network sql xml
RESOURCES = resources.qrc

INCLUDEPATH += \
    ../../../api \
    ../../../api/network \
    ../../../extensions \
    ../../../library \
    ./include

TARGET = FFmpeg
DESTDIR = ../../../build/plugins/Extensions/FFmpeg
LIBS += -L../../../build -lLightBirdLibrary
LIBS += -L../../../../server/plugins/Extensions/FFmpeg/lib -lavcodec -lavfilter -lavformat -lavutil -lswscale

HEADERS = Audio.h \
    FFmpeg.h \
    Identify.h \
    Plugin.h \
    Preview.h \
    Video.h
SOURCES = Audio.cpp \
    Identify.cpp \
    Plugin.cpp \
    Preview.cpp \
    Video.cpp
OTHER_FILES = Configuration.xml

OBJECTS_DIR = tmp
RCC_DIR = tmp
MOC_DIR = tmp
