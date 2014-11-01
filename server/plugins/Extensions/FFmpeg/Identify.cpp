#include "LightBird.h"
#include "Identify.h"
#include "Plugin.h"

Identify::Identify(LightBird::IApi *a)
    : api(a)
{
    IIdentify::_types << LightBird::IIdentify::AUDIO << LightBird::IIdentify::VIDEO;
}

Identify::~Identify()
{
}

bool    Identify::identify(const QString &file, LightBird::IIdentify::Information &information)
{
    QVariantMap     &data = information.data;
    AVFormatContext *format = NULL;
    AVStream        *audioStream = NULL;
    AVStream        *videoStream = NULL;
    AVCodecContext  *audio = NULL;
    AVCodecContext  *video = NULL;
    AVRational      sar;
    AVRational      dar;
    int             code;
    bool            result = true;

    try
    {
        // Opens the input file
        if ((code = avformat_open_input(&format, file.toUtf8().data(), NULL, NULL)) < 0)
        {
            LOG_TRACE("Could not open source file", Properties("file", file).add("error", this->_errorToString(code)).toMap(), "Identify", "identify");
            throw false;
        }
        if ((code = avformat_find_stream_info(format, NULL)) < 0)
        {
            LOG_TRACE("Could not find stream information", Properties("file", file).add("error", this->_errorToString(code)).toMap(), "Identify", "identify");
            throw false;
        }
        // The file should be a document
        if (format->iformat->name == QString("tty"))
        {
            information.type = LightBird::IIdentify::DOCUMENT;
            throw true;
        }
        audioStream = this->_openStream(format, audio, AVMEDIA_TYPE_AUDIO);
        videoStream = this->_openStream(format, video, AVMEDIA_TYPE_VIDEO);
        // No audio or video stream detected
        if (!audioStream && !videoStream)
            throw false;
        // Identify the format
        {
            data.insert("format", format->iformat->name);
            data.insert("format name", format->iformat->long_name);
            if (format->duration)
                data.insert("duration", format->duration * (1.0 / (double)AV_TIME_BASE));
            if (format->bit_rate)
                data.insert("bit rate", format->bit_rate);
            data.insert("streams", format->nb_streams);
            this->_addMetadata(format->metadata, data);
        }
        // Identify the audio stream
        if (audioStream)
        {
            information.type = LightBird::IIdentify::AUDIO;
            data.insert("audio codec", audio->codec->name);
            data.insert("audio codec name", audio->codec->long_name);
            if (audioStream->duration)
                data.insert("audio duration", audioStream->duration * av_q2d(audio->time_base));
            data.insert("channels", audio->channels);
            data.insert("sample rate", audio->sample_rate);
            if (audioStream->nb_frames)
                data.insert("audio frames", (qint64)audioStream->nb_frames);
            data.insert("audio time base", QString("%1/%2").arg(QString::number(audioStream->time_base.num), QString::number(audioStream->time_base.den)));
            data.insert("sample format", av_get_sample_fmt_name(audio->sample_fmt));
            this->_addMetadata(audioStream->metadata, data);
        }
        // Identify the video stream
        if (videoStream && !this->_isImage(videoStream))
        {
            information.type = LightBird::IIdentify::VIDEO;
            data.insert("video codec", video->codec->name);
            data.insert("video codec name", video->codec->long_name);
            data.insert("width", video->width);
            data.insert("height", video->height);
            if (videoStream->duration)
                data.insert("video duration", videoStream->duration * av_q2d(video->time_base));
            data.insert("frame rate", QString("%1/%2").arg(QString::number(videoStream->r_frame_rate.num), QString::number(videoStream->r_frame_rate.den)));
            data.insert("pixel format", av_get_pix_fmt_name(video->pix_fmt));
            if (videoStream->nb_frames)
                data.insert("video frames", (qint64)videoStream->nb_frames);
            if (video->has_b_frames)
                data.insert("has b frames", "true");
            data.insert("video time base", QString("%1/%2").arg(QString::number(videoStream->time_base.num), QString::number(videoStream->time_base.den)));
            sar = av_guess_sample_aspect_ratio(format, videoStream, NULL);
            if (sar.den)
            {
                av_reduce(&dar.num, &dar.den, video->width * sar.num, video->height * sar.den, 1024 * 1024);
                data.insert("aspect ratio", QString("%1/%2").arg(QString::number(dar.num), QString::number(dar.den)));
            }
            this->_addMetadata(videoStream->metadata, data);
        }
    }
    catch (bool exception)
    {
        result = exception;
    }
    if (audio)
        avcodec_close(audio);
    if (video)
        avcodec_close(video);
    if (format)
        avformat_close_input(&format);
    return (result);
}

AVStream     *Identify::_openStream(AVFormatContext *format, AVCodecContext *&context, enum AVMediaType type)
{
    int      result;
    AVStream *stream = NULL;
    AVCodec  *decoder = NULL;

    if ((result = av_find_best_stream(format, type, -1, -1, &decoder, 0)) < 0)
    {
        LOG_TRACE(QString("Could not find %1 stream in input file").arg(av_get_media_type_string(type)), "Identify", "_openStream");
        return (NULL);
    }
    stream = format->streams[result];
    if ((result = Plugin::avcodec_open2(stream->codec, decoder, NULL)))
    {
        LOG_TRACE(QString("Failed to open %1 codec").arg(av_get_media_type_string(type)), Properties("error", result).toMap(), "Identify", "_openStream");
        return (NULL);
    }
    context = stream->codec;
    return (stream);
}

void    Identify::_addMetadata(AVDictionary *metadata, QVariantMap &map)
{
    AVDictionaryEntry *tag = NULL;

    if (!metadata)
        return ;
    while ((tag = av_dict_get(metadata, "", tag, AV_DICT_IGNORE_SUFFIX)))
        map.insert(tag->key, tag->value);
}

QByteArray  Identify::_errorToString(int errnum)
{
    QByteArray  error(128, 0);

    av_strerror(errnum, error.data(), error.size());
    return (error);
}

bool    Identify::_isImage(AVStream *videoStream)
{
    return (av_q2d(videoStream->r_frame_rate) >= 90000);
}
