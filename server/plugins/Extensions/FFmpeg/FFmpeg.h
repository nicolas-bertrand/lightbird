#ifndef FFMPEG_H
# define FFMPEG_H

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
