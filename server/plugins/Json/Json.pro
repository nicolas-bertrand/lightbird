TEMPLATE = lib
CONFIG += plugin
QT += network sql xml
RESOURCES = resources.qrc

INCLUDEPATH += . \
    ../../api \
    ../../api/table \
    ../../api/network \
    ../../extensions \
    ./QJson

TARGET = Json
DESTDIR = ../../build/plugins/Json

# Builds the plugin with QJson, which allows it to unserialize a request in Json.
# Without QJson the plugin can only serialize QVariant objects into json.
LIBS += -L. -lqjson.0.7.1
DEFINES += QJSON

HEADERS = Plugin.h \
    QJson/parser.h \
    QJson/qjson_export.h \
    QJson/serializer.h
SOURCES = Plugin.cpp
OTHER_FILES = Configuration.xml

OBJECTS_DIR = tmp
RCC_DIR = tmp
MOC_DIR = tmp
