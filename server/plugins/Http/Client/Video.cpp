#include <QFileInfo>
#include <qmath.h>
#include <QUrlQuery>

#include "Video.h"

Video::Video(LightBird::IClient &client)
    : Media(client)
    , video(NULL)
{
    QString format;
    //qint32  width = this->file.getInformation("width").toInt();
    //quint32 quality = this->uri.queryItemValue("quality").toUInt();

    //this->response.getHeader().insert("content-length", "100000000");
    this->response.getHeader().insert("transfer-encoding", "chunked");
    // Get the extensions that can transcode the file
    if (!(extensions = this->api.extensions().get("IVideo")).isEmpty())
    {
        this->video = static_cast<LightBird::IVideo *>(extensions.first());
        this->video->setStart(qFloor(QUrlQuery(this->uri).queryItemValue("start").toFloat()));
        //this->video->setDuration(10);
        // Set the quality of the video
        /*width -= 100;
        if (width < 0)
            width = 480;
        width = (quality * width / 100) + 100;
        video->setWidth(width);
        if (width >= 480)
            video->setVideoBitRate(1000000);
        // Seek to the good position
        if (seek)
            video->setSeek(seek);*/
        // Set the format of the video
        if (this->uri.path().contains(".webm"))
        {
            format = "webm";
            this->video->setFormat("webm");
            this->video->setVideoCodec("libvpx");
            this->video->setAudioCodec("libvorbis");
            this->response.getHeader().insert("content-type", "video/webm");
        }
        else if (this->uri.path().contains(".mp4"))
        {
            format = "avi";
            this->video->setFormat("mp4");
            this->video->setVideoCodec("libx264");
            this->video->setAudioCodec("libmp3lame");
            QVariantHash options;
            options["x264"] = "faster";
            this->video->setOptions(options);
            // Ensure that the size of the video is not odd
            int w = this->file.getInformation("width").toUInt();
            int h = this->file.getInformation("height").toUInt();
            if (w && h)
            {
                h = (h * 480) / w;
                if (h % 2)
                    h++;
                this->video->setHeight(h);
            }
            this->response.getHeader().insert("content-type", "video/mp4");
        }
        else
        {
            format = "ogg";
            this->video->setFormat("ogg");
            this->video->setVideoCodec("libtheora");
            this->video->setAudioCodec("libvorbis");
            this->response.getHeader().insert("content-type", "video/ogg");
        }
        if (!this->video->initialize(this->file.getFullPath()))
            this->_error(415, "Unsupported Media Type", "Unable to initialize the transoding of the video.");
    }
}

Video::~Video()
{
    this->api.extensions().release(this->extensions);
}

void    Video::read()
{
    this->response.getContent().setData(this->video->transcode(), false);
}
