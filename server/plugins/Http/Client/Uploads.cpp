#include "Plugin.h"
#include "Uploads.h"

Uploads  *Uploads::instance = NULL;

Uploads  &Uploads::getInstance(QObject *parent)
{
    if (!Uploads::instance)
        Uploads::instance = new Uploads(parent);
    return (*Uploads::instance);
}

Uploads::Uploads(QObject *parent) : QObject(parent)
{
}

Uploads::~Uploads()
{
    Uploads::instance = NULL;
}

void        Uploads::start(LightBird::IClient &client)
{
    Upload  upload;
    QString id = client.getInformations().value("sid").toString() + client.getRequest().getUri().queryItemValue("id");

    this->mutex.lock();
    if (this->uploads.contains(id))
        this->uploads.remove(id);
    upload.clientId = client.getId();
    upload.size = client.getRequest().getHeader().value("content-length").toULongLong();
    upload.progress = client.getRequest().getContent().size();
    this->uploads[id] = upload;
    this->mutex.unlock();
}

void    Uploads::progress(LightBird::IClient &client)
{
    QString id = client.getInformations().value("sid").toString() + client.getRequest().getUri().queryItemValue("id");

    this->mutex.lock();
    if (this->uploads.contains(id))
    {
        this->uploads[id].progress = client.getRequest().getContent().size();
        if (this->stopList.contains(id))
        {
            this->stopList.removeAll(id);
            Plugin::getInstance().getApi().network().disconnect(client.getId());
        }
        else if (this->uploads[id].progress == this->uploads[id].size)
            this->uploads.remove(id);
    }
    this->mutex.unlock();
}

Uploads::Upload Uploads::state(LightBird::IClient &client)
{
    QString id = client.getInformations().value("sid").toString() + client.getRequest().getUri().queryItemValue("id");
    Upload  upload;

    upload.size = -1;
    upload.progress = 0;
    this->mutex.lock();
    if (this->uploads.contains(id))
        upload = this->uploads[id];
    this->mutex.unlock();
    return (upload);
}

void    Uploads::stop(LightBird::IClient &client)
{
    QString id = client.getInformations().value("sid").toString() + client.getRequest().getUri().queryItemValue("id");

    this->mutex.lock();
    if (this->uploads.contains(id))
        this->stopList << id;
    this->mutex.unlock();
}

void    Uploads::disconnected(LightBird::IClient &client)
{
    this->mutex.lock();
    QMutableMapIterator<QString, Upload> it(this->uploads);
    while (it.hasNext())
    {
        it.next();
        if (it.value().clientId == client.getId())
        {
            it.remove();
            break ;
        }
    }
    this->mutex.unlock();
}
