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

TARGET = FFmpeg
DESTDIR = ../../../build/plugins/Extensions/FFmpeg
LIBS += \
    -L../../../build -lLightBirdLibrary \
    -L../../../../server/plugins/Extensions/FFmpeg/lib

QT_INSTALL_PREFIX = $$[QT_INSTALL_PREFIX]
X64 = $$find(QT_INSTALL_PREFIX, 64)
win32 {
    isEmpty(X64) {
        CONFIG(debug, debug|release):LIBS += -lavcodec32D -lavfilter32D -lavformat32D -lavutil32D -lswscale32D
        CONFIG(release, debug|release):LIBS += -lavcodec32 -lavfilter32 -lavformat32 -lavutil32 -lswscale32
    } else {
        CONFIG(debug, debug|release):LIBS += -lavcodec64D -lavfilter64D -lavformat64D -lavutil64D -lswscale64D
        CONFIG(release, debug|release):LIBS += -lavcodec64 -lavfilter64 -lavformat64 -lavutil64 -lswscale64
    }
}
linux {
    #./configure --enable-shared --enable-libopenjpeg --enable-version3 --enable-libvo-aacenc --enable-libmp3lame --enable-libtwolame --enable-libvpx --enable-libwavpack --enable-libx264 --enable-gpl --enable-libx265 --enable-libvorbis --enable-libtheora --enable-libwebp --enable-libopus
    QMAKE_LFLAGS += -z defs
    LIBS += -lavcodec -lavfilter -lavformat -lavutil -lswscale -lswresample -lz -lmp3lame -lx264 -lx265 -lopenjpeg -ltwolame -lvorbis -lwebp -lwavpack -ltheoraenc -ltheoradec -lvorbisenc -lvo-aacenc -lvpx -llzma -lopus
}

HEADERS = \
    Audio.h \
    FFmpeg.h \
    Identify.h \
    Plugin.h \
    Preview.h \
    Video.h
SOURCES = \
    Audio.cpp \
    Identify.cpp \
    Plugin.cpp \
    Preview.cpp \
    Video.cpp
OTHER_FILES = Configuration.xml

OBJECTS_DIR = tmp
RCC_DIR = tmp
MOC_DIR = tmp
