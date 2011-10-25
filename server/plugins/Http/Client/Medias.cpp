#include "Plugin.h"
#include "Audio.h"
#include "Medias.h"
#include "Video.h"

Medias  *Medias::instance = NULL;

Medias  &Medias::getInstance(QObject *parent)
{
    if (!Medias::instance)
        Medias::instance = new Medias(parent);
    return (*Medias::instance);
}

Medias::Medias(QObject *parent) : QObject(parent)
{
}

Medias::~Medias()
{
    QMapIterator<QString, Media *> it(this->medias);
    while (it.hasNext())
        delete it.next();
    Medias::instance = NULL;
}

void        Medias::start(LightBird::IClient &client, bool video)
{
    Media   *media;

    this->mutex.lock();
    if (this->medias.contains(client.getId()))
    {
        delete this->medias[client.getId()];
        this->medias.remove(client.getId());
    }
    if (video)
        media = new Video(client);
    else
        media = new Audio(client);
    if (media->isError())
    {
        delete media;
        this->mutex.unlock();
        return ;
    }
    this->medias[client.getId()] = media;
    this->mutex.unlock();
}

void    Medias::onFinish(LightBird::IClient &client)
{
    this->mutex.lock();
    if (this->medias.contains(client.getId()))
    {
        Plugin::getInstance().getApi().network().disconnect(client.getId());
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
            Plugin::getInstance().getApi().network().disconnect(client.getId());
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
