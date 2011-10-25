#include <QFileInfo>

#include "Video.h"

Video::Video(LightBird::IClient &client) : Media(client)
{
    QString format;
    quint32 width = this->file->getInformation("width").toUInt();
    quint32 quality = this->uri.queryItemValue("quality").toUInt();
    quint32 seek = this->uri.queryItemValue("seek").toUInt();

    this->video = NULL;
    this->response.getHeader().insert("content-length", QString::number((quint64)(QFileInfo(this->file->getFullPath()).size() * 1.2)));
    // Get the extensions that can transcode the file
    if (!(extensions = this->api.extensions().get("IVideo")).isEmpty())
    {
        this->video = static_cast<LightBird::IVideo *>(extensions.first());
        // Set the quality of the video
        width -= 100;
        if (width < 0)
            width = 480;
        width = (quality * width / 100) + 100;
        video->setWidth(width);
        if (width >= 480)
            video->setVideoBitRate(1000000);
        // Seek to the good position
        if (seek)
            video->setSeek(seek);
        // Set the format of the video
        if (this->uri.path().contains(".webm"))
        {
            format = "webm";
            video->setVideoCodec("libvpx");
            video->setAudioCodec("libvorbis");
            this->response.getHeader().insert("content-type", "video/webm");
        }
        else if (this->uri.path().contains(".mp4"))
        {
            format = "avi";
            video->setVideoCodec("libx264");
            video->setAudioCodec("libmp3lame");
            QMap<QString, QString> options;
            options["x264"] = "faster";
            video->setOptions(options);
            // Ensure that the size of the video is not odd
            int w = this->file->getInformation("width").toUInt();
            int h = this->file->getInformation("height").toUInt();
            if (w && h)
            {
                h = (h * 480) / w;
                if (h % 2)
                    h++;
                video->setHeight(h);
            }
            this->response.getHeader().insert("content-type", "video/mp4");
        }
        else
        {
            format = "ogg";
            video->setVideoCodec("libtheora");
            video->setAudioCodec("libvorbis");
            if (width < 480)
                video->setVideoBitRate(300000);
            this->response.getHeader().insert("content-type", "video/ogg");
        }
        this->video->start(this->file->getFullPath(), "", format);
    }
}

Video::~Video()
{
    if (!this->video)
        return ;
    if (!this->video->isFinished())
        this->video->stop();
    // Release the extensions
    this->api.extensions().release(extensions);
}

void    Video::read()
{
    QByteArray  data;

    if (!this->video->waitForRead())
        return ;
    data = this->video->read(MAX_READ);
    this->response.getContent().setContent(data);
}
