#include "Audio.h"
#include "LightBird.h"
#include "Medias.h"
#include "Plugin.h"
#include "Video.h"

Medias::Medias()
{
}

Medias::~Medias()
{
    QMapIterator<QString, Media *> it(this->medias);
    while (it.hasNext())
        delete it.next();
}

void    Medias::start(LightBird::IClient &client, Medias::Type type)
{
    Mutex   mutex(this->mutex, "Medias", "start");
    Media   *media = NULL;

    if (!mutex)
        return;
    if (this->medias.contains(client.getId()))
    {
        delete this->medias[client.getId()];
        this->medias.remove(client.getId());
    }
    if (type == Medias::AUDIO)
        media = new Audio(client);
    else if (type == Medias::VIDEO)
        media = new Video(client);
    if (!media || media->isError())
        return (delete media);
    this->medias[client.getId()] = media;
}

void    Medias::onFinish(LightBird::IClient &client)
{
    this->mutex.lock();
    if (this->medias.contains(client.getId()))
    {
        Plugin::api().network().disconnect(client.getId());
        delete this->medias[client.getId()];
        this->medias.remove(client.getId());
    }
    this->mutex.unlock();
}

void        Medias::update(LightBird::IClient &client)
{
    Media   *media;

    this->mutex.lock();
    if (this->medias.contains(client.getId()))
    {
        media = this->medias[client.getId()];
        // If the streaming has to be stopped
        if (this->stopList.contains(media->getId()))
        {
            Plugin::api().network().disconnect(client.getId());
            delete media;
            this->medias.remove(client.getId());
        }
        // Otherwise we try to read and send another peace of data
        else if (!media->isError())
            media->read();
    }
    this->mutex.unlock();
}

void    Medias::stop(LightBird::IClient &client)
{
    this->mutex.lock();
    this->stopList << (client.getInformations().value("sid").toString() + client.getRequest().getUri().queryItemValue("streamId"));
    this->mutex.unlock();
}

void    Medias::disconnected(LightBird::IClient &client)
{
    this->mutex.lock();
    if (this->medias.contains(client.getId()))
    {
        delete this->medias[client.getId()];
        this->medias.remove(client.getId());
    }
    this->mutex.unlock();
}
