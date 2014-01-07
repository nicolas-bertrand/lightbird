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
LIBS += -lCORE_DB_bzlib_ -lCORE_DB_glib_ -lCORE_DB_jbig_ -lCORE_DB_jp2_ -lCORE_DB_jpeg_ -lCORE_DB_lcms_ -lCORE_DB_librsvg_ -lCORE_DB_libxml_ -lCORE_DB_lqr_ -lCORE_DB_Magick++_ -lCORE_DB_magick_ -lCORE_DB_pango_ -lCORE_DB_png_ -lCORE_DB_tiff_ -lCORE_DB_ttf_ -lCORE_DB_wand_ -lCORE_DB_webp_ -lCORE_DB_zlib_
LIBS += -lIM_MOD_DB_aai_ -lIM_MOD_DB_art_ -lIM_MOD_DB_avs_ -lIM_MOD_DB_bgr_ -lIM_MOD_DB_bmp_ -lIM_MOD_DB_braille_ -lIM_MOD_DB_cals_ -lIM_MOD_DB_caption_ -lIM_MOD_DB_cin_ -lIM_MOD_DB_cip_ -lIM_MOD_DB_clipboard_ -lIM_MOD_DB_clip_ -lIM_MOD_DB_cmyk_ -lIM_MOD_DB_cut_ -lIM_MOD_DB_dcm_ -lIM_MOD_DB_dds_ -lIM_MOD_DB_debug_ -lIM_MOD_DB_dib_ -lIM_MOD_DB_djvu_ -lIM_MOD_DB_dng_ -lIM_MOD_DB_dot_ -lIM_MOD_DB_dps_ -lIM_MOD_DB_dpx_ -lIM_MOD_DB_emf_ -lIM_MOD_DB_ept_ -lIM_MOD_DB_exr_ -lIM_MOD_DB_fax_ -lIM_MOD_DB_fd_ -lIM_MOD_DB_fits_ -lIM_MOD_DB_fpx_ -lIM_MOD_DB_gif_ -lIM_MOD_DB_gradient_ -lIM_MOD_DB_gray_ -lIM_MOD_DB_hald_ -lIM_MOD_DB_hdr_ -lIM_MOD_DB_histogram_ -lIM_MOD_DB_hrz_ -lIM_MOD_DB_html_ -lIM_MOD_DB_icon_ -lIM_MOD_DB_info_ -lIM_MOD_DB_inline_ -lIM_MOD_DB_ipl_ -lIM_MOD_DB_jbig_ -lIM_MOD_DB_jnx_ -lIM_MOD_DB_jp2_ -lIM_MOD_DB_jpeg_ -lIM_MOD_DB_label_ -lIM_MOD_DB_mac_ -lIM_MOD_DB_magick_ -lIM_MOD_DB_map_ -lIM_MOD_DB_mask_ -lIM_MOD_DB_matte_ -lIM_MOD_DB_mat_ -lIM_MOD_DB_meta_ -lIM_MOD_DB_miff_ -lIM_MOD_DB_mono_ -lIM_MOD_DB_mpc_ -lIM_MOD_DB_mpeg_ -lIM_MOD_DB_mpr_ -lIM_MOD_DB_msl_ -lIM_MOD_DB_mtv_ -lIM_MOD_DB_mvg_ -lIM_MOD_DB_null_ -lIM_MOD_DB_otb_ -lIM_MOD_DB_palm_ -lIM_MOD_DB_pango_ -lIM_MOD_DB_pattern_ -lIM_MOD_DB_pcd_ -lIM_MOD_DB_pcl_ -lIM_MOD_DB_pcx_ -lIM_MOD_DB_pdb_ -lIM_MOD_DB_pdf_ -lIM_MOD_DB_pes_ -lIM_MOD_DB_pict_ -lIM_MOD_DB_pix_ -lIM_MOD_DB_plasma_ -lIM_MOD_DB_png_ -lIM_MOD_DB_pnm_ -lIM_MOD_DB_preview_ -lIM_MOD_DB_ps2_ -lIM_MOD_DB_ps3_ -lIM_MOD_DB_psd_ -lIM_MOD_DB_ps_ -lIM_MOD_DB_pwp_ -lIM_MOD_DB_raw_ -lIM_MOD_DB_rgb_ -lIM_MOD_DB_rgf_ -lIM_MOD_DB_rla_ -lIM_MOD_DB_rle_ -lIM_MOD_DB_scr_ -lIM_MOD_DB_sct_ -lIM_MOD_DB_sfw_ -lIM_MOD_DB_sgi_ -lIM_MOD_DB_stegano_ -lIM_MOD_DB_sun_ -lIM_MOD_DB_svg_ -lIM_MOD_DB_tga_ -lIM_MOD_DB_thumbnail_ -lIM_MOD_DB_tiff_ -lIM_MOD_DB_tile_ -lIM_MOD_DB_tim_ -lIM_MOD_DB_ttf_ -lIM_MOD_DB_txt_ -lIM_MOD_DB_uil_ -lIM_MOD_DB_url_ -lIM_MOD_DB_uyvy_ -lIM_MOD_DB_vicar_ -lIM_MOD_DB_vid_ -lIM_MOD_DB_viff_ -lIM_MOD_DB_wbmp_ -lIM_MOD_DB_webp_ -lIM_MOD_DB_wmf_ -lIM_MOD_DB_wpg_ -lIM_MOD_DB_xbm_ -lIM_MOD_DB_xcf_ -lIM_MOD_DB_xc_ -lIM_MOD_DB_xpm_ -lIM_MOD_DB_xps_ -lIM_MOD_DB_xtrn_ -lIM_MOD_DB_ycbcr_ -lIM_MOD_DB_yuv_

DEFINES += _DLL
win32 { DEFINES += _VISUALC_ _WINDOWS }

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
