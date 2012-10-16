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
    void    _loadConfiguration();
    bool    _generatePrivateKey();
    /// @brief Generate Diffie-Hellman parameters. For use with DHE kx algorithms.
    /// When short bit length is used, it might be wise to regenerate parameters often.
    /// @return False if an error occured.
    bool    _generateDHParams();

    static Plugin       *instance;
    LightBird::IApi     *api;
    gnutls_certificate_credentials_t x509_cred;
    QString             certFile;           ///< The name of the certificate file.
    QString             keyFile;            ///< The name of the key file.
    gnutls_dh_params_t  dhParams;
    QString             dhParamsFile;       ///< The name of the file that stores the DH params cache.
    gnutls_sec_param_t  secParam;           ///< The security level of the DH params (GNUTLS_SEC_PARAM_*).
    QDateTime           dhParamsExpiration; ///< The expiration date of the DH params.
    gnutls_priority_t   priority;
    int                 handshakeTimeout;   ///< The maximum duration of the handshake.
    QReadWriteLock      mutex;
    QHash<int, LightBird::IClient *> clients;
};

#endif // PLUGIN_H
