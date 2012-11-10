TEMPLATE = lib
CONFIG += plugin
QT += network sql xml
RESOURCES = resources.qrc

INCLUDEPATH += . \
    ../../../api \
    ../../../api/network \
    ../../../extensions \
    ../../../library \
    ./include

TARGET = FFmpeg
DESTDIR = ../../../build/plugins/Extensions/FFmpeg
LIBS += -L../../../build -lLightBird
LIBS += -L../../../../server/plugins/Extensions/FFmpeg/lib -lavcodec -lavdevice -lavfilter -lavformat -lavutil -lpostproc -lswresample -lswscale

HEADERS = FFmpeg.h \
    Identify.h \
    Plugin.h \
    Preview.h
SOURCES = Identify.cpp \
    Plugin.cpp \
    Preview.cpp
OTHER_FILES = Configuration.xml

OBJECTS_DIR = tmp
RCC_DIR = tmp
MOC_DIR = tmp
