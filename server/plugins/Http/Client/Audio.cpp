#include <QFileInfo>

#include "Audio.h"

Audio::Audio(LightBird::IClient &client)
    : Media(client)
    , audio(NULL)
{
    QString format;

    // Determines the format
    if (this->uri.path().contains(".ogg"))
    {
        format = "ogg";
        this->response.getHeader().insert("content-type", "audio/ogg");
    }
    else
    {
        format = "mp3";
        this->response.getHeader().insert("content-type", "audio/mpeg3");
    }
    this->response.getHeader().insert("transfer-encoding", "chunked");
    // Initializes the transcoding
    if (!(extensions = this->api.extensions().get("IAudio")).isEmpty())
    {
        this->audio = static_cast<LightBird::IAudio *>(extensions.first());
        this->audio->setFormat(format);
        if (format == "ogg")
            this->audio->setCodec("libvorbis");
        else
            this->audio->setCodec("libmp3lame");
        if (!this->audio->initialize(this->file.getFullPath()))
            this->_error(415, "Unsupported Media Type", "Unable to initialize the transoding of the audio.");
    }
}

Audio::~Audio()
{
    this->api.extensions().release(this->extensions);
}

void    Audio::read()
{
    this->response.getContent().setData(this->audio->transcode(), false);
}
