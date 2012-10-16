TEMPLATE = lib
CONFIG += plugin
QT +=  network sql xml
RESOURCES = resources.qrc

INCLUDEPATH += . \
    ../../api \
    ../../api/table \
    ../../api/network \
    ../../extensions \
    ../../library \
    ./GnuTLS/include

TARGET = Plugin
DESTDIR = ../../build/plugins/Tls
LIBS += -L../../build -lLightBird
# GnuTLS should be located in the GnuTLS folder of the plugin
LIBS += -L../../../server/plugins/Tls/GnuTLS/lib -lgnutls.dll
# winsock2 is requires on Windows
win32:LIBS += -lWs2_32

HEADERS = Plugin.h
SOURCES = Plugin.cpp
OTHER_FILES = Configuration.xml

OBJECTS_DIR = tmp
RCC_DIR = tmp
MOC_DIR = tmp
