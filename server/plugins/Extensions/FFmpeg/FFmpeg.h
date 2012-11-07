#ifndef FFMPEG_H
# define FFMPEG_H

// INT64_C and UINT64_C are not defined in C++ and used by FFmpeg
#define INT64_C(c) (c ## LL)
#define UINT64_C(c) (c ## ULL)
// FFmpeg is a C library
extern "C"
{
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
    #include <libavfilter/avfiltergraph.h>
    #include <libavfilter/avcodec.h>
    #include <libavfilter/buffersrc.h>
    #include <libavfilter/buffersink.h>
    #include <libavutil/imgutils.h>
    #include <libavutil/timestamp.h>
    #include <libavutil/log.h>
    #include <libavutil/opt.h>
    #include <libswscale/swscale.h>
}

#endif // FFMPEG_H
