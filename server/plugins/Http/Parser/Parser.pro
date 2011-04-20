TEMPLATE    =   lib
CONFIG      +=  plugin
QT          +=  network sql xml
RESOURCES   =   resources.qrc

INCLUDEPATH +=  . \
                ../../../api \
                ../../../api/table \
                ../../../api/network \
                ../../../extensions

TARGET      =   Parser
DESTDIR     =   ../../../build/plugins/Http/Parser

HEADERS     =   Plugin.h \
                Parser.h
SOURCES     =   Plugin.cpp \
                Parser.cpp
OTHER_FILES =   Configuration.xml

OBJECTS_DIR =   tmp
RCC_DIR     =   tmp
MOC_DIR     =   tmp
