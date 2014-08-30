#include <QCoreApplication>
#include <QTimer>

#include "Handshake.h"
#include "Record.h"

Handshake::Handshake(LightBird::IApi *a, int timeout)
    : api(a)
    , handshakeTimeout(timeout)
{
}

Handshake::~Handshake()
{
}

void    Handshake::start(LightBird::IClient &client, gnutls_session_t session)
{
    QTimer  *timer = new QTimer();

    // Starts the handshake timeout in the server main thread
    timer->setProperty("idClient", client.getId());
    timer->setSingleShot(true);
    QObject::connect(timer, SIGNAL(timeout()), this, SLOT(timeout()), Qt::QueuedConnection);
    timer->start(this->handshakeTimeout);
    timer->moveToThread(QCoreApplication::instance()->thread());
    client.getInformations()["handshakeTimeout"].setValue(timer);
    // Sets the context
    gnutls_transport_set_pull_function(session, Handshake::pull);
    gnutls_transport_set_push_function(session, Handshake::push);
    client.getContexts() << "handshake";
    // Disables the default context to avoid any interference
    client.getContexts().removeAll("");
}

void    Handshake::removeTimeout(LightBird::IClient &client)
{
    if (!client.getInformations().contains("handshakeTimeout"))
        return ;
    // Deletes the handshake timeout in the server main thread
    client.getInformations().value("handshakeTimeout").value<QTimer *>()->deleteLater();
    client.getInformations().remove("handshakeTimeout");
}

bool    Handshake::doProtocol(LightBird::IClient &, const QByteArray &, QString &protocol, bool &)
{
    protocol = "TLS";
    return (true);
}

bool    Handshake::doDeserializeContent(LightBird::IClient &client, const QByteArray &data, quint64 &used)
{
    client.getRequest().getContent().setStorage(LightBird::IContent::BYTEARRAY);
    client.getRequest().getContent().setData(data);
    used = data.size();
    return (true);
}

bool    Handshake::doExecution(LightBird::IClient &client)
{
    int result;
    LightBird::IContent &content = client.getRequest().getContent();
    gnutls_session_t session = client.getInformations().value("gnutls_session").value<gnutls_session_t>();

    client.getResponse().getContent().setStorage(LightBird::IContent::BYTEARRAY);
    // Continues the handshake while we have data to feed it or it is finished
    do
    {
        result = gnutls_handshake(session);
        // Fatal handshake error
        if (gnutls_error_is_fatal(result))
        {
            LOG_WARNING("Handshake error", Properties("error", gnutls_strerror(result)).toMap(), "Handshake", "doExecution");
            client.getResponse().setError(true);
            return (true);
        }
    }
    while (result < 0 && content.size());
    // The handshake succeeded
    if (result == 0)
        client.getResponse().getInformations().insert("handshake succeeded", true);
    return (true);
}

bool    Handshake::doSerializeContent(LightBird::IClient &client, QByteArray &data)
{
    data = client.getResponse().getContent().getData();
    return (true);
}

void    Handshake::onFinish(LightBird::IClient &client)
{
    gnutls_session_t session = client.getInformations().value("gnutls_session").value<gnutls_session_t>();

    // An error occured in the handshake, so the client is disconnected
    if (client.getResponse().isError())
        this->api->network().disconnect(client.getId(), true);
    // The handshake succeeded, so we go to the record context
    else if (client.getResponse().getInformations().contains("handshake succeeded"))
    {
        this->removeTimeout(client);
        gnutls_transport_set_pull_function(session, Record::pull);
        gnutls_transport_set_push_function(session, Record::push);
        client.getContexts().removeAll("handshake");
        // Activates the record and the default contexts
        client.getContexts() << "record" << "";
    }
}

ssize_t Handshake::pull(gnutls_transport_ptr_t c, void *data, size_t size)
{
    LightBird::IClient  *client = (LightBird::IClient *)c;
    QByteArray          &content = *client->getRequest().getContent().getByteArray();

    if (size == 0)
        return (0);
    // If there is no data, we adopt the non-blocking behavior of recv
    if (!content.size())
    {
        gnutls_transport_set_errno(client->getInformations().value("gnutls_session").value<gnutls_session_t>(), EAGAIN);
        return (-1);
    }
    // There is not enougth data, so we read what we can
    if ((unsigned int)content.size() < size)
    {
        memcpy(data, content.data(), content.size());
        size = content.size();
        content.clear();
        return (size);
    }
    // All the data can be read
    memcpy(data, content.data(), size);
    content.remove(0, (int)size);
    return (size);
}

ssize_t Handshake::push(gnutls_transport_ptr_t c, const void *data, size_t size)
{
    LightBird::IClient  *client = (LightBird::IClient *)c;

    client->getResponse().getContent().setData(QByteArray((char *)data, (int)size));
    return (size);
}

void    Handshake::timeout()
{
    QTimer  *timer;

    // Disconnects the client
    if ((timer = qobject_cast<QTimer *>(QObject::sender())))
    {
        LOG_DEBUG("Handshake timeout", Properties("timeout", this->handshakeTimeout).toMap(), "Handshake", "timeout");
        this->api->network().disconnect(timer->property("idClient").toString(), true);
    }
}
