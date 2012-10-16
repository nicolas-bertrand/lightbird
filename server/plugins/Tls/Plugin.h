#ifndef PLUGIN_H
# define PLUGIN_H

# include <QObject>
# include <QReadWriteLock>
# include "gnutls/gnutls.h"

# include "IPlugin.h"
# include "IOnConnect.h"
# include "IOnDestroy.h"
# include "IDoRead.h"
# include "IDoWrite.h"

# define GNUTLS_CHECK_VERSION "3.1.2"

class Plugin : public QObject,
               public LightBird::IPlugin,
               public LightBird::IOnConnect,
               public LightBird::IOnDestroy,
               public LightBird::IDoRead,
               public LightBird::IDoWrite
{
    Q_OBJECT
    Q_INTERFACES(LightBird::IPlugin
                 LightBird::IOnConnect
                 LightBird::IOnDestroy
                 LightBird::IDoRead
                 LightBird::IDoWrite)

public:
    Plugin();
    ~Plugin();

    // LightBird::IPlugin
    bool    onLoad(LightBird::IApi *api);
    void    onUnload();
    bool    onInstall(LightBird::IApi *api);
    void    onUninstall(LightBird::IApi *api);
    void    getMetadata(LightBird::IMetadata &metadata) const;

    // Network
    bool    onConnect(LightBird::IClient &client);
    void    onDestroy(LightBird::IClient &client);
    bool    doRead(LightBird::IClient &client, QByteArray &data);
    qint64  doWrite(LightBird::IClient &client, const char *data, qint64 size);

    // Callbacks
    static ssize_t handshake_pull(gnutls_transport_ptr_t socketDescriptor, void *data, size_t size);
    static ssize_t handshake_push(gnutls_transport_ptr_t socketDescriptor, const void *data, size_t size);
    static ssize_t record_pull(gnutls_transport_ptr_t socketDescriptor, void *data, size_t size);
    static ssize_t record_push(gnutls_transport_ptr_t socketDescriptor, const void *data, size_t size);

private:
    /// @brief Loads the configuration of the plugin.
    void        _loadConfiguration();
    /// @brief Checks the RSA private key, and generate it if necessary.
    bool        _generatePrivateKey();
    /// @brief Generates a self-signed x.509 certificate based on the private key.
    bool        _generateCertificate();
    /// @brief Generate Diffie-Hellman parameters. For use with DHE kx algorithms.
    /// When short bit length is used, it might be wise to regenerate parameters often.
    /// @return False if an error occured.
    bool        _generateDHParams();
    /// @brief Generates a 16 bytes random serial number for the x.509 certificate.
    QByteArray  _generateSerial();

    static Plugin         *instance;
    LightBird::IApi       *api;
    gnutls_certificate_credentials_t x509_cred;
    gnutls_x509_crt_t     crt;                ///< The x.509 certificate.
    gnutls_x509_privkey_t key;                ///< The RSA private key.
    QString               crtFile;            ///< The name of the certificate file.
    QString               keyFile;            ///< The name of the key file.
    gnutls_dh_params_t    dhParams;
    QString               dhParamsFile;       ///< The name of the file that stores the DH params cache.
    QDateTime             dhParamsExpiration; ///< The expiration date of the DH params.
    gnutls_sec_param_t    secParam;           ///< Security parameters for passive attacks (GNUTLS_SEC_PARAM_*).
    gnutls_priority_t     priority;
    QByteArray            priorityStrings;    ///< The priority strings for the handshake algorithms.
    int                   handshakeTimeout;   ///< The maximum duration of the handshake.
    QReadWriteLock        mutex;
    QHash<int, LightBird::IClient *> clients;
};

#endif // PLUGIN_H
