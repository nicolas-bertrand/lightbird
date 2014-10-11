#include "WriteBuffer.h"

WriteBuffer::WriteBuffer(Client *client, const QByteArray &data, qint64 written)
    : _client(client)
    , _data(data)
    , _written(written)
{
}

WriteBuffer::~WriteBuffer()
{
}
