#include <QFileInfo>

#include "Handshake.h"
#include "Plugin.h"
#include "Record.h"

Plugin *Plugin::instance = NULL;

Plugin::Plugin()
{
    Plugin::instance = this;
}

Plugin::~Plugin()
{
    Plugin::instance = NULL;
}

bool    Plugin::onLoad(LightBird::IApi *api)
{
    int         error;
    const char  *err_pos = NULL;

    this->api = api;
    try
    {
        ASSERT_INIT(gnutls_global_init(), "global");
        if (!gnutls_check_version(GNUTLS_CHECK_VERSION))
            throw Properties("error", "Bad GnuTLS version").add("version required", GNUTLS_CHECK_VERSION);
        gnutls_global_set_audit_log_function(Plugin::log);
        this->_loadConfiguration();
        this->_loadPrivateKey();
        this->_loadCertificate();
        this->_loadDHParams();
        ASSERT_INIT(gnutls_certificate_allocate_credentials(&this->x509_cred), "credentials");
        ASSERT(gnutls_certificate_set_x509_key(this->x509_cred, &this->crt, 1, this->key));
        ASSERT_INIT(gnutls_priority_init(&this->priority, this->priorityStrings.data(), &err_pos), "priority");
        gnutls_certificate_set_dh_params(this->x509_cred, this->dhParams);
    }
    catch (Properties p)
    {
        if (err_pos)
            p.add("error position", err_pos).add("priority string", this->priorityStrings);
        LOG_FATAL("Unable to initialize GnuTLS", p.toMap(), "Plugin", "onLoad");
        this->_deinit();
        return (false);
    }
    this->contexts.insert("handshake", new Handshake(this->api, this->handshakeTimeout));
    this->contexts.insert("record", new Record(this->api));
    return (true);
}

void    Plugin::onUnload()
{
    QMapIterator<QString, QObject *> it(this->contexts);
    while (it.hasNext())
        delete it.next().value();
    this->_deinit();
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
    metadata.name = "Transport Layer Security";
    metadata.brief = "A TLS/SSL implementation.";
    metadata.description = "Secures a TCP communication using the Transport Layer Security.";
    metadata.autor = "LightBird team";
    metadata.site = "lightbird.cc";
    metadata.email = "team@lightbird.cc";
    metadata.version = "1.0";
    metadata.licence = "CC BY-NC-SA 3.0";
    metadata.dependencies << QString("GnuTLS %1").arg(GNUTLS_VERSION);
}

void    Plugin::getContexts(QMap<QString, QObject *> &contexts)
{
    contexts = this->contexts;
}

bool    Plugin::onConnect(LightBird::IClient &client)
{
    gnutls_session_t session;

    if (gnutls_init(&session, GNUTLS_SERVER) != GNUTLS_E_SUCCESS)
        return (false);
    client.getInformations().insert("gnutls_session", QVariant().fromValue(session));
    if (gnutls_priority_set(session, this->priority) != GNUTLS_E_SUCCESS)
        return (false);
    if (gnutls_credentials_set(session, GNUTLS_CRD_CERTIFICATE, this->x509_cred))
        return (false);
    gnutls_transport_set_ptr(session, (gnutls_transport_ptr_t)&client);
    // Starts the handshake
    qobject_cast<Handshake *>(this->contexts.value("handshake"))->start(client, session);
    return (true);
}

void    Plugin::onDestroy(LightBird::IClient &client)
{
    gnutls_session_t session;

    if (client.getInformations().contains("gnutls_session"))
    {
        session = client.getInformations().value("gnutls_session").value<gnutls_session_t>();
        // Properly terminates the TLS session
        if (client.getSocket().state() == QAbstractSocket::ConnectedState)
        {
            gnutls_transport_set_pull_function(session, Record::pull);
            gnutls_transport_set_push_function(session, Plugin::push);
            gnutls_bye(session, GNUTLS_SHUT_RDWR);
        }
        qobject_cast<Handshake *>(this->contexts.value("handshake"))->removeTimeout(client);
        gnutls_deinit(session);
    }
}

ssize_t Plugin::push(gnutls_transport_ptr_t c, const void *data, size_t size)
{
    LightBird::IClient  *client = (LightBird::IClient *)c;

    return (send((SOCKET)client->getSocket().socketDescriptor(), (char *)data, size, 0));
}

void    Plugin::log(gnutls_session_t, const char *log)
{
    if (Plugin::instance->api->log().isDebug())
        Plugin::instance->api->log().debug("GnuTLS log: " + QString(log), "Plugin", "log");
}

void    Plugin::_loadConfiguration()
{
    QMap<QString, gnutls_sec_param_t> secParams;
    int expiration;
    int i;

    secParams.insert("LOW", GNUTLS_SEC_PARAM_LOW);
    secParams.insert("LEGACY", GNUTLS_SEC_PARAM_LEGACY);
    secParams.insert("NORMAL", GNUTLS_SEC_PARAM_NORMAL);
    secParams.insert("HIGH", GNUTLS_SEC_PARAM_HIGH);
    secParams.insert("ULTRA", GNUTLS_SEC_PARAM_ULTRA);
    if ((this->priorityStrings = this->api->configuration(true).get("priority_strings").toLatin1()).isEmpty())
        this->priorityStrings = "SECURE128:-VERS-SSL3.0";
    if ((this->crtFile = this->api->configuration(true).get("crt")).isEmpty())
        this->crtFile = "crt.pem";
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
    if ((this->keyPassword = this->api->configuration(true).get("private_key_password").toLatin1()).isEmpty())
        this->api->configuration(true).set("private_key_password", (this->keyPassword = this->_generatePassword())).save();
    this->dhParamsExpiration = QDateTime::currentDateTime().addDays(-expiration);
    this->crtFile.prepend(this->api->getPluginPath());
    this->keyFile.prepend(this->api->getPluginPath());
    this->dhParamsFile.prepend(this->api->getPluginPath());
    // Appends the sec param to the file name
    if ((i = this->dhParamsFile.lastIndexOf('.')) >= 0)
        this->dhParamsFile.insert(i, "." + secParams.key(this->secParam).toLower());
    else
        this->dhParamsFile.append("." + secParams.key(this->secParam).toLower());
}

void    Plugin::_loadPrivateKey()
{
    QFile           file(this->keyFile);
    int             bits;
    size_t          size;
    QByteArray      data;
    gnutls_datum_t  datum;
    int             error;

    if (!file.open(QIODevice::ReadWrite))
        throw Properties("error", "Unable to open the private key file").add("file", this->keyFile);
    ASSERT_INIT(gnutls_x509_privkey_init(&this->key), "privkey");
    // Checks that the private key is valid
    if (file.size() > 0)
    {
        data = file.readAll();
        datum.size = data.size();
        datum.data = (unsigned char *)data.data();
        if ((error = gnutls_x509_privkey_import_pkcs8(this->key, &datum, GNUTLS_X509_FMT_PEM, this->keyPassword.data(), 0)) != GNUTLS_E_SUCCESS)
        {
            LOG_ERROR("Invalid private key", Properties("error", gnutls_strerror(error)).toMap(), "Plugin", "_generatePrivateKey");
            file.resize(0);
        }
        else if (gnutls_x509_privkey_sec_param(this->key) != this->secParam)
            file.resize(0);
    }
    // Generates the private key
    if (file.size() == 0)
    {
        bits = gnutls_sec_param_to_pk_bits(GNUTLS_PK_RSA, this->secParam);
        LOG_INFO("Generating a new private key", Properties("secParam", gnutls_sec_param_get_name(this->secParam)).add("bits", bits).toMap(), "Plugin", "_generatePrivateKey");
        ASSERT(gnutls_x509_privkey_generate(this->key, GNUTLS_PK_RSA, bits, 0));
        ASSERT(gnutls_x509_privkey_verify_params(this->key));
        size = bits;
        data.resize(size);
        ASSERT(gnutls_x509_privkey_export_pkcs8(this->key, GNUTLS_X509_FMT_PEM, this->keyPassword.data(), GNUTLS_PKCS_USE_PBES2_AES_256, data.data(), &size));
        data.resize(size);
        file.write(data);
    }
}

void    Plugin::_loadCertificate()
{
    QFile            file(this->crtFile);
    gnutls_datum_t   datum;
    unsigned int     size = 2048;
    QByteArray       data;
    gnutls_privkey_t privkey;
    gnutls_pubkey_t  pubkey;
    gnutls_pubkey_t  pubkeyCrt;
    int              error;
    QMap<char *, QByteArray> oid;
    gnutls_digest_algorithm_t digest;

    if (!file.open(QIODevice::ReadWrite))
        throw Properties("error", "Unable to open the certificate file").add("file", this->crtFile);
    ASSERT_INIT(gnutls_x509_crt_init(&this->crt), "crt");
    ASSERT(gnutls_privkey_init(&privkey));
    ASSERT(gnutls_privkey_import_x509(privkey, this->key, 0));
    ASSERT(gnutls_pubkey_init(&pubkey));
    ASSERT(gnutls_pubkey_import_privkey(pubkey, privkey, 0, 0));
    // Verifies that the certificate is valid
    if (file.size() > 0)
    {
        ASSERT(gnutls_pubkey_init(&pubkeyCrt));
        data = file.readAll();
        datum.size = data.size();
        datum.data = (unsigned char *)data.data();
        if (gnutls_x509_crt_import(this->crt, &datum, GNUTLS_X509_FMT_PEM) != GNUTLS_E_SUCCESS)
            file.resize(0);
        else if (gnutls_x509_crt_get_expiration_time(this->crt) < ::time(NULL) + CRT_EXPIRATION_REGEN)
            file.resize(0);
        else if (gnutls_pubkey_import_x509(pubkeyCrt, this->crt, 0) != GNUTLS_E_SUCCESS)
            file.resize(0);
        // Ensures that the public keys of the certificate and the private key match
        unsigned int size1 = size, size2 = size;
        QByteArray pub1(size1, 0), pub2(size2, 0);
        if (gnutls_pubkey_export(pubkey, GNUTLS_X509_FMT_PEM, pub1.data(), &size1) != GNUTLS_E_SUCCESS
            || gnutls_pubkey_export(pubkeyCrt, GNUTLS_X509_FMT_PEM, pub2.data(), &size2) != GNUTLS_E_SUCCESS
            || size1 != size2 || pub1 != pub2)
            file.resize(0);
        gnutls_pubkey_deinit(pubkeyCrt);
    }
    // Generates a new certificate
    if (file.size() == 0)
    {
        gnutls_x509_crt_deinit(this->crt);
        this->init.removeAll("crt");
        ASSERT_INIT(gnutls_x509_crt_init(&this->crt), "crt");
        LOG_INFO("Generating a new certificate", "Plugin", "_generateCertificate");
        oid.insert((char *)GNUTLS_OID_X520_COMMON_NAME, "LightBird");
        oid.insert((char *)GNUTLS_OID_X520_ORGANIZATION_NAME, "LightBird");
        QMapIterator<char *, QByteArray> it(oid);
        while (it.hasNext())
            ASSERT(gnutls_x509_crt_set_dn_by_oid(this->crt, it.key(), 0, it.value().data(), it.next().value().size()));
        ASSERT(gnutls_x509_crt_set_pubkey(this->crt, pubkey));
        data = this->_generateSerial();
        ASSERT(gnutls_x509_crt_set_serial(this->crt, data.data(), data.size()));
        ASSERT(gnutls_x509_crt_set_activation_time(this->crt, ::time(NULL)));
        ASSERT(gnutls_x509_crt_set_expiration_time(this->crt, ::time(NULL) + CRT_EXPIRATION));
        ASSERT(gnutls_x509_crt_set_basic_constraints(this->crt, 0, -1));
        ASSERT(gnutls_x509_crt_set_key_purpose_oid(this->crt, GNUTLS_KP_TLS_WWW_SERVER, 0));
        ASSERT(gnutls_x509_crt_set_key_usage(this->crt, GNUTLS_KEY_DIGITAL_SIGNATURE | GNUTLS_KEY_KEY_ENCIPHERMENT));
        data.resize(size);
        ASSERT(gnutls_x509_crt_get_key_id(this->crt, 0, (unsigned char *)data.data(), &size));
        ASSERT(gnutls_x509_crt_set_subject_key_id(this->crt, (unsigned char *)data.data(), size));
        ASSERT(gnutls_x509_crt_set_version(this->crt, 3));
        ASSERT(gnutls_pubkey_get_preferred_hash_algorithm(pubkey, &digest, NULL));
        ASSERT(gnutls_x509_crt_privkey_sign(this->crt, this->crt, privkey, digest, 0));
        size = data.size();
        ASSERT(gnutls_x509_crt_export(this->crt, GNUTLS_X509_FMT_PEM, data.data(), &size));
        data.resize(size);
        file.write(data);
    }
    gnutls_pubkey_deinit(pubkey);
    gnutls_privkey_deinit(privkey);
}

void    Plugin::_loadDHParams()
{
    QByteArray      data;
    QFile           file(this->dhParamsFile);
    int             bits;
    gnutls_datum_t  datum;
    int             error;

    if (!file.open(QIODevice::ReadWrite))
        throw Properties("error", "Unable to open the Diffie-Hellman parameters file").add("file", this->dhParamsFile);
    bits = gnutls_sec_param_to_pk_bits(GNUTLS_PK_DH, this->secParam);
    ASSERT_INIT(gnutls_dh_params_init(&this->dhParams), "dh_params");
    // The Diffie-Hellman parameters expired
    if (QFileInfo(file).created() < this->dhParamsExpiration)
        file.resize(0);
    // Import the DH parameters from the PEM file
    if (file.size() > 0)
    {
        data = file.readAll();
        datum.data = (unsigned char *)data.data();
        datum.size = data.size();
        if (gnutls_dh_params_import_pkcs3(this->dhParams, &datum, GNUTLS_X509_FMT_PEM) != GNUTLS_E_SUCCESS)
            file.resize(0);
    }
    // Generates the DH parameters and store them in a PEM file
    if (file.size() == 0)
    {
        LOG_INFO("Generating new Diffie-Hellman parameters. This might take some time.", Properties("secParam", gnutls_sec_param_get_name(this->secParam)).add("bits", bits).toMap(), "Plugin", "_generateDHParams");
        data.resize(bits);
        datum.data = (unsigned char *)data.data();
        datum.size = bits;
        ASSERT(gnutls_dh_params_generate2(this->dhParams, bits));
        ASSERT(gnutls_dh_params_export_pkcs3(this->dhParams, GNUTLS_X509_FMT_PEM, datum.data, &datum.size));
        file.write(data.data(), datum.size);
    }
}

QByteArray      Plugin::_generateSerial()
{
    QString     uuid = LightBird::createUuid().remove('-');
    QByteArray  result(uuid.size() / 2, 0);
    quint8      *r = (quint8 *)result.data();
    bool        ok;

    for (int i = 0; i < uuid.size(); i += 2)
        *(r++) = QString(uuid[i]).toUInt(&ok, 16) | QString(uuid[i + 1]).toUInt(&ok, 16) << 4;
    return (result);
}

QByteArray      Plugin::_generatePassword(unsigned int size)
{
    QByteArray  data(size, 0);
    int         error;

    ASSERT(gnutls_rnd(GNUTLS_RND_RANDOM, data.data(), size));
    for (unsigned int i = 0; i < size; ++i)
        data[i] = ((unsigned char)data[i] % 93 + 32);
    return (data);
}

void    Plugin::_deinit()
{
    if (this->init.contains("priority"))
        gnutls_priority_deinit(this->priority);
    if (this->init.contains("credentials"))
        gnutls_certificate_free_credentials(this->x509_cred);
    if (this->init.contains("dh_params"))
        gnutls_dh_params_deinit(this->dhParams);
    if (this->init.contains("crt"))
        gnutls_x509_crt_deinit(this->crt);
    if (this->init.contains("privkey"))
        gnutls_x509_privkey_deinit(this->key);
    if (this->init.contains("global"))
        gnutls_global_deinit();
    this->init.clear();
}
