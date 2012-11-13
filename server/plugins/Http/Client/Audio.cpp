#include <QFileInfo>

#include "Audio.h"

Audio::Audio(LightBird::IClient &client) : Media(client)
{
    QString format = "mp3";
    /*qint32  bitrate = this->file.getInformation("bit rate").toInt();
    quint32 quality = this->uri.queryItemValue("quality").toUInt();
    quint32 seek = this->uri.queryItemValue("seek").toUInt();*/

    this->audio = NULL;
    // Set the format of the audio
    if (this->uri.path().contains(".ogg"))
    {
        format = "ogg";
        this->response.getHeader().insert("content-type", "audio/ogg");
    }
    else
        this->response.getHeader().insert("content-type", "audio/mpeg3");
    this->response.getHeader().insert("content-length", QString::number((quint64)(QFileInfo(this->file.getFullPath()).size() * 1.2)));
    // Get the extensions that can transcode the file
    if (!(extensions = this->api.extensions().get("IAudio")).isEmpty())
    {
        /*this->audio = static_cast<LightBird::IAudio *>(extensions.first());
        // Set the quality of the audio
        bitrate -= 32000;
        if (bitrate < 0)
            bitrate = 32000;
        audio->setBitRate((quality * bitrate / 100) + 32000);
        if (quality < 10)
            audio->setFrequency(11025);
        else if (quality < 40)
            audio->setFrequency(22050);
        // Seek to the good position
        if (seek)
            audio->setSeek(seek);
        // Set the format
        if (format == "ogg")
            this->audio->setCodec("libvorbis");
        this->audio->start(this->file.getFullPath(), "", format);*/
    }
}

Audio::~Audio()
{
    if (!this->audio)
        return ;
    /*if (!this->audio->isFinished())
        this->audio->stop();*/
    // Release the extensions
    this->api.extensions().release(extensions);
}

void    Audio::read()
{
    QByteArray  data;

    /*if (!this->audio->waitForRead())
        return ;
    data = this->audio->read(MAX_READ);*/
    this->response.getContent().setContent(data);
}
