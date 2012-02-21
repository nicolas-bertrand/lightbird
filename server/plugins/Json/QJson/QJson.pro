TEMPLATE = lib
CONFIG += warn_off debug staticlib
QT -= gui

INCLUDEPATH += .

TARGET = qjson.0.7.1
DESTDIR = ..

PRIVATE_HEADERS += \
    json_parser.hh \
    json_scanner.h \
    location.hh \
    parser_p.h  \
    position.hh \
    qjson_debug.h  \
    stack.hh
PUBLIC_HEADERS += \
    parser.h \
    parserrunnable.h \
    qobjecthelper.h \
    serializer.h \
    serializerrunnable.h \
    qjson_export.h

HEADERS += $$PRIVATE_HEADERS $$PUBLIC_HEADERS
SOURCES += \
    json_parser.cc \
    json_scanner.cpp \
    parser.cpp \
    parserrunnable.cpp \
    qobjecthelper.cpp \
    serializer.cpp \
    serializerrunnable.cpp

OBJECTS_DIR = tmp
RCC_DIR = tmp
MOC_DIR = tmp
