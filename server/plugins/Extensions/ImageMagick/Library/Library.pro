TEMPLATE = lib
CONFIG += plugin
QT += network sql xml
RESOURCES = resources.qrc

INCLUDEPATH += \
    ../../../../api \
    ../../../../api/network \
    ../../../../extensions \
    ../../../../library \
    ./include

TARGET = ImageMagick
DESTDIR = ../../../../build/plugins/Extensions/ImageMagick/Library
LIBS += -L../../../../build -lLightBirdLibrary -L../../../../../server/plugins/Extensions/ImageMagick/Library/lib

QT_INSTALL_PREFIX = $$[QT_INSTALL_PREFIX]
X64 = $$find(QT_INSTALL_PREFIX, 64)
isEmpty(X64) {
    CONFIG(debug, debug|release):LIBS += -lCORE_bzlib_32D -lCORE_coders_32D -lCORE_glib_32D -lCORE_jbig_32D -lCORE_jp2_32D -lCORE_jpeg_32D -lCORE_lcms_32D -lCORE_librsvg_32D -lCORE_libxml_32D -lCORE_lqr_32D -lCORE_Magick++_32D -lCORE_magick_32D -lCORE_openjpeg_32D -lCORE_pango_32D -lCORE_png_32D -lCORE_tiff_32D -lCORE_ttf_32D -lCORE_wand_32D -lCORE_webp_32D -lCORE_zlib_32D -lCORE_cairo_32D -lCORE_croco_32D -lCORE_ffi_32D -lCORE_filters_32D -lCORE_pixman_32D
    CONFIG(release, debug|release):LIBS += -lCORE_bzlib_32 -lCORE_coders_32 -lCORE_glib_32 -lCORE_jbig_32 -lCORE_jp2_32 -lCORE_jpeg_32 -lCORE_lcms_32 -lCORE_librsvg_32 -lCORE_libxml_32 -lCORE_lqr_32 -lCORE_Magick++_32 -lCORE_magick_32 -lCORE_openjpeg_32 -lCORE_pango_32 -lCORE_png_32 -lCORE_tiff_32 -lCORE_ttf_32 -lCORE_wand_32 -lCORE_webp_32 -lCORE_zlib_32 -lCORE_cairo_32 -lCORE_croco_32 -lCORE_ffi_32 -lCORE_filters_32 -lCORE_pixman_32
} else {
    CONFIG(debug, debug|release):LIBS += -lCORE_bzlib_64D -lCORE_coders_64D -lCORE_glib_64D -lCORE_jbig_64D -lCORE_jp2_64D -lCORE_jpeg_64D -lCORE_lcms_64D -lCORE_librsvg_64D -lCORE_libxml_64D -lCORE_lqr_64D -lCORE_Magick++_64D -lCORE_magick_64D -lCORE_openjpeg_64D -lCORE_pango_64D -lCORE_png_64D -lCORE_tiff_64D -lCORE_ttf_64D -lCORE_wand_64D -lCORE_webp_64D -lCORE_zlib_64D -lCORE_cairo_64D -lCORE_croco_64D -lCORE_ffi_64D -lCORE_filters_64D -lCORE_pixman_64D
    CONFIG(release, debug|release):LIBS += -lCORE_bzlib_64 -lCORE_coders_64 -lCORE_glib_64 -lCORE_jbig_64 -lCORE_jp2_64 -lCORE_jpeg_64 -lCORE_lcms_64 -lCORE_librsvg_64 -lCORE_libxml_64 -lCORE_lqr_64 -lCORE_Magick++_64 -lCORE_magick_64 -lCORE_openjpeg_64 -lCORE_pango_64 -lCORE_png_64 -lCORE_tiff_64 -lCORE_ttf_64 -lCORE_wand_64 -lCORE_webp_64 -lCORE_zlib_64 -lCORE_cairo_64 -lCORE_croco_64 -lCORE_ffi_64 -lCORE_filters_64 -lCORE_pixman_64
}

DEFINES += _DLL \ # User code is a DLL
    STATIC_MAGICK \ # Using the static version of ImageMagick
    NOAUTOLINK_MAGICK # Prevents the use of pragmas to link the libraries
win32 {
    DEFINES += _VISUALC_ _WINDOWS
    LIBS += Advapi32.lib OleAut32.lib Shell32.lib ole32.lib urlmon.lib
}

HEADERS = \
    Identify.h \
    Image.h \
    Plugin.h
SOURCES = \
    Identify.cpp \
    Image.cpp \
    Plugin.cpp
OTHER_FILES = Configuration.xml \
    Readme.txt

OBJECTS_DIR = tmp
RCC_DIR = tmp
MOC_DIR = tmp
