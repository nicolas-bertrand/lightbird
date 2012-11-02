#include <QtPlugin>

#include "Plugin.h"

Plugin::Plugin()
{
    this->inputFormat = NULL;
    this->outputFormat = NULL;
    this->source = "green.avi";
    this->source = "big_buck_bunny.mov";
    this->destination = "LightBird.mkv";
    this->codecId = AV_CODEC_ID_VP8;
    this->inputVideoStream = NULL;
    this->outputVideoStream = NULL;
    this->decoderContext = NULL;
    this->encoderContext = NULL;
    this->frame = NULL;
    this->framePts = 0;
}

Plugin::~Plugin()
{
}
#include <iostream>
bool    Plugin::onLoad(LightBird::IApi *api)
{
    AVCodec *encoder;

    this->api = api;
    avcodec_register_all();
    av_register_all();
    av_log_set_level(AV_LOG_FATAL);

    if (avformat_open_input(&this->inputFormat, this->source, NULL, NULL) < 0)
    {
        LOG_FATAL("Could not open source file");
        return (false);
    }
    if (avformat_find_stream_info(this->inputFormat, NULL) < 0)
    {
        LOG_FATAL("Could not find stream information");
        return (false);
    }
    if (!(this->inputVideoStream = this->_openStream(AVMEDIA_TYPE_VIDEO)))
    {
        LOG_FATAL("Could not open the video stream");
        return (false);
    }
    this->decoderContext = this->inputVideoStream->codec;

    if (!(encoder = avcodec_find_encoder(this->codecId)))
    {
        LOG_FATAL("Encoder not found");
        return (false);
    }
    if (avformat_alloc_output_context2(&this->outputFormat, NULL, NULL, this->destination) < 0 || !this->outputFormat)
    {
        LOG_FATAL("Could not deduce output format from file extension");
        return (false);
    }
    if (!(this->outputVideoStream = avformat_new_stream(this->outputFormat, encoder)))
    {
        LOG_FATAL("Could not allocate output video stream");
        return (false);
    }
    this->outputVideoStream->id = this->outputFormat->nb_streams - 1;
    this->outputVideoStream->disposition = this->inputVideoStream->disposition;
    this->encoderContext = this->outputVideoStream->codec;
    this->encoderContext->width = this->decoderContext->width;
    this->encoderContext->height = this->decoderContext->height;
    this->encoderContext->pix_fmt = this->decoderContext->pix_fmt;
    this->encoderContext->bits_per_raw_sample = this->decoderContext->bits_per_raw_sample;
    this->encoderContext->chroma_sample_location = this->decoderContext->chroma_sample_location;
    this->encoderContext->bit_rate = 200000;
    this->encoderContext->time_base = (AVRational){ 1, 24 }; // stream r_frame_rate av_inv_q / av_buffersink_get_frame_rate
    this->encoderContext->codec_type = AVMEDIA_TYPE_VIDEO;
    //this->encoderContext->gop_size = 10;
    //this->encoderContext->max_b_frames = 1;
    if (this->outputFormat->oformat->flags & AVFMT_GLOBALHEADER)
        this->encoderContext->flags |= CODEC_FLAG_GLOBAL_HEADER;
    //this->outputVideoStream->duration = (quint64)(this->inputVideoStream->duration * this->inputVideoStream->time_base.num / (double)this->inputVideoStream->time_base.den * ((double)this->outputVideoStream->time_base.den / (double)this->outputVideoStream->time_base.num));
    //this->outputVideoStream->nb_frames = this->inputVideoStream->nb_frames;
    if(this->codecId == AV_CODEC_ID_H264)
        av_opt_set(this->encoderContext->priv_data, "preset", "fast", 0);
    if (avcodec_open2(this->encoderContext, encoder, NULL) < 0)
    {
        LOG_FATAL("Could not open encoder");
        return (false);
    }
    if (!(this->outputFormat->oformat->flags & AVFMT_NOFILE)
        && avio_open(&this->outputFormat->pb, this->destination, AVIO_FLAG_WRITE) < 0)
    {
        LOG_FATAL("Could not open the file");
        return (false);
    }
    if (avformat_write_header(this->outputFormat, NULL) < 0)
    {
        LOG_FATAL("Error occurred when opening output file");
        return (false);
    }
    if (!(this->frame = avcodec_alloc_frame()))
    {
        LOG_FATAL("Could not allocate the frame");
        return (false);
    }
    // read frames from the file
    while (av_read_frame(this->inputFormat, &this->packetIn) >= 0)
    {
        if (this->packetIn.stream_index == this->inputVideoStream->index)
            this->_transcode(AVMEDIA_TYPE_VIDEO);
        av_free_packet(&this->packetIn);
    }
    // flush cached frames
    this->packetIn.data = NULL;
    this->packetIn.size = 0;
    while (this->_transcode(AVMEDIA_TYPE_VIDEO))
        ;
    if (av_write_trailer(this->outputFormat) < 0)
    {
        LOG_FATAL("Error occurred when closing output file");
        return (false);
    }

    // Clean up
    avcodec_free_frame(&this->frame);
    if (!(this->outputFormat->oformat->flags & AVFMT_NOFILE))
        avio_closep(&this->outputFormat->pb);
    avcodec_close(this->decoderContext);
    avcodec_close(this->encoderContext);
    avformat_close_input(&this->inputFormat);
    avformat_close_input(&this->outputFormat);
    return (true);
}

void    Plugin::onUnload()
{
}

bool    Plugin::onInstall(LightBird::IApi *api)
{
    this->api = api;
    return (true);
}

void    Plugin::onUninstall(LightBird::IApi *api)
{
    this->api = api;
}

void    Plugin::getMetadata(LightBird::IMetadata &metadata) const
{
    metadata.name = "FFmpeg";
    metadata.brief = "Implements the IVideo and IAudio extensions using FFmpeg.";
    metadata.description = "Implements the IVideo and IAudio extensions using FFmpeg.";
    metadata.autor = "LightBird team";
    metadata.site = "lightbird.cc";
    metadata.email = "team@lightbird.cc";
    metadata.version = "1.0";
    metadata.licence = "CC BY-NC-SA 3.0";
}

QStringList Plugin::getExtensionsNames()
{
    return (QStringList());
}

void        *Plugin::getExtension(const QString &name)
{
    return (NULL);
}

void        Plugin::releaseExtension(const QString &, void *)
{
}

AVStream *Plugin::_openStream(enum AVMediaType type)
{
    int            result = -1;
    AVStream       *stream = NULL;
    AVCodec        *decoder = NULL;

    if ((result = av_find_best_stream(this->inputFormat, type, -1, -1, &decoder, 0)) < 0)
    {
        LOG_FATAL(QString("Could not find %s stream in input file").arg(av_get_media_type_string(type)));
        return (NULL);
    }
    stream = this->inputFormat->streams[result];
    if ((result = avcodec_open2(stream->codec, decoder, NULL)))
    {
        LOG_FATAL(QString("Failed to open %1 codec").arg(av_get_media_type_string(type)));
        return (NULL);
    }
    return (stream);
}

bool    Plugin::_transcode(AVMediaType type)
{
    int gotFrame;
    int gotOutput;

    if (type == AVMEDIA_TYPE_VIDEO)
    {
        if (avcodec_decode_video2(this->decoderContext, this->frame, &gotFrame, &this->packetIn) < 0)
            LOG_FATAL("Could not decode the video frame");
        av_init_packet(&this->packetOut);
        this->packetOut.data = NULL;
        this->packetOut.size = 0;
        if (gotFrame)
        {
            this->_swap(this->frame->pts, framePts);
            if (avcodec_encode_video2(this->encoderContext, &this->packetOut, this->frame, &gotOutput) < 0)
            {
                LOG_FATAL("Error encoding frame");
                return (false);
            }
            this->_swap(this->frame->pts, framePts);
            if (gotOutput)
            {
                std::cout << "time:" << (float)outputVideoStream->pts.val * outputVideoStream->time_base.num / (float)outputVideoStream->time_base.den
                          << " timebase:" << outputVideoStream->time_base.num << "/" << outputVideoStream->time_base.den << " frame:" << framePts << " rescale:" << av_rescale_q(1, outputVideoStream->codec->time_base, outputVideoStream->time_base)
                          << " pts:" << this->packetOut.pts << " dts:" << this->packetOut.dts << " duration:" << this->packetOut.duration << std::endl;
                if (this->encoderContext->coded_frame->key_frame)
                    this->packetOut.flags |= AV_PKT_FLAG_KEY;
                this->packetOut.stream_index = this->outputVideoStream->index;
                if ((av_interleaved_write_frame(this->outputFormat, &this->packetOut)) < 0)
                {
                    LOG_FATAL("Could not write the frame");
                    return (false);
                }
                av_free_packet(&this->packetOut);
            }
            framePts += av_rescale_q(1, outputVideoStream->codec->time_base, outputVideoStream->time_base);
            return (true);
        }
    }
    return (false);
}

void    Plugin::_swap(qint64 &a, qint64 &b)
{
    qint64 c = a;

    a = b;
    b = c;
}

Q_EXPORT_PLUGIN2(plugin, Plugin)
