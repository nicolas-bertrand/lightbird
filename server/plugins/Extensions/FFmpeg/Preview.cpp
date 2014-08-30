#include <QFileInfo>
#include <QTemporaryFile>
#include <QImage>

#include "FFmpeg.h"
#include "LightBird.h"
#include "Plugin.h"
#include "Preview.h"

Preview::Preview(LightBird::IApi *a)
    : api(a)
{
    this->fileTemplate = this->api->configuration().get("temporaryPath") + "/" + "XXXXXX.jpeg";
}

Preview::~Preview()
{
}

bool    Preview::generate(const QString &source, QString &destination, LightBird::IImage::Format fmt, unsigned int width, unsigned int height, unsigned int position, float quality)
{
    QTemporaryFile  tmp;
    QList<void *>   extensions;
    AVFormatContext *format = NULL;
    AVStream        *stream = NULL;
    AVCodec         *decoder = NULL;
    AVCodecContext  *context = NULL;
    AVPacket        packet;
    AVFrame         *frame = NULL;
    SwsContext      *swsContext = NULL;
    int             gotFrame = 0;
    unsigned char   *data;
    int             linesize[AV_NUM_DATA_POINTERS];
    Properties      properties;
    int             ret;
    bool            result = false;
    Mutex           mutex(Plugin::getMutex(), this->api->getId(), "Preview", "generate");

    try
    {
        // Checks the source and the destination
        if (!mutex || !QFileInfo(source).isFile() || source == destination || destination.isEmpty())
            throw false;
        properties.add("source", source).add("destination", destination).add("format", fmt);
        // Opens the source and its video stream
        if ((ret = avformat_open_input(&format, source.toLatin1().data(), NULL, NULL)) < 0)
        {
            LOG_DEBUG("Could not open source file", properties.add("error", ret).toMap(), "Preview", "generate");
            throw false;
        }
        if ((ret = avformat_find_stream_info(format, NULL)) < 0)
        {
            LOG_DEBUG("Could not find stream information", properties.add("error", ret).toMap(), "Preview", "generate");
            throw false;
        }
        if ((ret = av_find_best_stream(format, AVMEDIA_TYPE_VIDEO, -1, -1, &decoder, 0)) < 0)
        {
            LOG_DEBUG("Could not find a video stream in input file", properties.toMap(), "Preview", "generate");
            throw false;
        }
        stream = format->streams[ret];
        if ((ret = Plugin::avcodec_open2(stream->codec, decoder, NULL)))
        {
            LOG_DEBUG("Failed to open video codec", properties.add("error", ret).toMap(), "Preview", "generate");
            throw false;
        }
        context = stream->codec;
        if (!(frame = av_frame_alloc()))
        {
            LOG_DEBUG("Could not allocate the frame", properties.toMap(), "Preview", "generate");
            throw false;
        }
        // Converts the position where the preview is captured to the stream time base
        position *= qRound64(1 / av_q2d(stream->time_base));
        // The default position is 1/2 of the video duration
        if (position > stream->duration || position == 0)
            position = stream->duration / 2;
        // Seeks the stream to the position
        if ((ret = av_seek_frame(format, stream->index, position, 0)) < 0)
        {
            LOG_DEBUG("Could not seek to the given position", properties.add("position", position).add("error", ret).toMap(), "Preview", "generate");
            throw false;
        }
        // Reads and decodes one frame
        while (av_read_frame(format, &packet) >= 0 && !gotFrame)
        {
            if (packet.stream_index == stream->index)
                if ((ret = avcodec_decode_video2(context, frame, &gotFrame, &packet)) < 0)
                {
                    LOG_DEBUG("Could not decode the video frame", properties.add("error", ret).toMap(), "Preview", "generate");
                    av_free_packet(&packet);
                    throw false;
                }
            av_free_packet(&packet);
        }
        // No frame read
        if (!gotFrame)
            throw false;
        // Keeps the original size
        if (!width && !height)
        {
            width = context->width;
            height = context->height;
        }
        // Keeps the ratio
        else if (!width && context->height)
            width = height * context->width / (float)context->height;
        else if (!height && context->width)
            height = width * context->height / (float)context->width;
        // Creates the destination image
        QImage image(width, height, QImage::Format_RGB32);
        data = image.bits();
        memset(linesize, 0, sizeof(linesize));
        linesize[0] = width * 4; // width * RGBA
        // Converts the frame to RGBA and scale it to the given size
        // "width - 1" removes a black column that appears on the right side of the image in some cases
        if (!(swsContext = sws_getContext(context->width - 1, context->height, context->pix_fmt, width, height, AV_PIX_FMT_RGB32, SWS_BICUBIC, NULL, NULL, NULL)))
        {
            LOG_DEBUG("Could not allocate the sws context", properties.toMap(), "Preview", "generate");
            throw false;
        }
        sws_scale(swsContext, frame->data, frame->linesize, 0, context->height, &data, linesize);
        // Saves the image to the destination file if the format is supported by Qt
        if (!LightBird::saveImage(image, destination, fmt, quality))
        // Otherwise converts the image into the requested format using the IImage extension
        {
            // Opens the temporary file
            tmp.setFileTemplate(this->fileTemplate);
            if (!tmp.open())
            {
                LOG_DEBUG("Error with QTemporaryFile::open", properties.add("tmp", tmp.fileName()).toMap(), "Preview", "generate");
                throw false;
            }
            // Saves the image in it
            image.save(tmp.fileName());
            // And tries to convert the image
            QListIterator<void *> it(extensions = this->api->extensions().get("IImage"));
            while (it.hasNext() && !result)
                result = static_cast<LightBird::IImage *>(it.next())->convert(tmp.fileName(), destination, fmt, width, height, quality);
            this->api->extensions().release(extensions);
            if (!result)
            {
                LOG_DEBUG("Could not find a IImage extension to convert the file", properties.toMap(), "Preview", "generate");
                throw false;
            }
        }
        result = true;
    }
    catch (bool e)
    {
        result = e;
    }
    // Cleans the allocated structures
    if (swsContext)
        sws_freeContext(swsContext);
    if (frame)
        av_frame_free(&frame);
    if (context)
        avcodec_close(context);
    if (format)
        avformat_close_input(&format);
    return (result);
}
