TEMPLATE = lib
CONFIG += plugin
QT +=  network sql xml
RESOURCES = resources.qrc

INCLUDEPATH += \
    ../../api \
    ../../api/network \
    ../../extensions \
    ../../library \
    ./GnuTLS/include

TARGET = Tls
DESTDIR = ../../build/plugins/Tls
LIBS += \
    -L../../build -lLightBirdLibrary \
    -L../../../server/plugins/Tls/GnuTLS/lib
win32:LIBS += -lWs2_32 # winsock2 is required on Windows

QT_INSTALL_PREFIX = $$[QT_INSTALL_PREFIX]
X64 = $$find(QT_INSTALL_PREFIX, 64)
win32 {
    isEmpty(X64) {
        CONFIG(debug, debug|release):LIBS += -lgnutls.dll.32D
        CONFIG(release, debug|release):LIBS += -lgnutls.dll.32
    } else {
        CONFIG(debug, debug|release):LIBS += -lgnutls.dll.64D
        CONFIG(release, debug|release):LIBS += -lgnutls.dll.64
    }
}
linux {
    #./configure --prefix=PATH  CXXFLAGS="-fPIC" CFLAGS="-fPIC" --enable-static=yes --enable-shared=no
    QMAKE_LFLAGS += -z defs
    LIBS += -lgnutls -lp11-kit -lgmp -lhogweed -lz -lnettle
}

HEADERS = \
    Handshake.h \
    Plugin.h \
    Record.h \
    Tls.h
SOURCES = \
    Handshake.cpp \
    Plugin.cpp \
    Record.cpp
OTHER_FILES = Configuration.xml

OBJECTS_DIR = tmp
RCC_DIR = tmp
MOC_DIR = tmp
