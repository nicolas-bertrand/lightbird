#include <QFileInfo>

#include "Audio.h"
#include "Plugin.h"

Audio::Audio(LightBird::IApi *a, QStringList &f)
    : api(a)
    , formats(f)
{
    this->ioBuffer = (unsigned char *)av_malloc(IO_BUFFER_SIZE);
    this->_clear(false, true);
}

Audio::~Audio()
{
    this->_clear();
    av_free(this->ioBuffer);
}

bool    Audio::initialize(const QString &source)
{
    int ret;

    // Frees the allocated structures
    this->_clear();
    // Opens the input file and its streams
    this->source = source;
    if ((ret = avformat_open_input(&this->formatIn, this->source.toUtf8().data(), NULL, NULL)) < 0)
        this->api->log().error("Could not open the source file", this->_getProperties(ret), "Audio", "initialize");
    else if ((ret = avformat_find_stream_info(this->formatIn, NULL)) < 0)
        this->api->log().error("Could not find stream information", this->_getProperties(ret), "Audio", "initialize");
    else
        this->streamIn = this->_openStream(this->decoder);
    // The file doesn't have a valid audio stream
    if (!this->streamIn)
    {
        this->_clear();
        return (false);
    }
    return (true);
}

QByteArray  Audio::transcode()
{
    QByteArray  result;
    int         read = 0;
    int         ret;

    try
    {
        // The transcoding session has not been initialized
        if (!this->formatIn)
            throw false;
        this->buffer.clear();
        // Initializes the output format and starts the transcoding
        if (!this->formatOut)
            this->_initializeOutput();
        // Transcodes one second of the source
        if (!this->finished)
        {
            // The number of frames to encode one second
            this->framesToEncode = (this->framePts / this->encoder->sample_rate + 1) * this->decoder->sample_rate - this->framePts;
            while ((this->framesToEncode > 0 || this->buffer.isEmpty()) && (read = av_read_frame(this->formatIn, &this->packetIn)) >= 0)
            {
                if (this->packetIn.stream_index == this->streamIn->index)
                    this->_transcode();
                av_free_packet(&this->packetIn);
            }
            // The transcodage duration has been reached
            if (this->duration && this->duration <= this->framePts)
                read = -1;
        }
        // Finishes the transcoding
        if (read < 0)
        {
            this->finished = true;
            // Flushes the cached frames
            this->packetIn.data = NULL;
            this->packetIn.size = 0;
            if (this->streamIn)
                this->_transcode(true);
            // Writes the format trailer. This might not work properly since the muxer can try to seek to the header.
            if ((ret = av_write_trailer(this->formatOut)) < 0)
            {
                LOG_ERROR("Could not write the format trailer", this->_getProperties(ret), "Audio", "transcode");
                throw false;
            }
        }
        result = this->buffer;
    }
    catch (bool)
    {
        this->_clear();
    }
    return (result);
}

int     Audio::getTime()
{
    if (!this->encoder)
        return (0);
    return (this->framePts / this->encoder->sample_rate);
}

void    Audio::clear()
{
    this->_clear(true, true);
}

bool    Audio::setFormat(const QString &format)
{
    if (!this->formats.contains(format.toLower()))
        return (false);
    this->format = format;
    return (true);
}

bool    Audio::setCodec(const QString &c)
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
    this->codec = codec;
    return (true);
}

void    Audio::setBitRate(unsigned int bitRate)
{
    this->bitRate = bitRate;
}

void    Audio::setSampleRate(unsigned int sampleRate)
{
    this->sampleRate = sampleRate;
}

void    Audio::setChannels(unsigned int channels)
{
    this->channels = channels;
}

void    Audio::setStart(double start)
{
    if (start < 0)
        start = 0;
    this->start = start;
}

void    Audio::setDuration(double duration)
{
    if (duration < 0)
        duration = 0;
    this->duration = duration;
}

void    Audio::setOptions(const QVariantHash &options)
{
    this->options = options;
}

int     Audio::writePacket(void *opaque, uint8_t *buf, int buf_size)
{
    Audio       *instance = static_cast<Audio *>(opaque);

    instance->buffer.append((char *)buf, buf_size);
    return (buf_size);
}

void    Audio::_clear(bool free, bool settings)
{
    this->buffer.clear();
    this->finished = false;
    // Initializes the settings
    if (settings)
    {
        this->source.clear();
        this->format.clear();
        this->codec.clear();
        this->bitRate = 0;
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
        if (this->filterGraph)
            avfilter_graph_free(&this->filterGraph);
        if (this->encoder)
            avcodec_close(this->encoder);
        if (this->decoder)
            avcodec_close(this->decoder);
        if (this->formatOut)
            avformat_free_context(this->formatOut);
        if (this->formatIn)
            avformat_close_input(&this->formatIn);
    }
    // Initializes FFmpeg
    this->formatIn = NULL;
    this->formatOut = NULL;
    this->codecId = AV_CODEC_ID_NONE;
    this->streamIn = NULL;
    this->streamOut = NULL;
    this->decoder = NULL;
    this->encoder = NULL;
    this->filterGraph = NULL;
    this->filterIn = NULL;
    this->filterOut = NULL;
    this->ioContext = NULL;
    this->frame = NULL;
    this->packetIn.data = NULL;
    this->packetOut.data = NULL;
    this->framePts = 0;
}

AVStream    *Audio::_openStream(AVCodecContext *&context)
{
    int      ret = -1;
    AVStream *stream = NULL;
    AVCodec  *decoder = NULL;

    if ((ret = av_find_best_stream(this->formatIn, AVMEDIA_TYPE_AUDIO, -1, -1, &decoder, 0)) < 0)
    {
        LOG_ERROR("Could not find an audio stream in input file", this->_getProperties(ret), "Audio", "_openStream");
        return (NULL);
    }
    stream = this->formatIn->streams[ret];
    if ((ret = Plugin::avcodec_open2(stream->codec, decoder, NULL)))
    {
        LOG_ERROR("Failed to open the codec", this->_getProperties(ret), "Audio", "_openStream");
        return (NULL);
    }
    context = stream->codec;
    return (stream);
}

void    Audio::_initializeOutput()
{
    int ret;

    this->_settings();
    // Initializes the output format and streams
    if ((ret = avformat_alloc_output_context2(&this->formatOut, NULL, this->format.toUtf8().data(), NULL)) < 0)
    {
        LOG_ERROR("Could not allocate the output format", this->_getProperties(ret), "Audio", "transcode");
        throw false;
    }
    if (!this->_openEncoder())
        throw false;
    // Creates the io context that will get the transcoded data, via Audio::writePacket
    if (!(this->ioContext = avio_alloc_context(this->ioBuffer, IO_BUFFER_SIZE, 1, this, NULL, Audio::writePacket, NULL)))
    {
        LOG_ERROR("Could not allocate the io context", this->_getProperties(), "Audio", "transcode");
        throw false;
    }
    this->formatOut->pb = this->ioContext;
    if (this->formatOut->oformat->flags & AVFMT_NOFILE)
        LOG_WARNING("AVFMT_NOFILE is defined", this->_getProperties(), "Audio", "transcode");
    if (!(this->frame = av_frame_alloc()))
    {
        LOG_ERROR("Could not allocate the frame", this->_getProperties(), "Audio", "transcode");
        throw false;
    }
    // Writes the format header
    if ((ret = avformat_write_header(this->formatOut, NULL)) < 0)
    {
        LOG_ERROR("Could not write the format header", this->_getProperties(ret), "Audio", "transcode");
        throw false;
    }
    // Seeks the source to the starting position
    this->start *= qRound64(1 / av_q2d(this->streamIn->time_base));
    if (this->start && av_seek_frame(this->formatIn, this->streamIn->index, this->start, 0) < 0)
    {
        LOG_ERROR("Could not seek to the starting position", this->_getProperties(0, Properties("start", this->start)), "Audio", "transcode");
        throw false;
    }
    LOG_DEBUG("Transcoding started", this->_getProperties(), "Audio", "transcode");
}

void    Audio::_settings()
{
    AVCodec *codec;

    // The default format and codecs are the same as the source file
    if (this->format.isEmpty())
        this->format = QString(this->formatIn->iformat->name).split(",").first();
    if (this->codec.isEmpty())
        this->codec = this->decoder->codec->name;
    // Gets the id of the encoders
    if ((codec = avcodec_find_encoder_by_name(this->codec.toUtf8().data())) ||
        (codec = avcodec_find_decoder_by_name(this->codec.toUtf8().data())))
        this->codecId = codec->id;
    // Bit rates
    if (!this->bitRate)
        if (!(this->bitRate = this->decoder->bit_rate))
            this->bitRate = 192000;
}

bool    Audio::_openEncoder()
{
    AVCodec *encoder;
    int     ret;

    if (!(encoder = avcodec_find_encoder(this->codecId)))
    {
        LOG_ERROR("Audio encoder not found", this->_getProperties(), "Audio", "_openEncoder");
        return (false);
    }
    if (!(this->streamOut = avformat_new_stream(this->formatOut, encoder)))
    {
        LOG_ERROR("Could not allocate output audio stream", this->_getProperties(), "Audio", "_openEncoder");
        return (false);
    }
    this->streamOut->id = this->streamOut->index;
    this->encoder = this->streamOut->codec;
    this->encoder->codec = encoder;
    if (!this->_configureFilter())
        return (false);
    this->encoder->sample_fmt = (AVSampleFormat)this->filterOut->inputs[0]->format;
    this->encoder->sample_rate = this->filterOut->inputs[0]->sample_rate;
    this->encoder->channel_layout = this->filterOut->inputs[0]->channel_layout;
    this->encoder->channels = av_get_channel_layout_nb_channels(this->encoder->channel_layout);
    this->encoder->time_base.num = 1;
    this->encoder->time_base.den = this->decoder->sample_rate;
    this->duration *= this->decoder->sample_rate;
    if (this->formatOut->oformat->flags & AVFMT_GLOBALHEADER)
        this->encoder->flags |= CODEC_FLAG_GLOBAL_HEADER;
    this->encoder->bit_rate = this->bitRate;
    if ((ret = Plugin::avcodec_open2(this->encoder, encoder, NULL)) < 0)
    {
        LOG_ERROR("Could not open the encoder", this->_getProperties(ret), "Audio", "_openEncoder");
        return (false);
    }
    if (!(this->encoder->codec->capabilities & CODEC_CAP_VARIABLE_FRAME_SIZE))
        av_buffersink_set_frame_size(this->filterOut, this->encoder->frame_size);
    return (true);
}

bool    Audio::_configureFilter()
{
    AVFilterInOut *inputs, *outputs;
    int           ret;

    this->filterGraph = avfilter_graph_alloc();
    if ((ret = avfilter_graph_parse2(this->filterGraph, "anull", &inputs, &outputs)) < 0
        || !inputs || inputs->next || !outputs || outputs->next)
    {
        LOG_ERROR("The filter graph does not have exactly one input and output", this->_getProperties(ret), "Audio", "_configureFilter");
        return (false);
    }
    if (!this->_configureInputFilter(inputs))
        return (false);
    if (!this->_configureOutputFilter(outputs))
        return (false);
    if ((ret = avfilter_graph_config(this->filterGraph, NULL)) < 0)
    {
        LOG_ERROR("Could not configure the filter graph", this->_getProperties(ret), "Audio", "_configureFilter");
        return (false);
    }
    return (true);
}

bool    Audio::_configureInputFilter(AVFilterInOut *inputs)
{
    QByteArray  args;
    int         ret;

    args = QString("time_base=1/%1:sample_rate=%1:sample_fmt=%2:channel_layout=0x%3")
                  .arg(QString::number(this->decoder->sample_rate),
                       av_get_sample_fmt_name(this->decoder->sample_fmt),
                       QString::number(this->decoder->channel_layout, 16)).toLatin1();
    if ((ret = avfilter_graph_create_filter(&this->filterIn, avfilter_get_by_name("abuffer"), "abuffer", args.data(), NULL, this->filterGraph)) < 0)
    {
        LOG_ERROR("Could not create the abuffer filter", this->_getProperties(ret, Properties("args", args)), "Audio", "_configureInputFilter");
        return (false);
    }
    if (avfilter_link(this->filterIn, 0, inputs->filter_ctx, inputs->pad_idx) < 0)
    {
        LOG_ERROR("Could not link the abuffer and anull filters", this->_getProperties(), "Audio", "_configureInputFilter");
        return (false);
    }
    avfilter_inout_free(&inputs);
    return (true);
}

bool    Audio::_configureOutputFilter(AVFilterInOut *outputs)
{
    AVFilterContext *aformat;
    QByteArray      args;
    QByteArray      arg;
    int             ret;

    if ((ret = avfilter_graph_create_filter(&this->filterOut, avfilter_get_by_name("ffabuffersink"), "ffabuffersink", NULL, NULL, this->filterGraph)) < 0)
    {
        LOG_ERROR("Could not create the ffabuffersink filter", this->_getProperties(ret), "Audio", "_configureOutputFilter");
        return (false);
    }
    if (!(arg = this->_getSampleFormats(this->encoder->codec)).isEmpty())
        args += "sample_fmts=" + arg;
    if (!(arg = this->_getSampleRates(this->encoder->codec, this->sampleRate)).isEmpty())
        args += QByteArray(args.isEmpty() ? "" : ":") + "sample_rates=" + arg;
    if (!(arg = this->_getChannelLayouts(this->encoder->codec, av_get_default_channel_layout(this->channels))).isEmpty())
        args += QByteArray(args.isEmpty() ? "" : ":") + "channel_layouts=" + arg;
    if ((ret = avfilter_graph_create_filter(&aformat, avfilter_get_by_name("aformat"), "aformat", args.data(), NULL, this->filterGraph)) < 0)
    {
        LOG_ERROR("Could not create the aformat filter", this->_getProperties(ret, Properties("args", args)), "Audio", "_configureOutputFilter");
        return (false);
    }
    if (avfilter_link(outputs->filter_ctx, outputs->pad_idx, aformat, 0) < 0)
    {
        LOG_ERROR("Could not link the aformat and anull filters", this->_getProperties(), "Audio", "_configureOutputFilter");
        return (false);
    }
    if (avfilter_link(aformat, 0, this->filterOut, 0) < 0)
    {
        LOG_ERROR("Could not link the aformat and ffabuffersink filters", this->_getProperties(), "Audio", "_configureOutputFilter");
        return (false);
    }
    avfilter_inout_free(&outputs);
    return (true);
}

QByteArray  Audio::_getSampleFormats(const AVCodec *codec)
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

QByteArray  Audio::_getSampleRates(const AVCodec *codec, unsigned int prefered)
{
    const int   *p = codec->supported_samplerates;
    QStringList result;

    while (p && *p != 0)
        result.append(QString::number(*(p++)));
    if (prefered && (result.isEmpty() || result.contains(QString::number(prefered))))
        result = QStringList() << QString::number(prefered);
    return (result.join(",").toLatin1());
}

QByteArray  Audio::_getChannelLayouts(const AVCodec *codec, unsigned int prefered)
{
    const quint64 *p = codec->channel_layouts;
    QStringList result;

    while (p && *p != 0)
        result.append("0x" + QString::number(*(p++), 16));
    if (prefered && (result.isEmpty() || result.contains("0x" + QString::number(prefered, 16))))
        result = QStringList() << ("0x" + QString::number(prefered, 16));
    return (result.join(",").toLatin1());
}

void    Audio::_transcode(bool flush)
{
    AVPacket packet = this->packetIn;
    int      gotFrame = 1;
    int      gotPacket;
    int      ret;
    AVFilterBufferRef *bufferRef = NULL;

    while ((packet.size > 0 || (flush && gotFrame)))
    {
        if ((ret = avcodec_decode_audio4(this->decoder, this->frame, &gotFrame, &packet)) < 0)
        {
            LOG_ERROR("Could not decode the frame", this->_getProperties(ret), "Audio", "_transcode");
            throw false;
        }
        packet.data += ret;
        packet.size -= ret;
        if (gotFrame)
        {
            this->frame->pts = this->frame->pkt_pts;
            if ((ret = av_buffersrc_add_frame_flags(this->filterIn, this->frame, AV_BUFFERSRC_FLAG_PUSH)) < 0)
            {
                LOG_ERROR("Error while feeding the filter graph", this->_getProperties(ret), "Audio", "_transcode");
                throw false;
            }
            while (true)
            {
                if ((ret = av_buffersink_get_buffer_ref(this->filterOut, &bufferRef, AV_BUFFERSINK_FLAG_NO_REQUEST)) < 0)
                {
                    if (ret != AVERROR(EAGAIN) && ret != AVERROR_EOF)
                    {
                        LOG_ERROR("Could not get the output of the filter graph", this->_getProperties(ret), "Audio", "_transcode");
                        throw false;
                    }
                    break;
                }
                avfilter_copy_buf_props(this->frame, bufferRef);
                av_init_packet(&this->packetOut);
                this->packetOut.data = NULL;
                this->packetOut.size = 0;
                this->frame->pts = this->framePts;
                if ((ret = avcodec_encode_audio2(this->encoder, &this->packetOut, this->frame, &gotPacket)) < 0)
                {
                    LOG_ERROR("Could not encode the frame", this->_getProperties(ret), "Audio", "_transcode");
                    throw false;
                }
                if (gotPacket)
                {
                    if (this->packetOut.pts != AV_NOPTS_VALUE)
                        this->packetOut.pts = av_rescale_q(this->packetOut.pts, this->encoder->time_base, this->streamOut->time_base);
                    if (this->packetOut.dts != AV_NOPTS_VALUE)
                        this->packetOut.dts = av_rescale_q(this->packetOut.dts, this->encoder->time_base, this->streamOut->time_base);
                    if (this->packetOut.duration > 0)
                        this->packetOut.duration = av_rescale_q(this->packetOut.duration, this->encoder->time_base, this->streamOut->time_base);
                    this->packetOut.stream_index = this->streamOut->index;
                    if ((ret = av_interleaved_write_frame(this->formatOut, &this->packetOut)) < 0)
                    {
                        LOG_ERROR("Could not write the packet", this->_getProperties(ret), "Audio", "_transcode");
                        throw false;
                    }
                    av_free_packet(&this->packetOut);
                }
                avfilter_unref_bufferp(&bufferRef);
                this->framePts += this->frame->nb_samples;
                this->framesToEncode -= this->frame->nb_samples;
            }
        }
    }
}

QMap<QString, QString> Audio::_getProperties(int errnum, Properties p)
{
    Properties  properties(p);

    if (errnum)
        properties.add("error", this->_strError(errnum));
    properties.add("source", this->source);
    if (this->formatOut && this->formatOut->oformat)
        properties.add("format", this->formatOut->oformat->name);
    if (this->encoder)
    {
        if (this->encoder->codec)
            properties.add("codec", this->encoder->codec->name);
        if (this->filterOut)
            properties.add("sample rate", this->encoder->sample_rate)
                      .add("sample format", av_get_sample_fmt_name(this->encoder->sample_fmt))
                      .add("bit rate", this->encoder->bit_rate).add("channels", this->encoder->channels);
    }
    properties.add("frame", this->framePts).add("duration", this->duration).add("start", this->start);
    return (properties.toMap());
}

QByteArray  Audio::_strError(int errnum)
{
    QByteArray  error(128, 0);

    av_strerror(errnum, error.data(), error.size());
    return (error);
}
