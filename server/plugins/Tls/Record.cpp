#include "Record.h"

Record::Record(LightBird::IApi *a)
    : api(a)
{
}

Record::~Record()
{
}

bool    Record::doRead(LightBird::IClient &client, QByteArray &data)
{
    int result;

    data.resize(client.getSocket().size());
    if (data.isEmpty())
        return (true);
    result = gnutls_record_recv(client.getInformations().value("gnutls_session").value<gnutls_session_t>(), data.data(), data.size());
    data.resize(result);
    if (result < 0)
    {
        if (gnutls_error_is_fatal(result))
        {
            LOG_WARNING("GnuTLS fatal session error", Properties("id", client.getId()).add("error", gnutls_strerror(result)).toMap(), "Record", "doRead");
            this->api->network().disconnect(client.getId(), true);
        }
        return (false);
    }
    return (true);
}

qint64  Record::doWrite(LightBird::IClient &client, const char *data, qint64 size)
{
    int result;

    if ((result = gnutls_record_send(client.getInformations().value("gnutls_session").value<gnutls_session_t>(), data, size)) < 0)
    {
        if (gnutls_error_is_fatal(result))
        {
            LOG_WARNING("GnuTLS fatal session error", Properties("id", client.getId()).add("error", gnutls_strerror(result)).toMap(), "Record", "doWrite");
            this->api->network().disconnect(client.getId(), true);
        }
        return (-1);
    }
    return (result);
}

ssize_t Record::pull(gnutls_transport_ptr_t c, void *data, size_t size)
{
    LightBird::IClient  *client = (LightBird::IClient *)c;

    ssize_t result = client->getSocket().read((char *)data, size);
    // If no data was received, we adopt the non-blocking behavior of recv
    if (result == 0 && size > 0)
    {
        gnutls_transport_set_errno(client->getInformations().value("gnutls_session").value<gnutls_session_t>(), EAGAIN);
        return (-1);
    }
    return (result);
}

ssize_t Record::push(gnutls_transport_ptr_t c, const void *data, size_t size)
{
    LightBird::IClient  *client = (LightBird::IClient *)c;

    ssize_t result = client->getSocket().write((char *)data, size);
    return (result);
}
