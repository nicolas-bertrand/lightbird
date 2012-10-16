#include <QtPlugin>
#include <QDir>
#include <gnutls/x509.h>

#include "LightBird.h"
#include "Plugin.h"

#ifdef Q_OS_WIN32
# include <winsock2.h>
#else
# include <sys/socket.h>
#endif // Q_OS_WIN32
typedef unsigned int SOCKET;

Q_DECLARE_METATYPE(gnutls_session_t)

Plugin *Plugin::instance = NULL;

Plugin::Plugin()
{
    Plugin::instance = this;
}

Plugin::~Plugin()
{
    Plugin::instance = NULL;
}

bool        Plugin::onLoad(LightBird::IApi *api)
{
    this->api = api;
    this->_loadConfiguration();
    // Initializes GnuTLS
    if (gnutls_global_init() != GNUTLS_E_SUCCESS)
        return (false);
    if (!gnutls_check_version(GNUTLS_CHECK_VERSION))
        return (false);
    if (!this->_generatePrivateKey())
        return (false);
    if (gnutls_certificate_allocate_credentials(&this->x509_cred) != GNUTLS_E_SUCCESS)
        return (false);
    if (gnutls_certificate_set_x509_key_file(this->x509_cred, this->certFile.toAscii().data(), this->keyFile.toAscii().data(), GNUTLS_X509_FMT_PEM) != GNUTLS_E_SUCCESS)
        return (false);
    if ((gnutls_priority_init(&this->priority, "PERFORMANCE:%SERVER_PRECEDENCE", NULL)) != GNUTLS_E_SUCCESS)
        return (false);
    if (!this->_generateDHParams())
        return (false);
    gnutls_certificate_set_dh_params(this->x509_cred, this->dhParams);
    return (true);
}

void    Plugin::onUnload()
{
    gnutls_dh_params_deinit(this->dhParams);
    gnutls_certificate_free_credentials(this->x509_cred);
    gnutls_global_deinit();
}

bool    Plugin::onInstall(LightBird::IApi *api)
{
    this->api = api;
    return (true);
}

void    Plugin::onUninstall(LightBird::IApi *api)
{
    this->api = api;
}

void    Plugin::getMetadata(LightBird::IMetadata &metadata) const
{
    metadata.name = "Transport Layer Secutiry";
    metadata.brief = "A TLS/SSL implementation.";
    metadata.description = "Secures a TCP communication using the Transport Layer Security.";
    metadata.autor = "LightBird team";
    metadata.site = "lightbird.cc";
    metadata.email = "team@lightbird.cc";
    metadata.version = "1.0";
    metadata.licence = "CC BY-NC-SA 3.0";
}

bool    Plugin::onConnect(LightBird::IClient &client)
{
    gnutls_session_t session;
    int              result;
    QTime            timeout;

    if (gnutls_init(&session, GNUTLS_SERVER) != GNUTLS_E_SUCCESS)
        return (false);
    client.getInformations().insert("gnutls_session", QVariant().fromValue(session));
    if (gnutls_priority_set(session, this->priority) != GNUTLS_E_SUCCESS)
        return (false);
    if (gnutls_credentials_set(session, GNUTLS_CRD_CERTIFICATE, this->x509_cred))
        return (false);
    this->mutex.lockForWrite();
    this->clients.insert(client.getSocket().socketDescriptor(), &client);
    this->mutex.unlock();
    gnutls_transport_set_ptr(session, (gnutls_transport_ptr_t)client.getSocket().socketDescriptor());
    gnutls_transport_set_pull_function(session, Plugin::handshake_pull);
    gnutls_transport_set_push_function(session, Plugin::handshake_push);
    timeout.start();
    do
    {
        if ((result = gnutls_handshake(session)) == GNUTLS_E_AGAIN)
            // Slows down the handsake if more data are needed.
            // We can't call waitForRead because we are not in the socket thread.
            LightBird::sleep(2);
        if (timeout.elapsed() > this->handshakeTimeout)
            return (false);
    }
    while (result < 0 && gnutls_error_is_fatal(result) == 0);
    if (result < 0)
        return (false);
    gnutls_transport_set_pull_function(session, Plugin::record_pull);
    gnutls_transport_set_push_function(session, Plugin::record_push);
    return (true);
}

void    Plugin::onDestroy(LightBird::IClient &client)
{
    this->mutex.lockForWrite();
    QMutableHashIterator<int, LightBird::IClient *> it(this->clients);
    while (it.hasNext())
        if (it.next().value() == &client)
            it.remove();
    if (client.getInformations().contains("gnutls_session"))
        gnutls_deinit(client.getInformations().value("gnutls_session").value<gnutls_session_t>());
    this->mutex.unlock();
}

bool    Plugin::doRead(LightBird::IClient &client, QByteArray &data)
{
    int result;

    if (client.getSocket().size() == 0)
        return (true);
    data.resize(1024);
    result = gnutls_record_recv(client.getInformations().value("gnutls_session").value<gnutls_session_t>(), data.data(), 1024);
    data.resize(result);
    if (result < 0)
    {
        this->api->log().fatal(QString("doRead : ") + gnutls_strerror(result));
        return (false);
    }
    return (true);
}

qint64  Plugin::doWrite(LightBird::IClient &client, const char *data, qint64 size)
{
    int result;

    if ((result = gnutls_record_send(client.getInformations().value("gnutls_session").value<gnutls_session_t>(), data, size)) < 0)
        return (-1);
    return (result);
}

ssize_t Plugin::handshake_pull(gnutls_transport_ptr_t socketDescriptor, void *data, size_t size)
{
    LightBird::IClient  *client;

    Plugin::instance->mutex.lockForRead();
    client = Plugin::instance->clients.value((int)socketDescriptor);
    Plugin::instance->mutex.unlock();
    ssize_t result = client->getSocket().read((char *)data, size);
    //Plugin::instance->api->log().fatal("handshake_pull " + QString::number(result) + " " + QString::number(size));
    // If no data was received, we adopt the non-blocking behavior of recv
    if (result == 0 && size > 0)
    {
        gnutls_transport_set_errno(client->getInformations().value("gnutls_session").value<gnutls_session_t>(), EAGAIN);
        return (-1);
    }
    return (result);
}

ssize_t Plugin::handshake_push(gnutls_transport_ptr_t socketDescriptor, const void *data, size_t size)
{
    LightBird::IClient  *client;

    Plugin::instance->mutex.lockForRead();
    client = Plugin::instance->clients.value((int)socketDescriptor);
    Plugin::instance->mutex.unlock();
    ssize_t result = send((SOCKET)socketDescriptor, (char *)data, size, 0);
    //Plugin::instance->api->log().fatal("handshake_push " + QString::number(result) + " " + QString::number(size));
    return (result);
}

ssize_t Plugin::record_pull(gnutls_transport_ptr_t socketDescriptor, void *data, size_t size)
{
    LightBird::IClient  *client;

    Plugin::instance->mutex.lockForRead();
    client = Plugin::instance->clients.value((int)socketDescriptor);
    Plugin::instance->mutex.unlock();
    ssize_t result = client->getSocket().read((char *)data, size);
    //Plugin::instance->api->log().fatal("record_pull " + QString::number(result) + " " + QString::number(size));
    // If no data was received, we adopt the non-blocking behavior of recv
    if (result == 0 && size > 0)
    {
        gnutls_transport_set_errno(client->getInformations().value("gnutls_session").value<gnutls_session_t>(), EAGAIN);
        return (-1);
    }
    return (result);
}

ssize_t Plugin::record_push(gnutls_transport_ptr_t socketDescriptor, const void *data, size_t size)
{
    LightBird::IClient  *client;

    Plugin::instance->mutex.lockForRead();
    client = Plugin::instance->clients.value((int)socketDescriptor);
    Plugin::instance->mutex.unlock();
    ssize_t result = client->getSocket().write((char *)data, size);
    //Plugin::instance->api->log().fatal("record_push " + QString::number(result) + " " + QString::number(size));
    return (result);
}

void    Plugin::_loadConfiguration()
{
    QMap<QString, gnutls_sec_param_t> secParams;
    unsigned int expiration;
    int          i;

    secParams.insert("LOW", GNUTLS_SEC_PARAM_LOW);
    secParams.insert("LEGACY", GNUTLS_SEC_PARAM_LEGACY);
    secParams.insert("NORMAL", GNUTLS_SEC_PARAM_NORMAL);
    secParams.insert("HIGH", GNUTLS_SEC_PARAM_HIGH);
    secParams.insert("ULTRA", GNUTLS_SEC_PARAM_ULTRA);
    if ((this->certFile = this->api->configuration(true).get("cert")).isEmpty())
        this->certFile = "cert.pem";
    if ((this->keyFile = this->api->configuration(true).get("key")).isEmpty())
        this->keyFile = "key.pem";
    if ((this->dhParamsFile = this->api->configuration(true).get("dh_params")).isEmpty())
        this->dhParamsFile = "dh_params.pem";
    if (!(this->secParam = secParams.value(this->api->configuration(true).get("sec_param").toUpper())))
        this->secParam = GNUTLS_SEC_PARAM_HIGH;
    if (!(expiration = this->api->configuration(true).get("dh_params_expiration").toUInt()))
        expiration = 90;
    if (!(this->handshakeTimeout = this->api->configuration(true).get("handshake_timeout").toInt()))
        this->handshakeTimeout = 5000;
    this->dhParamsExpiration = QDateTime::currentDateTime().addDays(-expiration);
    this->certFile.prepend(this->api->getPluginPath());
    this->keyFile.prepend(this->api->getPluginPath());
    this->dhParamsFile.prepend(this->api->getPluginPath());
    // Appends the sec param to the file name
    if ((i = this->dhParamsFile.lastIndexOf('.')) >= 0)
        this->dhParamsFile.insert(i, "." + secParams.key(this->secParam).toLower());
    else
        this->dhParamsFile.append("." + secParams.key(this->secParam).toLower());
}

bool    Plugin::_generatePrivateKey()
{
    QFile                   file(this->keyFile);
    gnutls_x509_privkey_t   key;
    int                     bits;
    size_t                  size;
    QByteArray              data;

    if (!file.open(QIODevice::ReadWrite))
        return (false);
    if (file.size() == 0)
    {
        if (gnutls_x509_privkey_init(&key) != GNUTLS_E_SUCCESS)
            return (false);
        if ((bits = gnutls_sec_param_to_pk_bits(GNUTLS_PK_RSA, this->secParam)) == 0)
            return (false);
        if (gnutls_x509_privkey_generate(key, GNUTLS_PK_RSA, bits, 0) != GNUTLS_E_SUCCESS)
            return (false);
        if (gnutls_x509_privkey_verify_params(key) != GNUTLS_E_SUCCESS)
            return (false);
        size = bits;
        data.resize(size);
        if (gnutls_x509_privkey_export(key, GNUTLS_X509_FMT_PEM, data.data(), &size) != GNUTLS_E_SUCCESS)
            return (false);
        data.resize(size);
        file.write(data);
        gnutls_x509_privkey_deinit(key);
    }
    return (true);
}

bool                Plugin::_generateDHParams()
{
    QByteArray      data;
    QFile           file(this->dhParamsFile);
    int             bits;
    gnutls_datum_t  datum;

    if ((bits = gnutls_sec_param_to_pk_bits(GNUTLS_PK_DH, this->secParam)) == 0)
        return (false);
    if (gnutls_dh_params_init(&this->dhParams) != GNUTLS_E_SUCCESS)
        return (false);
    if (!file.open(QIODevice::ReadWrite))
        return (false);
    // The DH parameters expired
    if (QFileInfo(file).created() < this->dhParamsExpiration)
        file.remove();
    this->api->log().fatal(this->dhParamsExpiration.toString(DATE_FORMAT));
    // Import the DH params from the PEM file
    if (file.size() > 0)
    {
        data = file.readAll();
        datum.data = (unsigned char *)data.data();
        datum.size = data.size();
        if (gnutls_dh_params_import_pkcs3(this->dhParams, &datum, GNUTLS_X509_FMT_PEM) != GNUTLS_E_SUCCESS)
            file.remove();
    }
    // Generates the DH params and store them in a PEM file
    if (file.size() == 0)
    {
        data.resize(bits);
        datum.data = (unsigned char *)data.data();
        datum.size = bits;
        this->api->log().info("Generating new DH params. This might take some time.", Properties("secParam", this->secParam).add("bits", bits).toMap(), "Plugin", "_generateDHParams");
        if (gnutls_dh_params_generate2(this->dhParams, bits) != GNUTLS_E_SUCCESS)
            return (false);
        if ((gnutls_dh_params_export_pkcs3(this->dhParams, GNUTLS_X509_FMT_PEM, datum.data, &datum.size)) != GNUTLS_E_SUCCESS)
            return (false);
        file.write(data.data(), datum.size);
    }
    return (true);
}

Q_EXPORT_PLUGIN2(Plugin, Plugin)
