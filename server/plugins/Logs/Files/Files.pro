TEMPLATE    =   lib
CONFIG      +=  plugin
QT          +=  network sql xml
RESOURCES   =   resources.qrc

INCLUDEPATH +=  . \
                ../../../api \
                ../../../api/table \
                ../../../api/network \
                ../../../extensions

TARGET      =   Files
DESTDIR     =   ../../../build/plugins/Logs/Files

HEADERS     =   Files.h
SOURCES     =   Files.cpp
OTHER_FILES =   Configuration.xml

OBJECTS_DIR =   tmp
RCC_DIR     =   tmp
MOC_DIR     =   tmp
