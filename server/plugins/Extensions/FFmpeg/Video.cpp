#include <QFileInfo>

#include "Plugin.h"
#include "Video.h"

Video::Video(LightBird::IApi *a, QStringList &f)
    : api(a)
    , formats(f)
{
    this->ioBuffer = (unsigned char *)av_malloc(IO_BUFFER_SIZE);
    this->_clear(false, true);
}

Video::~Video()
{
    this->_clear();
    av_free(this->ioBuffer);
}

bool    Video::initialize(const QString &source)
{
    int ret;

    // Frees the allocated structures
    this->_clear();
    // Opens the input file and its streams
    this->source = source;
    if ((ret = avformat_open_input(&this->formatIn, this->source.toUtf8().data(), NULL, NULL)) < 0)
        this->api->log().error("Could not open the source file", this->_getProperties(ret), "Video", "initialize");
    else if ((ret = avformat_find_stream_info(this->formatIn, NULL)) < 0)
        this->api->log().error("Could not find stream information", this->_getProperties(ret), "Video", "initialize");
    else if ((this->videoStreamIn = this->_openStream(this->videoDec, AVMEDIA_TYPE_VIDEO)))
        this->audioStreamIn = this->_openStream(this->audioDec, AVMEDIA_TYPE_AUDIO);
    // The file doesn't have a valid video stream
    if (!this->videoStreamIn)
    {
        this->_clear();
        return (false);
    }
    return (true);
}

QByteArray  Video::transcode()
{
    QByteArray  result;
    int         read = 0;
    int         ret;

    try
    {
        // The transcoding session has not been initialized
        if (!this->formatIn)
            throw false;
        this->buffers.append(QByteArray());
        // Initializes the output format and starts the transcoding
        if (!this->formatOut)
            this->_initializeOutput();
        // Fills the buffers with enough data to allows the seeking
        while (this->buffers.size() < BUFFERS_NUMBER && !this->finished && read >= 0)
        {
            // The number of frames to encode for this buffer to make one second
            this->framesToEncode = (this->time + 1) * av_q2d(this->videoStreamOut->r_frame_rate) - this->videoFramePts;
            // Transcodes one second of the source
            while ((this->framesToEncode > 0 || this->buffers.last().isEmpty()) && (read = av_read_frame(this->formatIn, &this->packetIn)) >= 0)
            {
                if (this->packetIn.stream_index == this->videoStreamIn->index)
                    this->_transcodeVideo();
                if (this->audioStreamIn && this->packetIn.stream_index == this->audioStreamIn->index)
                    this->_transcodeAudio();
                av_free_packet(&this->packetIn);
            }
            this->time++;
            // The transcodage duration has been reached
            if (this->duration && this->duration <= this->videoFramePts)
                read = -1;
            if (read >= 0)
                this->buffers.append(QByteArray());
        }
        // Finishes the transcoding
        if (read < 0)
        {
            this->finished = true;
            // Flushes the cached frames
            this->packetIn.data = NULL;
            this->packetIn.size = 0;
            if (this->audioStreamIn)
                this->_transcodeAudio(true);
            if (this->videoStreamIn)
                while (this->_transcodeVideo())
                    ;
            // Writes the format trailer. This might not work properly since the muxer can try to seek to the header.
            if ((ret = av_write_trailer(this->formatOut)) < 0)
            {
                LOG_ERROR("Could not write the format trailer", this->_getProperties(ret), "Video", "transcode");
                throw false;
            }
        }
        // Returns the first buffer
        if (!this->buffers.isEmpty())
        {
            result = this->buffers.first();
            this->bytesTranscoded += result.size();
            this->buffers.removeFirst();
            if (!this->buffers.isEmpty() && this->buffers.last().isEmpty())
                this->buffers.removeLast();
        }
    }
    catch (bool)
    {
        this->_clear();
    }
    return (result);
}

int     Video::getTime()
{
    if (this->buffers.isEmpty() && this->videoStreamOut && av_q2d(this->videoStreamOut->r_frame_rate))
        return (this->videoFramePts / av_q2d(this->videoStreamOut->r_frame_rate));
    return (this->time - this->buffers.size());
}

void    Video::clear()
{
    this->_clear(true, true);
}

bool    Video::setFormat(const QString &format)
{
    if (!this->formats.contains(format.toLower()))
        return (false);
    this->format = format;
    return (true);
}

bool    Video::setVideoCodec(const QString &c)
{
    QByteArray  codec = c.toLower().toUtf8();
    AVCodec     *encoder;

    // Gets the encoder based on the name, or gets the corresponding encoder if a decoder name is supplied.
    // For example h264 is a decoder and libx264 is the encoder, but they have the same codec ID.
    if (!(encoder = avcodec_find_encoder_by_name(codec.data())) &&
        (encoder = avcodec_find_decoder_by_name(codec.data())))
        encoder = avcodec_find_encoder(encoder->id);
    // Ensures that the encoder is valid
    if (!encoder || encoder->type != AVMEDIA_TYPE_VIDEO)
        return (false);
    this->videoCodec = codec;
    return (true);
}

bool    Video::setAudioCodec(const QString &c)
{
    QByteArray  codec = c.toLower().toUtf8();
    AVCodec     *encoder;

    // Gets the encoder based on the name, or gets the corresponding encoder if a decoder name is supplied.
    if (!(encoder = avcodec_find_encoder_by_name(codec.data())) &&
        (encoder = avcodec_find_decoder_by_name(codec.data())))
        encoder = avcodec_find_encoder(encoder->id);
    // Ensures that the encoder is valid
    if (!encoder || encoder->type != AVMEDIA_TYPE_AUDIO)
        return (false);
    this->audioCodec = codec;
    return (true);
}

void    Video::setWidth(int width)
{
    if (width < -1)
        width = -1;
    this->width = width;
}

void    Video::setHeight(int height)
{
    if (height < -1)
        height = -1;
    this->height = height;
}

void    Video::setVideoBitRate(unsigned int bitRate)
{
    this->videoBitRate = bitRate;
}

void    Video::setAudioBitRate(unsigned int bitRate)
{
    this->audioBitRate = bitRate;
}

void    Video::setFrameRate(unsigned int frameRate)
{
    this->frameRate = frameRate;
}

void    Video::setSampleRate(unsigned int sampleRate)
{
    this->sampleRate = sampleRate;
}

void    Video::setChannels(unsigned int channels)
{
    this->channels = channels;
}

void    Video::setStart(double start)
{
    if (start < 0)
        start = 0;
    this->start = start;
}

void    Video::setDuration(double duration)
{
    if (duration < 0)
        duration = 0;
    this->duration = duration;
}

void    Video::setOptions(const QVariantHash &options)
{
    this->options = options;
}

int     Video::writePacket(void *opaque, uint8_t *buf, int buf_size)
{
    Video       *instance = static_cast<Video *>(opaque);
    int         position = instance->bytesTranscoded;
    QByteArray  *buffer;
    int         length;

    // The muxer tries to write on a buffer already returned
    if (instance->seek < position)
    {
        if (instance->seek > 1024)
            instance->api->log().warning("Could not write on this buffer anymore", instance->_getProperties(0, Properties("seek", instance->seek).add("position", position).add("buf_size", buf_size)), "Video", "writePacket");
        // The muxer wants to modify the header while writing the trailer
        else
            instance->api->log().debug("Could not write on this buffer anymore", instance->_getProperties(0, Properties("seek", instance->seek).add("position", position).add("buf_size", buf_size)), "Video", "writePacket");
        return (buf_size);
    }
    // Searches the buffer to write in
    QMutableListIterator<QByteArray> it(instance->buffers);
    while (it.hasNext() && (position += it.next().size()) < instance->seek)
        ;
    buffer = &it.peekPrevious();
    // The muxer seeked too far
    if (position < instance->seek)
    {
        instance->api->log().warning("Seek too far", instance->_getProperties(0, Properties("seek", instance->seek).add("position", position).add("buf_size", buf_size)), "Video", "writePacket");
        instance->seek = position;
    }
    // The position to write in the current buffer
    position = instance->seek - (position - buffer->size());
    // The position is at the end of the buffer and there is another buffer after
    if (it.hasNext() && position == buffer->size())
    {
        buffer = &it.next();
        position = 0;
    }
    // Appends the data at the end of the buffers
    if (!it.hasNext() && position == buffer->size())
    {
        buffer->append((char *)buf, buf_size);
        instance->seek += buf_size;
        return (buf_size);
    }
    // From now we want to replace the data of the buffer
    length = buffer->size() - position;
    // Writes the data if there is enough space in the current buffer
    if (buf_size <= length)
    {
        buffer->replace(position, buf_size, (char *)buf, buf_size);
        instance->seek += buf_size;
        return (buf_size);
    }
    // Otherwise we write what we can and write the rest recursively
    buffer->replace(position, length, (char *)buf, length);
    instance->seek += length;
    Video::writePacket(opaque, buf + length, buf_size - length);
    return (buf_size);
}

int     Video::readPacket(void *opaque, uint8_t *, int buf_size)
{
    Video   *instance = static_cast<Video *>(opaque);

    instance->api->log().warning("This method should not have been called", "Video", "readPacket");
    return (buf_size);
}

int64_t Video::seekPacket(void *opaque, int64_t offset, int)
{
    Video   *instance = static_cast<Video *>(opaque);

    instance->seek = offset;
    return (offset);
}

void    Video::_clear(bool free, bool settings)
{
    this->buffers.clear();
    this->time = 0;
    this->seek = 0;
    this->bytesTranscoded = 0;
    this->finished = false;
    // Initializes the settings
    if (settings)
    {
        this->source.clear();
        this->format.clear();
        this->videoCodec.clear();
        this->audioCodec.clear();
        this->width = -1;
        this->height = -1;
        this->videoBitRate = 0;
        this->audioBitRate = 0;
        this->frameRate = 0;
        this->sampleRate = 0;
        this->channels = 0;
        this->start = 0;
        this->duration = 0;
        this->options.clear();
    }
    // Frees the allocated structures
    if (free)
    {
        if (this->packetOut.data)
            av_free_packet(&this->packetOut);
        if (this->packetIn.data)
            av_free_packet(&this->packetIn);
        if (this->frame)
            av_frame_free(&this->frame);
        if (this->ioContext)
            av_free(this->ioContext);
        if (this->videoFilterGraph)
            avfilter_graph_free(&this->videoFilterGraph);
        if (this->audioFilterGraph)
            avfilter_graph_free(&this->audioFilterGraph);
        if (this->videoEnc)
            avcodec_close(this->videoEnc);
        if (this->audioEnc)
            avcodec_close(this->audioEnc);
        if (this->videoDec)
            avcodec_close(this->videoDec);
        if (this->audioDec)
            avcodec_close(this->audioDec);
        if (this->formatOut)
            avformat_free_context(this->formatOut);
        if (this->formatIn)
            avformat_close_input(&this->formatIn);
    }
    // Initializes FFmpeg
    this->formatIn = NULL;
    this->formatOut = NULL;
    this->videoCodecId = AV_CODEC_ID_NONE;
    this->audioCodecId = AV_CODEC_ID_NONE;
    this->videoStreamIn = NULL;
    this->audioStreamIn = NULL;
    this->videoStreamOut = NULL;
    this->audioStreamOut = NULL;
    this->videoDec = NULL;
    this->audioDec = NULL;
    this->videoEnc = NULL;
    this->audioEnc = NULL;
    this->videoFilterGraph = NULL;
    this->audioFilterGraph = NULL;
    this->videoFilterIn = NULL;
    this->audioFilterIn = NULL;
    this->videoFilterOut = NULL;
    this->audioFilterOut = NULL;
    this->ioContext = NULL;
    this->frame = NULL;
    this->packetIn.data = NULL;
    this->packetOut.data = NULL;
    this->videoFramePts = 0;
    this->audioFramePts = 0;
}

AVStream     *Video::_openStream(AVCodecContext *&context, enum AVMediaType type)
{
    int      ret = -1;
    AVStream *stream = NULL;
    AVCodec  *decoder = NULL;

    if ((ret = av_find_best_stream(this->formatIn, type, -1, -1, &decoder, 0)) < 0)
    {
        LOG_ERROR(QString("Could not find %1 stream in input file").arg(av_get_media_type_string(type)), this->_getProperties(ret), "Video", "_openStream");
        return (NULL);
    }
    stream = this->formatIn->streams[ret];
    if ((ret = Plugin::avcodec_open2(stream->codec, decoder, NULL)))
    {
        LOG_ERROR(QString("Failed to open %1 codec").arg(av_get_media_type_string(type)), this->_getProperties(ret), "Video", "_openStream");
        return (NULL);
    }
    context = stream->codec;
    return (stream);
}

void    Video::_initializeOutput()
{
    int ret;

    this->_settings();
    // Initializes the output format and streams
    if ((ret = avformat_alloc_output_context2(&this->formatOut, NULL, this->format.toUtf8().data(), NULL)) < 0)
    {
        LOG_ERROR("Could not allocate the output format", this->_getProperties(ret), "Video", "transcode");
        throw false;
    }
    if (!this->_openVideoEncoder())
        throw false;
    if (this->audioStreamIn && !this->_openAudioEncoder())
        throw false;
    // Creates the io context that will get the transcoded data, via Video::writePacket
    if (!(this->ioContext = avio_alloc_context(this->ioBuffer, IO_BUFFER_SIZE, 1, this, Video::readPacket, Video::writePacket, Video::seekPacket)))
    {
        LOG_ERROR("Could not allocate the io context", this->_getProperties(), "Video", "transcode");
        throw false;
    }
    this->formatOut->pb = this->ioContext;
    if (this->formatOut->oformat->flags & AVFMT_NOFILE)
        LOG_WARNING("AVFMT_NOFILE is defined", this->_getProperties(), "Video", "transcode");
    if (!(this->frame = av_frame_alloc()))
    {
        LOG_ERROR("Could not allocate the frame", this->_getProperties(), "Video", "transcode");
        throw false;
    }
    // Writes the format header
    if ((ret = avformat_write_header(this->formatOut, NULL)) < 0)
    {
        LOG_ERROR("Could not write the format header", this->_getProperties(ret), "Video", "transcode");
        throw false;
    }
    // Seeks the source to the starting position
    this->start *= qRound64(1 / av_q2d(this->videoStreamIn->time_base));
    if (this->start && av_seek_frame(this->formatIn, this->videoStreamIn->index, this->start, 0) < 0)
    {
        LOG_ERROR("Could not seek to the starting position", this->_getProperties(0, Properties("start", this->start)), "Video", "transcode");
        throw false;
    }
    LOG_DEBUG("Transcoding started", this->_getProperties(), "Video", "transcode");
}

void    Video::_settings()
{
    AVCodec *codec;

    // The default format and codecs are the same as the source file
    if (this->format.isEmpty())
        this->format = QString(this->formatIn->iformat->name).split(",").first();
    if (this->videoCodec.isEmpty())
        this->videoCodec = this->videoDec->codec->name;
    if (this->audioCodec.isEmpty())
        this->audioCodec = this->audioDec->codec->name;
    // Gets the id of the encoders
    if ((codec = avcodec_find_encoder_by_name(this->videoCodec.toUtf8().data())) ||
        (codec = avcodec_find_decoder_by_name(this->videoCodec.toUtf8().data())))
        this->videoCodecId = codec->id;
    if ((codec = avcodec_find_encoder_by_name(this->audioCodec.toUtf8().data())) ||
        (codec = avcodec_find_decoder_by_name(this->audioCodec.toUtf8().data())))
        this->audioCodecId = codec->id;
    // Bit rates
    if (!this->videoBitRate)
        if (!(this->videoBitRate = this->videoDec->bit_rate))
            this->videoBitRate = 2000000;
    if (!this->audioBitRate)
        if (!(this->audioBitRate = this->audioDec->bit_rate))
            this->audioBitRate = 192000;
}

bool    Video::_openVideoEncoder()
{
    AVCodec *encoder;
    int     ret;

    // Creates the video stream
    if (!(encoder = avcodec_find_encoder(this->videoCodecId)))
    {
        LOG_ERROR("Video encoder not found", this->_getProperties(), "Video", "_openVideoEncoder");
        return (false);
    }
    if (!(this->videoStreamOut = avformat_new_stream(this->formatOut, encoder)))
    {
        LOG_ERROR("Could not allocate output video stream", this->_getProperties(), "Video", "_openVideoEncoder");
        return (false);
    }
    this->videoStreamOut->id = this->videoStreamOut->index;
    this->videoStreamOut->disposition = this->videoStreamIn->disposition;
    this->videoEnc = this->videoStreamOut->codec;
    this->videoEnc->codec = encoder;
    if (!this->_configureVideoFilter())
        return (false);
    // Initializes the encoder according to the output filter
    this->videoEnc->width = this->videoFilterOut->inputs[0]->w;
    this->videoEnc->height = this->videoFilterOut->inputs[0]->h;
    this->videoEnc->pix_fmt = (AVPixelFormat)this->videoFilterOut->inputs[0]->format;
    this->videoEnc->sample_aspect_ratio = this->videoFilterOut->inputs[0]->sample_aspect_ratio;
    this->videoEnc->bits_per_raw_sample = this->videoDec->bits_per_raw_sample;
    this->videoEnc->chroma_sample_location = this->videoDec->chroma_sample_location;
    this->videoEnc->bit_rate = this->videoBitRate;
    this->videoStreamOut->r_frame_rate = av_buffersink_get_frame_rate(this->videoFilterOut);
    this->videoEnc->time_base = av_inv_q(this->videoStreamOut->r_frame_rate);
    this->duration *= av_q2d(this->videoStreamOut->r_frame_rate);
    //this->videoEnc->gop_size = 10;
    //this->videoEnc->max_b_frames = 2;
    if (this->formatOut->oformat->flags & AVFMT_GLOBALHEADER)
        this->videoEnc->flags |= CODEC_FLAG_GLOBAL_HEADER;
    if(this->videoCodecId == AV_CODEC_ID_H264)
        av_opt_set(this->videoEnc->priv_data, "preset", "slow", 0);
    if ((ret = Plugin::avcodec_open2(this->videoEnc, encoder, NULL)) < 0)
    {
        LOG_ERROR("Could not open the video encoder", this->_getProperties(ret), "Video", "_openVideoEncoder");
        return (false);
    }
    return (true);
}

bool    Video::_openAudioEncoder()
{
    AVCodec *encoder;
    int     ret;

    if (!(encoder = avcodec_find_encoder(this->audioCodecId)))
    {
        LOG_ERROR("Audio encoder not found", this->_getProperties(), "Video", "_openAudioEncoder");
        return (false);
    }
    if (!(this->audioStreamOut = avformat_new_stream(this->formatOut, encoder)))
    {
        LOG_ERROR("Could not allocate output audio stream", this->_getProperties(), "Video", "_openAudioEncoder");
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
    this->audioEnc->time_base.num = 1;
    this->audioEnc->time_base.den = this->audioDec->sample_rate;
    if (this->formatOut->oformat->flags & AVFMT_GLOBALHEADER)
        this->audioEnc->flags |= CODEC_FLAG_GLOBAL_HEADER;
    this->audioEnc->bit_rate = this->audioBitRate;
    if ((ret = Plugin::avcodec_open2(this->audioEnc, encoder, NULL)) < 0)
    {
        LOG_ERROR("Could not open the audio encoder", this->_getProperties(ret), "Video", "_openAudioEncoder");
        return (false);
    }
    if (!(this->audioEnc->codec->capabilities & CODEC_CAP_VARIABLE_FRAME_SIZE))
        av_buffersink_set_frame_size(this->audioFilterOut, this->audioEnc->frame_size);
    return (true);
}

bool    Video::_configureVideoFilter()
{
    AVFilterInOut *inputs, *outputs;
    int           ret;

    this->videoFilterGraph = avfilter_graph_alloc();
    if ((ret = avfilter_graph_parse2(this->videoFilterGraph, "null", &inputs, &outputs)) < 0
        || !inputs || inputs->next || !outputs || outputs->next)
    {
        LOG_ERROR("The video filter graph does not have exactly one input and output", this->_getProperties(ret), "Video", "_configureVideoFilter");
        return (false);
    }
    if (!this->_configureInputVideoFilter(inputs))
        return (false);
    if (!this->_configureOutputVideoFilter(outputs))
        return (false);
    if ((ret = avfilter_graph_config(this->videoFilterGraph, NULL)) < 0)
    {
        LOG_ERROR("Could not configure the video filter graph", this->_getProperties(ret), "Video", "_configureVideoFilter");
        return (false);
    }
    return (true);
}

bool    Video::_configureInputVideoFilter(AVFilterInOut *inputs)
{
    QString args("video_size=%1x%2:pix_fmt=%3:time_base=%4/%5:pixel_aspect=0/1:sws_param=flags=%6");
    int     ret;

    args = args.arg(QString::number(this->videoDec->width), QString::number(this->videoDec->height),
                    QString::number(this->videoDec->pix_fmt),
                    QString::number(this->videoStreamIn->time_base.num), QString::number(this->videoStreamIn->time_base.den),
                    QString::number(SWS_BILINEAR + ((this->videoDec->flags & CODEC_FLAG_BITEXACT) ? SWS_BITEXACT : 0)));
    if (this->videoStreamIn->r_frame_rate.num && this->videoStreamIn->r_frame_rate.den)
        args.append(QString(":frame_rate=%1/%2").arg(QString::number(this->videoStreamIn->r_frame_rate.num), QString::number(this->videoStreamIn->r_frame_rate.den)));
    if ((ret = avfilter_graph_create_filter(&this->videoFilterIn, avfilter_get_by_name("buffer"), "buffer", args.toLatin1().data(), NULL, this->videoFilterGraph)) < 0)
    {
        LOG_ERROR("Could not create the buffer filter", this->_getProperties(ret, Properties("args", args)), "Video", "_configureInputVideoFilter");
        return (false);
    }
    if (avfilter_link(this->videoFilterIn, 0, inputs->filter_ctx, inputs->pad_idx) < 0)
    {
        LOG_ERROR("Could not link the buffer and null filters", this->_getProperties(), "Video", "_configureInputVideoFilter");
        return (false);
    }
    avfilter_inout_free(&inputs);
    return (true);
}

bool    Video::_configureOutputVideoFilter(AVFilterInOut *outputs)
{
    AVFilterContext *scale;
    AVFilterContext *format;
    AVFilterContext *fps;
    QByteArray      args;
    QByteArray      pixelFormats;
    AVFilterContext *lastFilter = outputs->filter_ctx;
    int             ret;

    if ((ret = avfilter_graph_create_filter(&this->videoFilterOut, avfilter_get_by_name("ffbuffersink"), "ffbuffersink", NULL, NULL, this->videoFilterGraph)) < 0)
    {
        LOG_ERROR("Could not create the ffbuffersink filter", this->_getProperties(ret), "Video", "_configureOutputVideoFilter");
        return (false);
    }
    if ((this->width || this->height) && (this->width != -1 || this->height != -1))
    {
        args = QString("%1:%2:flags=0x%3").arg(QString::number(this->width),
               QString::number(this->height), QString::number(SWS_BICUBIC, 16)).toLatin1();
        if ((ret = avfilter_graph_create_filter(&scale, avfilter_get_by_name("scale"), "scale", args.data(), NULL, this->videoFilterGraph)) < 0)
        {
            LOG_ERROR("Could not create the scale filter", this->_getProperties(ret, Properties("args", args)), "Video", "_configureOutputVideoFilter");
            return (false);
        }
        if (avfilter_link(lastFilter, 0, scale, 0) < 0)
        {
            LOG_ERROR("Could not link the scale filter", this->_getProperties(), "Video", "_configureOutputVideoFilter");
            return (false);
        }
        lastFilter = scale;
    }
    if (!(pixelFormats = this->_getPixelFormats(this->videoEnc->codec)).isEmpty())
    {
        if ((ret = avfilter_graph_create_filter(&format, avfilter_get_by_name("format"), "format", pixelFormats.data(), NULL, this->videoFilterGraph)) < 0)
        {
            LOG_ERROR("Could not create the format filter", this->_getProperties(ret, Properties("pixel formats", pixelFormats)), "Video", "_configureOutputVideoFilter");
            return (false);
        }
        if (avfilter_link(lastFilter, 0, format, 0) < 0)
        {
            LOG_ERROR("Could not link the format filter", this->_getProperties(), "Video", "_configureOutputVideoFilter");
            return (false);
        }
        lastFilter = format;
    }
    if (this->frameRate)
    {
        args = QString("fps=%1/1").arg(QString::number(this->frameRate)).toLatin1();
        if ((ret = avfilter_graph_create_filter(&fps, avfilter_get_by_name("fps"), "fps", args.data(), NULL, this->videoFilterGraph)) < 0)
        {
            LOG_ERROR("Could not create the fps filter", this->_getProperties(ret, Properties("args", args)), "Video", "_configureOutputVideoFilter");
            return (false);
        }
        if (avfilter_link(lastFilter, 0, fps, 0) < 0)
        {
            LOG_ERROR("Could not link the fps filter", this->_getProperties(), "Video", "_configureOutputVideoFilter");
            return (false);
        }
        lastFilter = fps;
    }
    if (avfilter_link(lastFilter, 0, this->videoFilterOut, 0) < 0)
    {
        LOG_ERROR("Could not link the format and ffbuffersink filters", this->_getProperties(), "Video", "_configureOutputVideoFilter");
        return (false);
    }
    avfilter_inout_free(&outputs);
    return (true);
}

bool    Video::_configureAudioFilter()
{
    AVFilterInOut *inputs, *outputs;
    int           ret;

    this->audioFilterGraph = avfilter_graph_alloc();
    if ((ret = avfilter_graph_parse2(this->audioFilterGraph, "anull", &inputs, &outputs)) < 0
        || !inputs || inputs->next || !outputs || outputs->next)
    {
        LOG_ERROR("The audio filter graph does not have exactly one input and output", this->_getProperties(ret), "Video", "_configureAudioFilter");
        return (false);
    }
    if (!this->_configureInputAudioFilter(inputs))
        return (false);
    if (!this->_configureOutputAudioFilter(outputs))
        return (false);
    if ((ret = avfilter_graph_config(this->audioFilterGraph, NULL)) < 0)
    {
        LOG_ERROR("Could not configure the audio filter graph", this->_getProperties(ret), "Video", "_configureAudioFilter");
        return (false);
    }
    return (true);
}

bool    Video::_configureInputAudioFilter(AVFilterInOut *inputs)
{
    QByteArray  args;
    int         ret;

    args = QString("time_base=1/%1:sample_rate=%1:sample_fmt=%2:channel_layout=0x%3")
                  .arg(QString::number(this->audioDec->sample_rate),
                       av_get_sample_fmt_name(this->audioDec->sample_fmt),
                       QString::number(this->audioDec->channel_layout, 16)).toLatin1();
    if ((ret = avfilter_graph_create_filter(&this->audioFilterIn, avfilter_get_by_name("abuffer"), "abuffer", args.data(), NULL, this->audioFilterGraph)) < 0)
    {
        LOG_ERROR("Could not create the abuffer filter", this->_getProperties(ret, Properties("args", args)), "Video", "_configureInputAudioFilter");
        return (false);
    }
    if (avfilter_link(this->audioFilterIn, 0, inputs->filter_ctx, inputs->pad_idx) < 0)
    {
        LOG_ERROR("Could not link the abuffer and anull filters", this->_getProperties(), "Video", "_configureInputAudioFilter");
        return (false);
    }
    avfilter_inout_free(&inputs);
    return (true);
}

bool    Video::_configureOutputAudioFilter(AVFilterInOut *outputs)
{
    AVFilterContext *aformat;
    QByteArray      args;
    QByteArray      arg;
    int             ret;

    if ((ret = avfilter_graph_create_filter(&this->audioFilterOut, avfilter_get_by_name("ffabuffersink"), "ffabuffersink", NULL, NULL, this->audioFilterGraph)) < 0)
    {
        LOG_ERROR("Could not create the ffabuffersink filter", this->_getProperties(ret), "Video", "_configureOutputAudioFilter");
        return (false);
    }
    if (!(arg = this->_getSampleFormats(this->audioEnc->codec)).isEmpty())
        args += "sample_fmts=" + arg;
    if (!(arg = this->_getSampleRates(this->audioEnc->codec, this->sampleRate)).isEmpty())
        args += QByteArray(args.isEmpty() ? "" : ":") + "sample_rates=" + arg;
    if (!(arg = this->_getChannelLayouts(this->audioEnc->codec, av_get_default_channel_layout(this->channels))).isEmpty())
        args += QByteArray(args.isEmpty() ? "" : ":") + "channel_layouts=" + arg;
    if ((ret = avfilter_graph_create_filter(&aformat, avfilter_get_by_name("aformat"), "aformat", args.data(), NULL, this->audioFilterGraph)) < 0)
    {
        LOG_ERROR("Could not create the aformat filter", this->_getProperties(ret, Properties("args", args)), "Video", "_configureOutputAudioFilter");
        return (false);
    }
    if (avfilter_link(outputs->filter_ctx, outputs->pad_idx, aformat, 0) < 0)
    {
        LOG_ERROR("Could not link the aformat and anull filters", this->_getProperties(), "Video", "_configureOutputAudioFilter");
        return (false);
    }
    if (avfilter_link(aformat, 0, this->audioFilterOut, 0) < 0)
    {
        LOG_ERROR("Could not link the aformat and ffabuffersink filters", this->_getProperties(), "Video", "_configureOutputAudioFilter");
        return (false);
    }
    avfilter_inout_free(&outputs);
    return (true);
}

QByteArray  Video::_getPixelFormats(const AVCodec *codec) const
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

QByteArray  Video::_getSampleFormats(const AVCodec *codec)
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

QByteArray  Video::_getSampleRates(const AVCodec *codec, unsigned int prefered)
{
    const int   *p = codec->supported_samplerates;
    QStringList result;

    while (p && *p != 0)
        result.append(QString::number(*(p++)));
    if (prefered && (result.isEmpty() || result.contains(QString::number(prefered))))
        result = QStringList() << QString::number(prefered);
    return (result.join(",").toLatin1());
}

QByteArray  Video::_getChannelLayouts(const AVCodec *codec, unsigned int prefered)
{
    const quint64 *p = (const quint64 *)codec->channel_layouts;
    QStringList result;

    while (p && *p != 0)
        result.append("0x" + QString::number(*(p++), 16));
    if (prefered && (result.isEmpty() || result.contains("0x" + QString::number(prefered, 16))))
        result = QStringList() << ("0x" + QString::number(prefered, 16));
    return (result.join(",").toLatin1());
}

bool    Video::_transcodeVideo()
{
    int gotFrame;
    int gotPacket;
    int ret;
    AVFilterBufferRef *bufferRef = NULL;

    if ((ret = avcodec_decode_video2(this->videoDec, this->frame, &gotFrame, &this->packetIn)) < 0)
        LOG_ERROR("Could not decode the video frame", this->_getProperties(ret), "Video", "_transcodeVideo");
    if (gotFrame)
    {
        this->frame->pts = (int)av_frame_get_best_effort_timestamp(this->frame);
        if ((ret = av_buffersrc_add_frame_flags(this->videoFilterIn, this->frame, AV_BUFFERSRC_FLAG_PUSH)) < 0)
        {
            LOG_ERROR("Error while feeding the video filter graph", this->_getProperties(ret), "Video", "_transcodeVideo");
            throw false;
        }
        while (true)
        {
            if ((ret = av_buffersink_get_buffer_ref(this->videoFilterOut, &bufferRef, AV_BUFFERSINK_FLAG_NO_REQUEST)) < 0)
            {
                if (ret != AVERROR(EAGAIN) && ret != AVERROR_EOF)
                {
                    LOG_ERROR("Could not get the output of the filter graph", this->_getProperties(ret), "Video", "_transcodeVideo");
                    throw false;
                }
                break;
            }
            avfilter_copy_buf_props(this->frame, bufferRef);
            av_init_packet(&this->packetOut);
            this->packetOut.data = NULL;
            this->packetOut.size = 0;
            this->frame->pts = videoFramePts;
            if ((ret = avcodec_encode_video2(this->videoEnc, &this->packetOut, this->frame, &gotPacket)) < 0)
            {
                LOG_ERROR("Could not encode the video frame", this->_getProperties(ret), "Video", "_transcodeVideo");
                throw false;
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
                if ((ret = av_interleaved_write_frame(this->formatOut, &this->packetOut)) < 0)
                {
                    LOG_ERROR("Could not write the video packet", this->_getProperties(ret), "Video", "_transcodeVideo");
                    throw false;
                }
                av_free_packet(&this->packetOut);
            }
            avfilter_unref_bufferp(&bufferRef);
            this->videoFramePts++;
            this->framesToEncode--;
        }
    }
    return (gotFrame != 0);
}

void    Video::_transcodeAudio(bool flush)
{
    AVPacket packet = this->packetIn;
    int      gotFrame = 1;
    int      gotPacket;
    int      ret;
    AVFilterBufferRef *bufferRef = NULL;

    while ((packet.size > 0 || (flush && gotFrame)))
    {
        if ((ret = avcodec_decode_audio4(this->audioDec, this->frame, &gotFrame, &packet)) < 0)
        {
            LOG_ERROR("Could not decode the audio frame", this->_getProperties(ret), "Video", "_transcodeAudio");
            throw false;
        }
        packet.data += ret;
        packet.size -= ret;
        if (gotFrame)
        {
            this->frame->pts = this->frame->pkt_pts;
            if ((ret = av_buffersrc_add_frame_flags(this->audioFilterIn, this->frame, AV_BUFFERSRC_FLAG_PUSH)) < 0)
            {
                LOG_ERROR("Error while feeding the audio filter graph", this->_getProperties(ret), "Video", "_transcodeAudio");
                throw false;
            }
            while (true)
            {
                if ((ret = av_buffersink_get_buffer_ref(this->audioFilterOut, &bufferRef, AV_BUFFERSINK_FLAG_NO_REQUEST)) < 0)
                {
                    if (ret != AVERROR(EAGAIN) && ret != AVERROR_EOF)
                    {
                        LOG_ERROR("Could not get the output of the audio filter graph", this->_getProperties(ret), "Video", "_transcodeAudio");
                        throw false;
                    }
                    break;
                }
                avfilter_copy_buf_props(this->frame, bufferRef);
                av_init_packet(&this->packetOut);
                this->packetOut.data = NULL;
                this->packetOut.size = 0;
                this->frame->pts = this->audioFramePts;
                if ((ret = avcodec_encode_audio2(this->audioEnc, &this->packetOut, this->frame, &gotPacket)) < 0)
                {
                    LOG_ERROR("Could not encode the audio frame", this->_getProperties(ret), "Video", "_transcodeAudio");
                    throw false;
                }
                if (gotPacket)
                {
                    if (this->packetOut.pts != AV_NOPTS_VALUE)
                        this->packetOut.pts = av_rescale_q(this->packetOut.pts, this->audioEnc->time_base, this->audioStreamOut->time_base);
                    if (this->packetOut.dts != AV_NOPTS_VALUE)
                        this->packetOut.dts = av_rescale_q(this->packetOut.dts, this->audioEnc->time_base, this->audioStreamOut->time_base);
                    if (this->packetOut.duration > 0)
                        this->packetOut.duration = av_rescale_q(this->packetOut.duration, this->audioEnc->time_base, this->audioStreamOut->time_base);
                    this->packetOut.stream_index = this->audioStreamOut->index;
                    if ((ret = av_interleaved_write_frame(this->formatOut, &this->packetOut)) < 0)
                    {
                        LOG_ERROR("Could not write the audio packet", this->_getProperties(ret), "Video", "_transcodeAudio");
                        throw false;
                    }
                    av_free_packet(&this->packetOut);
                }
                avfilter_unref_bufferp(&bufferRef);
                this->audioFramePts += this->frame->nb_samples;
            }
        }
    }
}

QMap<QString, QString> Video::_getProperties(int errnum, Properties p)
{
    Properties  properties(p);

    if (errnum)
        properties.add("error", this->_strError(errnum));
    properties.add("source", this->source);
    if (this->formatOut && this->formatOut->oformat)
        properties.add("format", this->formatOut->oformat->name);
    if (this->videoEnc)
    {
        if (this->videoEnc->codec)
            properties.add("video codec", this->videoEnc->codec->name);
        if (this->videoFilterOut)
            properties.add("width", this->videoEnc->width).add("height", this->videoEnc->height)
                      .add("pixel format", av_get_pix_fmt_name(this->videoDec->pix_fmt))
                      .add("bit rate", this->videoEnc->bit_rate).add("frame rate", av_q2d(this->videoStreamOut->r_frame_rate));
    }
    if (this->audioEnc)
    {
        if (this->audioEnc->codec)
            properties.add("audio codec", this->audioEnc->codec->name);
        if (this->audioFilterOut)
            properties.add("sample rate", this->audioEnc->sample_rate)
                      .add("sample format", av_get_sample_fmt_name(this->audioEnc->sample_fmt))
                      .add("bit rate", this->audioEnc->bit_rate).add("channels", this->audioEnc->channels);
    }
    properties.add("frame", this->videoFramePts).add("duration", this->duration).add("start", this->start);
    return (properties.toMap());
}

QByteArray  Video::_strError(int errnum)
{
    QByteArray  error(128, 0);

    av_strerror(errnum, error.data(), error.size());
    return (error);
}

void    Video::_printRawFrame()
{
    static uint8_t  *data[4] = { NULL };
    static int      linesize[4];
    static int      size = 0;
    static QFile    *f = NULL;

    if (!size)
    {
        size = av_image_alloc(data, linesize, this->videoDec->width, this->videoDec->height, this->videoDec->pix_fmt, 1);
        f = new QFile("rawvideo.yuv");
        f->open(QIODevice::WriteOnly | QIODevice::Truncate);
        printf("ffplay -f rawvideo -pix_fmt %s -video_size %dx%d rawvideo.yuv\n", av_get_pix_fmt_name(this->videoDec->pix_fmt), this->videoDec->width, this->videoDec->height);
    }
    av_image_copy(data, linesize, (const uint8_t **)(frame->data), frame->linesize, this->videoDec->pix_fmt, this->videoDec->width, this->videoDec->height);
    f->write((char *)data[0], size);
    f->flush();
}
