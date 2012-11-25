TEMPLATE = lib
CONFIG += plugin
QT += network sql xml
RESOURCES = resources.qrc

INCLUDEPATH += . \
    ../../../api \
    ../../../api/network \
    ../../../extensions \
    ../../../library

TARGET = HttpClient
DESTDIR = ../../../build/plugins/Http/Client
LIBS += -L../../../build -lLightBird

HEADERS = Plugin.h \
    Audio.h \
    Commands.h \
    Files.h \
    Media.h \
    Medias.h \
    Preview.h \
    Uploads.h \
    Video.h
SOURCES = Plugin.cpp \
    Audio.cpp \
    Commands.cpp \
    Files.cpp \
    Media.cpp \
    Medias.cpp \
    Preview.cpp \
    Uploads.cpp \
    Video.cpp
OTHER_FILES = Configuration.xml \
    Queries.xml

OBJECTS_DIR = tmp
RCC_DIR = tmp
MOC_DIR = tmp
