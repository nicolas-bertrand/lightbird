#include <QtPlugin>

#include "Plugin.h"

Plugin::Plugin()
{
    this->identify = NULL;
}

Plugin::~Plugin()
{
}

bool    Plugin::onLoad(LightBird::IApi *api)
{
    this->api = api;
    avcodec_register_all();
    av_register_all();
    avfilter_register_all();
    this->_loadConfiguration();
    this->identify = new Identify(this->api);
    return (true);
}

void    Plugin::onUnload()
{
    delete this->identify;
    this->identify = NULL;
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
    return (QStringList() << "IIdentify");
}

void    *Plugin::getExtension(const QString &name)
{
    if (name == "IIdentify")
        return (dynamic_cast<LightBird::IIdentify *>(this->identify));
    return (NULL);
}

void    Plugin::releaseExtension(const QString &, void *)
{
}

void    Plugin::_loadConfiguration()
{
    QMap<QString, int>  logLevels;
    QString             logLevel;

    logLevels.insert("QUIET"  , AV_LOG_QUIET);
    logLevels.insert("PANIC"  , AV_LOG_PANIC);
    logLevels.insert("FATAL"  , AV_LOG_FATAL);
    logLevels.insert("ERROR"  , AV_LOG_ERROR);
    logLevels.insert("WARNING", AV_LOG_WARNING);
    logLevels.insert("INFO"   , AV_LOG_INFO);
    logLevels.insert("VERBOSE", AV_LOG_VERBOSE);
    logLevels.insert("DEBUG"  , AV_LOG_DEBUG);
    if ((logLevel = this->api->configuration(true).get("av_log_level")).isEmpty() || !logLevels.contains(logLevel))
        logLevel = "FATAL";
    av_log_set_level(logLevels.value(logLevel));
}

bool    Plugin::_transcode()
{
    this->formatIn = NULL;
    this->formatOut = NULL;
    this->source = "big_buck_bunny.ogg";
    this->destination = "LightBird.mkv";
    this->audioCodecId = AV_CODEC_ID_VORBIS;
    this->videoCodecId = AV_CODEC_ID_VP8;
    this->audioStreamIn = NULL;
    this->audioStreamOut = NULL;
    this->videoStreamIn = NULL;
    this->videoStreamOut = NULL;
    this->audioDec = NULL;
    this->audioEnc = NULL;
    this->videoDec = NULL;
    this->videoEnc = NULL;
    this->frame = NULL;
    this->audioFramePts = 0;
    this->videoFramePts = 0;
    this->audioFilterGraph = NULL;
    this->videoFilterGraph = NULL;

    // Opens the input file
    if (avformat_open_input(&this->formatIn, this->source, NULL, NULL) < 0)
    {
        LOG_ERROR("Could not open source file");
        return (false);
    }
    if (avformat_find_stream_info(this->formatIn, NULL) < 0)
    {
        LOG_ERROR("Could not find stream information");
        return (false);
    }
    // Opens the output file
    if (avformat_alloc_output_context2(&this->formatOut, NULL, NULL, this->destination) < 0 || !this->formatOut)
    {
        LOG_ERROR("Could not deduce output format from file extension");
        return (false);
    }
    if ((this->audioStreamIn = this->_openStream(this->audioDec, AVMEDIA_TYPE_AUDIO)))
        if (!this->_openAudioEncoder())
            return (false);
    if ((this->videoStreamIn = this->_openStream(this->videoDec, AVMEDIA_TYPE_VIDEO)))
        if (!this->_openVideoEncoder())
            return (false);
    if (!(this->formatOut->oformat->flags & AVFMT_NOFILE)
        && avio_open(&this->formatOut->pb, this->destination, AVIO_FLAG_WRITE) < 0)
    {
        LOG_ERROR("Could not open the file");
        return (false);
    }
    // Writes the format header
    if (avformat_write_header(this->formatOut, NULL) < 0)
    {
        LOG_ERROR("Error occurred when opening output file");
        return (false);
    }
    if (!(this->frame = avcodec_alloc_frame()))
    {
        LOG_ERROR("Could not allocate the frame");
        return (false);
    }
    // Read frames from the file and transcode them
    while (av_read_frame(this->formatIn, &this->packetIn) >= 0)
    {
        if (this->audioStreamIn && this->packetIn.stream_index == this->audioStreamIn->index)
            this->_transcodeAudio();
        if (this->videoStreamIn && this->packetIn.stream_index == this->videoStreamIn->index)
            this->_transcodeVideo();
        av_free_packet(&this->packetIn);
    }
    // Flush cached frames
    this->packetIn.data = NULL;
    this->packetIn.size = 0;
    if (this->audioStreamIn)
        this->_transcodeAudio(true);
    if (this->videoStreamIn)
        while (this->_transcodeVideo())
            ;
    // Writes the format trailer
    if (av_write_trailer(this->formatOut) < 0)
    {
        LOG_ERROR("Error occurred when closing output file");
        return (false);
    }
    // Clean up
    avcodec_free_frame(&this->frame);
    if (!(this->formatOut->oformat->flags & AVFMT_NOFILE))
        avio_closep(&this->formatOut->pb);
    avfilter_graph_free(&this->audioFilterGraph);
    avfilter_graph_free(&this->videoFilterGraph);
    avcodec_close(this->audioDec);
    avcodec_close(this->audioEnc);
    avcodec_close(this->videoDec);
    avcodec_close(this->videoEnc);
    avformat_close_input(&this->formatIn);
    avformat_close_input(&this->formatOut);
    return (true);
}

AVStream    *Plugin::_openStream(AVCodecContext *&context, enum AVMediaType type)
{
    int      result = -1;
    AVStream *stream = NULL;
    AVCodec  *decoder = NULL;

    if ((result = av_find_best_stream(this->formatIn, type, -1, -1, &decoder, 0)) < 0)
    {
        LOG_ERROR(QString("Could not find %1 stream in input file").arg(av_get_media_type_string(type)));
        return (NULL);
    }
    stream = this->formatIn->streams[result];
    if ((result = avcodec_open2(stream->codec, decoder, NULL)))
    {
        LOG_ERROR(QString("Failed to open %1 codec").arg(av_get_media_type_string(type)));
        return (NULL);
    }
    context = stream->codec;
    return (stream);
}

bool    Plugin::_openAudioEncoder()
{
    AVCodec *encoder;

    if (!(encoder = avcodec_find_encoder(this->audioCodecId)))
    {
        LOG_ERROR("Audio encoder not found");
        return (false);
    }
    if (!(this->audioStreamOut = avformat_new_stream(this->formatOut, encoder)))
    {
        LOG_ERROR("Could not allocate output audio stream");
        return (false);
    }
    this->audioStreamOut->id = this->audioStreamOut->index;
    this->audioEnc = this->audioStreamOut->codec;
    this->audioEnc->codec = encoder;
    if (!this->_configureAudioFilter())
        return (false);
    this->audioEnc->sample_fmt = (AVSampleFormat)this->audioFilterOut->inputs[0]->format;
    this->audioEnc->sample_rate = this->audioFilterOut->inputs[0]->sample_rate;
    this->audioEnc->channel_layout = this->audioFilterOut->inputs[0]->channel_layout;
    this->audioEnc->channels = av_get_channel_layout_nb_channels(this->audioEnc->channel_layout);
    this->audioEnc->time_base = (AVRational){ 1, this->audioDec->sample_rate };
    if (this->formatOut->oformat->flags & AVFMT_GLOBALHEADER)
        this->audioEnc->flags |= CODEC_FLAG_GLOBAL_HEADER;
    this->audioEnc->bit_rate = this->audioDec->bit_rate;
    if (avcodec_open2(this->audioEnc, encoder, NULL) < 0)
    {
        LOG_ERROR("Could not open audio encoder");
        return (false);
    }
    if (!(this->audioEnc->codec->capabilities & CODEC_CAP_VARIABLE_FRAME_SIZE))
        av_buffersink_set_frame_size(this->audioFilterOut, this->audioEnc->frame_size);
    return (true);
}

bool    Plugin::_openVideoEncoder()
{
    AVCodec *encoder;

    if (!(encoder = avcodec_find_encoder(this->videoCodecId)))
    {
        LOG_ERROR("Video encoder not found");
        return (false);
    }
    if (!(this->videoStreamOut = avformat_new_stream(this->formatOut, encoder)))
    {
        LOG_ERROR("Could not allocate output video stream");
        return (false);
    }
    this->videoStreamOut->id = this->videoStreamOut->index;
    this->videoStreamOut->disposition = this->videoStreamIn->disposition;
    this->videoEnc = this->videoStreamOut->codec;
    this->videoEnc->codec = encoder;
    if (!this->_configureVideoFilter())
        return (false);
    this->videoEnc->width = this->videoFilterOut->inputs[0]->w;
    this->videoEnc->height = this->videoFilterOut->inputs[0]->h;
    this->videoEnc->pix_fmt = (AVPixelFormat)this->videoFilterOut->inputs[0]->format;
    this->videoEnc->sample_aspect_ratio = this->videoFilterOut->inputs[0]->sample_aspect_ratio;
    this->videoEnc->bits_per_raw_sample = this->videoDec->bits_per_raw_sample;
    this->videoEnc->chroma_sample_location = this->videoDec->chroma_sample_location;
    this->videoEnc->bit_rate = this->videoDec->bit_rate ? this->videoDec->bit_rate : 3000000;
    this->videoEnc->time_base = av_inv_q(av_buffersink_get_frame_rate(this->videoFilterOut));
    //this->videoEnc->gop_size = 10;
    //this->videoEnc->max_b_frames = 2;
    if (this->formatOut->oformat->flags & AVFMT_GLOBALHEADER)
        this->videoEnc->flags |= CODEC_FLAG_GLOBAL_HEADER;
    if(this->videoCodecId == AV_CODEC_ID_H264)
        av_opt_set(this->videoEnc->priv_data, "preset", "fast", 0);
    if (avcodec_open2(this->videoEnc, encoder, NULL) < 0)
    {
        LOG_ERROR("Could not open video encoder");
        return (false);
    }
    return (true);
}

bool    Plugin::_configureAudioFilter()
{
    AVFilterInOut *inputs, *outputs;

    this->audioFilterGraph = avfilter_graph_alloc();
    if (avfilter_graph_parse2(this->audioFilterGraph, "anull", &inputs, &outputs) < 0
        || !inputs || inputs->next || !outputs || outputs->next)
    {
        LOG_ERROR("The audio filter graph does not have exactly one input and output");
        return (false);
    }
    if (!this->_configureInputAudioFilter(inputs))
        return (false);
    if (!this->_configureOutputAudioFilter(outputs))
        return (false);
    if (avfilter_graph_config(this->audioFilterGraph, NULL) < 0)
    {
        LOG_ERROR("Could not configure the audio filter graph");
        return (false);
    }
    return (true);
}

bool    Plugin::_configureInputAudioFilter(AVFilterInOut *inputs)
{
    QByteArray  args;

    args = QString("time_base=1/%1:sample_rate=%1:sample_fmt=%2:channel_layout=0x%3")
                  .arg(QString::number(this->audioDec->sample_rate),
                       av_get_sample_fmt_name(this->audioDec->sample_fmt),
                       QString::number(this->audioDec->channel_layout, 16)).toAscii();
    if (avfilter_graph_create_filter(&this->audioFilterIn, avfilter_get_by_name("abuffer"), "abuffer", args.data(), NULL, this->audioFilterGraph) < 0)
    {
        LOG_ERROR("Could not create the abuffer filter");
        return (false);
    }
    if (avfilter_link(this->audioFilterIn, 0, inputs->filter_ctx, inputs->pad_idx) < 0)
    {
        LOG_ERROR("Could not link the abuffer and anull filters");
        return (false);
    }
    avfilter_inout_free(&inputs);
    return (true);
}

bool    Plugin::_configureOutputAudioFilter(AVFilterInOut *outputs)
{
    AVFilterContext *aformat;
    QByteArray      args;
    QByteArray      arg;

    if (avfilter_graph_create_filter(&this->audioFilterOut, avfilter_get_by_name("ffabuffersink"), "ffabuffersink", NULL, NULL, this->audioFilterGraph) < 0)
    {
        LOG_ERROR("Could not create the ffabuffersink filter");
        return (false);
    }
    if (!(arg = this->_getSampleFormats(this->audioEnc->codec)).isEmpty())
        args += "sample_fmts=" + arg;
    if (!(arg = this->_getSampleRates(this->audioEnc->codec)).isEmpty())
        args += "sample_rates=" + arg;
    if (!(arg = this->_getChannelLayouts(this->audioEnc->codec)).isEmpty())
        args += "channel_layouts=" + arg;
    if (avfilter_graph_create_filter(&aformat, avfilter_get_by_name("aformat"), "aformat", args.data(), NULL, this->audioFilterGraph) < 0)
    {
        LOG_ERROR("Could not create the aformat filter");
        return (false);
    }
    if (avfilter_link(outputs->filter_ctx, outputs->pad_idx, aformat, 0) < 0)
    {
        LOG_ERROR("Could not link the aformat and anull filters");
        return (false);
    }
    if (avfilter_link(aformat, 0, this->audioFilterOut, 0) < 0)
    {
        LOG_ERROR("Could not link the aformat and ffabuffersink filters");
        return (false);
    }
    avfilter_inout_free(&outputs);
    return (true);
}

bool    Plugin::_configureVideoFilter()
{
    AVFilterInOut *inputs, *outputs;

    this->videoFilterGraph = avfilter_graph_alloc();
    if (avfilter_graph_parse2(this->videoFilterGraph, "null", &inputs, &outputs) < 0
        || !inputs || inputs->next || !outputs || outputs->next)
    {
        LOG_ERROR("The video filter graph does not have exactly one input and output");
        return (false);
    }
    if (!this->_configureInputVideoFilter(inputs))
        return (false);
    if (!this->_configureOutputVideoFilter(outputs))
        return (false);
    if (avfilter_graph_config(this->videoFilterGraph, NULL) < 0)
    {
        LOG_ERROR("Could not configure the video filter graph");
        return (false);
    }
    return (true);
}

bool    Plugin::_configureInputVideoFilter(AVFilterInOut *inputs)
{
    QString args("video_size=%1x%2:pix_fmt=%3:time_base=%4/%5:pixel_aspect=0/1:sws_param=flags=%6");

    args = args.arg(QString::number(this->videoDec->width), QString::number(this->videoDec->height),
                    QString::number(this->videoDec->pix_fmt),
                    QString::number(this->videoStreamIn->time_base.num), QString::number(this->videoStreamIn->time_base.den),
                    QString::number(SWS_BILINEAR + ((this->videoDec->flags & CODEC_FLAG_BITEXACT) ? SWS_BITEXACT : 0)));
    if (this->videoStreamIn->r_frame_rate.num && this->videoStreamIn->r_frame_rate.den)
        args.append(QString(":frame_rate=%1/%2").arg(QString::number(this->videoStreamIn->r_frame_rate.num), QString::number(this->videoStreamIn->r_frame_rate.den)));
    if (avfilter_graph_create_filter(&this->videoFilterIn, avfilter_get_by_name("buffer"), "buffer", args.toAscii().data(), NULL, this->videoFilterGraph) < 0)
    {
        LOG_ERROR("Could not create the abuffer filter");
        return (false);
    }
    if (avfilter_link(this->videoFilterIn, 0, inputs->filter_ctx, inputs->pad_idx) < 0)
    {
        LOG_ERROR("Could not link the abuffer and anull filters");
        return (false);
    }
    avfilter_inout_free(&inputs);
    return (true);
}

bool    Plugin::_configureOutputVideoFilter(AVFilterInOut *outputs)
{
    AVFilterContext *format;
    AVFilterContext *scale;
    QByteArray      args;
    QByteArray      pixelFormats;
    AVFilterContext *lastFilter = outputs->filter_ctx;

    if (avfilter_graph_create_filter(&this->videoFilterOut, avfilter_get_by_name("ffbuffersink"), "ffbuffersink", NULL, NULL, this->videoFilterGraph) < 0)
    {
        LOG_ERROR("Could not create the ffbuffersink filter");
        return (false);
    }
    if (this->videoEnc->width || this->videoEnc->height)
    {
        args = QString("%1:%2:flags=0x%3").arg(QString::number(this->videoEnc->width),
               QString::number(this->videoEnc->height), QString::number(SWS_BICUBIC, 16)).toAscii();
        if (avfilter_graph_create_filter(&scale, avfilter_get_by_name("scale"), "scale", args.data(), NULL, this->videoFilterGraph) < 0)
        {
            LOG_ERROR("Could not create the scale filter");
            return (false);
        }
        if (avfilter_link(lastFilter, 0, scale, 0) < 0)
        {
            LOG_ERROR("Could not link the scale and anull filters");
            return (false);
        }
        lastFilter = scale;
    }
    if (!(pixelFormats = this->_getPixelFormats(this->videoEnc->codec)).isEmpty())
    {
        if (avfilter_graph_create_filter(&format, avfilter_get_by_name("format"), "format", pixelFormats.data(), NULL, this->videoFilterGraph) < 0)
        {
            LOG_ERROR("Could not create the format filter" + pixelFormats);
            return (false);
        }
        if (avfilter_link(lastFilter, 0, format, 0) < 0)
        {
            LOG_ERROR("Could not link the format and anull filters");
            return (false);
        }
        lastFilter = format;
    }
    if (avfilter_link(lastFilter, 0, this->videoFilterOut, 0) < 0)
    {
        LOG_ERROR("Could not link the format and ffbuffersink filters");
        return (false);
    }
    avfilter_inout_free(&outputs);
    return (true);
}

void         Plugin::_transcodeAudio(bool flush)
{
    AVPacket packet = this->packetIn;
    int      gotFrame = 1;
    int      gotPacket;
    int      result;
    AVFilterBufferRef *bufferRef = NULL;

    while ((packet.size > 0 || (flush && gotFrame)))
    {
        if ((result = avcodec_decode_audio4(this->audioDec, this->frame, &gotFrame, &packet)) < 0)
        {
            LOG_ERROR("Could not decode the audio frame");
            return ;
        }
        packet.data += result;
        packet.size -= result;
        if (gotFrame)
        {
            if (av_buffersrc_add_frame(this->audioFilterIn, this->frame, AV_BUFFERSRC_FLAG_PUSH) < 0)
            {
                LOG_ERROR("Error while feeding the audio filtergraph");
                return ;
            }
            while (true)
            {
                if ((result = av_buffersink_get_buffer_ref(this->audioFilterOut, &bufferRef, AV_BUFFERSINK_FLAG_NO_REQUEST)) < 0)
                {
                    if (result != AVERROR(EAGAIN) && result != AVERROR_EOF)
                    {
                        LOG_ERROR("Error while getting the output of the audio filtergraph");
                        return ;
                    }
                    break;
                }
                avfilter_copy_buf_props(this->frame, bufferRef);
                av_init_packet(&this->packetOut);
                this->packetOut.data = NULL;
                this->packetOut.size = 0;
                this->frame->pts = this->audioFramePts;
                if (avcodec_encode_audio2(this->audioEnc, &this->packetOut, this->frame, &gotPacket) < 0)
                {
                    LOG_ERROR("Error encoding audio frame");
                    return ;
                }
                this->audioFramePts += this->frame->nb_samples;
                if (gotPacket)
                {
                    if (this->packetOut.pts != AV_NOPTS_VALUE)
                        this->packetOut.pts = av_rescale_q(this->packetOut.pts, this->audioEnc->time_base, this->audioStreamOut->time_base);
                    if (this->packetOut.dts != AV_NOPTS_VALUE)
                        this->packetOut.dts = av_rescale_q(this->packetOut.dts, this->audioEnc->time_base, this->audioStreamOut->time_base);
                    if (this->packetOut.duration > 0)
                        this->packetOut.duration = av_rescale_q(this->packetOut.duration, this->audioEnc->time_base, this->audioStreamOut->time_base);
                    this->packetOut.stream_index = this->audioStreamOut->index;
                    if ((av_interleaved_write_frame(this->formatOut, &this->packetOut)) < 0)
                    {
                        LOG_ERROR("Could not write audio frame");
                        return ;
                    }
                    av_free_packet(&this->packetOut);
                }
                avfilter_unref_bufferp(&bufferRef);
            }
        }
    }
}

bool    Plugin::_transcodeVideo()
{
    int gotFrame;
    int gotPacket;
    int result;
    AVFilterBufferRef *bufferRef = NULL;

    if (avcodec_decode_video2(this->videoDec, this->frame, &gotFrame, &this->packetIn) < 0)
        LOG_ERROR("Could not decode the video frame");
    if (gotFrame)
    {
        if (av_buffersrc_add_frame(this->videoFilterIn, this->frame, AV_BUFFERSRC_FLAG_PUSH) < 0)
        {
            LOG_ERROR("Error while feeding the video filtergraph");
            return (false);
        }
        while (true)
        {
            if ((result = av_buffersink_get_buffer_ref(this->videoFilterOut, &bufferRef, AV_BUFFERSINK_FLAG_NO_REQUEST)) < 0)
            {
                if (result != AVERROR(EAGAIN) && result != AVERROR_EOF)
                {
                    LOG_ERROR("Error while getting the output of the video filtergraph");
                    return (false);
                }
                break;
            }
            avfilter_copy_buf_props(this->frame, bufferRef);
            av_init_packet(&this->packetOut);
            this->packetOut.data = NULL;
            this->packetOut.size = 0;
            this->frame->pts = videoFramePts;
            if (avcodec_encode_video2(this->videoEnc, &this->packetOut, this->frame, &gotPacket) < 0)
            {
                LOG_ERROR("Error encoding video frame");
                return (false);
            }
            if (gotPacket)
            {
                if (this->videoEnc->coded_frame->key_frame)
                    this->packetOut.flags |= AV_PKT_FLAG_KEY;
                if (this->packetOut.pts != AV_NOPTS_VALUE)
                    this->packetOut.pts = av_rescale_q(this->packetOut.dts, this->videoEnc->time_base, this->videoStreamOut->time_base);
                if (this->packetOut.dts != AV_NOPTS_VALUE)
                    this->packetOut.dts = av_rescale_q(this->packetOut.dts, this->videoEnc->time_base, this->videoStreamOut->time_base);
                this->packetOut.stream_index = this->videoStreamOut->index;
                if ((av_interleaved_write_frame(this->formatOut, &this->packetOut)) < 0)
                {
                    LOG_ERROR("Could not write video frame");
                    return (false);
                }
                av_free_packet(&this->packetOut);
            }
            this->videoFramePts++;
            avfilter_unref_bufferp(&bufferRef);
        }
    }
    return (gotFrame != 0);
}

QByteArray  Plugin::_getSampleFormats(const AVCodec *codec)
{
    const AVSampleFormat *p = codec->sample_fmts;
    QByteArray           result;

    while (p && *p != AV_SAMPLE_FMT_NONE)
    {
        if (!result.isEmpty())
            result.append(",");
        result.append(av_get_sample_fmt_name(*(p++)));
    }
    return (result);
}

QByteArray  Plugin::_getSampleRates(const AVCodec *codec)
{
    const int   *p = codec->supported_samplerates;
    QByteArray  result;

    while (p && *p != 0)
    {
        if (!result.isEmpty())
            result.append(",");
        result.append(QString::number(*(p++)));
    }
    return (result);
}

QByteArray  Plugin::_getChannelLayouts(const AVCodec *codec)
{
    const quint64 *p = codec->channel_layouts;
    QByteArray    result;

    while (p && *p != 0)
    {
        if (!result.isEmpty())
            result.append(",");
        result.append("0x" + QString::number(*(p++), 16));
    }
    return (result);
}

QByteArray  Plugin::_getPixelFormats(const AVCodec *codec) const
{
    const AVPixelFormat *p = codec->pix_fmts;
    QByteArray          result;

    while (p && *p != AV_PIX_FMT_NONE)
    {
        if (!result.isEmpty())
            result.append(":");
        result.append(av_get_pix_fmt_name(*(p++)));
    }
    return (result);
}

Q_EXPORT_PLUGIN2(plugin, Plugin)
