TEMPLATE    =   lib
CONFIG      +=  plugin
QT          +=  network sql xml
RESOURCES   =   resources.qrc

INCLUDEPATH +=  . \
                ../../../api \
                ../../../api/table \
                ../../../api/network \
                ../../../extensions

TARGET      =   Get
DESTDIR     =   ../../../build/plugins/HTTP/Get

HEADERS     =   Plugin.h
SOURCES     =   Plugin.cpp
OTHER_FILES =   Configuration.xml

OBJECTS_DIR =   tmp
RCC_DIR     =   tmp
MOC_DIR     =   tmp
