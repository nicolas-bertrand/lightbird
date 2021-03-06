#include <QUrlQuery>

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
        delete it.next().value();
}

void    Medias::start(LightBird::IClient &client, Medias::Type type)
{
    Mutex   mutex(this->mutex, "Medias", "start");
    Media   *media = NULL;

    if (!mutex)
        return ;
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
    {
        delete media;
        return ;
    }
    this->medias[client.getId()] = media;
}

void    Medias::onFinish(LightBird::IClient &client)
{
    this->mutex.lock();
    if (this->medias.contains(client.getId()))
    {
        Plugin::api().network().disconnect(client.getId(), true);
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
        // The streaming has to be stopped
        if (this->stopList.contains(media->getId()))
        {
            Plugin::api().network().disconnect(client.getId(), true);
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
    this->stopList << (client.getSession()->getInformation("identifiant").toString() + ":" + QUrlQuery(client.getRequest().getUri()).queryItemValue("mediaId"));
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
