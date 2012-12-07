#include "WriteBuffer.h"

WriteBuffer::WriteBuffer(Client *c, QByteArray *d, QThread *thread)
    : client(c)
    , data(d)
{
    this->written = 0;
    this->writing = false;
    this->moveToThread(thread);
    QObject::connect(&this->client->getSocket(), SIGNAL(bytesWritten(qint64)), this, SLOT(_bytesWritten()), Qt::QueuedConnection);
}

WriteBuffer::~WriteBuffer()
{
    delete this->data;
    this->client->bytesWritten();
}

const char  *WriteBuffer::getData() const
{
    return (this->data->data() + this->written);
}

int     WriteBuffer::getSize() const
{
    int result;

    if ((result = this->data->size() - this->written) < 0)
        result = 0;
    return (result);
}

int     WriteBuffer::getTotalSize() const
{
    return (this->data->size());
}

void    WriteBuffer::bytesWriting(int written)
{
    this->written += written;
    this->writing = true;
}

bool    WriteBuffer::isWriting() const
{
    return (this->writing);
}

bool    WriteBuffer::isWritten() const
{
    return (!this->writing && this->written >= this->data->size());
}

void    WriteBuffer::_bytesWritten()
{
    this->writing = false;
    emit this->bytesWritten();
}
