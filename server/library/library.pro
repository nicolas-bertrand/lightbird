TEMPLATE = lib

INCLUDEPATH += . \
    ../api

TARGET = LightBird
DESTDIR = ../build
DEFINES += LIGHTBIRD_LIBRARY

HEADERS = Export.h \
    Library.h \
    LightBird.h \
    Properties.h \
    SmartMutex.h
SOURCES = Library.cpp \
    LightBird.cpp \
    Properties.cpp \
    Sha256.cpp \
    SmartMutex.cpp

OBJECTS_DIR = tmp
RCC_DIR = tmp
MOC_DIR = tmp
