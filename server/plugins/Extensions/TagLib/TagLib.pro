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

DEFINES += TAGLIB_STATIC

TARGET = TagLib
DESTDIR = ../../../build/plugins/Extensions/TagLib
LIBS += \
    -L../../../build -lLightBirdLibrary \
    -L../../../../server/plugins/Extensions/TagLib

QT_INSTALL_PREFIX = $$[QT_INSTALL_PREFIX]
X64 = $$find(QT_INSTALL_PREFIX, 64)
win32 {
    isEmpty(X64) {
        CONFIG(debug, debug|release):LIBS += -lTagLib32D
        CONFIG(release, debug|release):LIBS += -lTagLib32
    } else {
        CONFIG(debug, debug|release):LIBS += -lTagLib64D
        CONFIG(release, debug|release):LIBS += -lTagLib64
    }
}
linux {
    #cmake -DCMAKE_INSTALL_PREFIX=./build -DCMAKE_BUILD_TYPE=Release -DENABLE_STATIC=ON -DENABLE_STATIC_RUNTIME=ON -DCMAKE_CXX_FLAGS="-fPIC" .
    QMAKE_LFLAGS += -z defs
    LIBS += -lTagLib -lz
}

HEADERS = \
    Identify.h \
    Plugin.h \
    Preview.h
SOURCES = \
    Identify.cpp \
    Plugin.cpp \
    Preview.cpp
OTHER_FILES = Configuration.xml

OBJECTS_DIR = tmp
RCC_DIR = tmp
MOC_DIR = tmp
