#ifndef PLUGIN_H
# define PLUGIN_H

# include <QObject>
# include <QReadWriteLock>

// Windows does not define ssize_t
# ifdef Q_OS_WIN
#  define ssize_t int
# endif // Q_OS_WIN

# include "gnutls/gnutls.h"

# include "IPlugin.h"
# include "IOnConnect.h"
# include "IOnDestroy.h"
# include "IDoRead.h"
# include "IDoWrite.h"

# define GNUTLS_CHECK_VERSION "3.1.2"            // The GnuTLS version used by the current implementation of the plugin.
# define CRT_EXPIRATION       360 * 24 * 60 * 60 // The number of seconds of validity of the certificate.
# define CRT_EXPIRATION_REGEN 7 * 24 * 60 * 60   // The number of seconds before the expiration of the certificate after which it is regenerated.
# define WAIT_FOR_READ 4                         // The number of milliseconds during which the server sleeps, waiting for more data.

class Plugin : public QObject,
               public LightBird::IPlugin,
               public LightBird::IOnConnect,
               public LightBird::IOnDestroy,
               public LightBird::IDoRead,
               public LightBird::IDoWrite
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "cc.lightbird.Tls")
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
    /// @brief Runs the handshake.
    bool    onConnect(LightBird::IClient &client);
    void    onDestroy(LightBird::IClient &client);
    /// @brief Reads and decrypts data from the socket.
    bool    doRead(LightBird::IClient &client, QByteArray &data);
    /// @brief Encrypts and writes data on the socket.
    qint64  doWrite(LightBird::IClient &client, const char *data, qint64 size);

    // Callbacks
    /// @brief Reads data from the QTcpSocket. Called during the handshake.
    static ssize_t handshake_pull(gnutls_transport_ptr_t socketDescriptor, void *data, size_t size);
    /// @brief Writes data directly on the socket using send. This is needed
    /// because data can only be written using QTcpSocket from the thread where
    /// the object lives. Called during the handshake.
    static ssize_t handshake_push(gnutls_transport_ptr_t socketDescriptor, const void *data, size_t size);
    /// @brief Reads encrypted data from the QTcpSocket. Called indirectly by doRead.
    static ssize_t record_pull(gnutls_transport_ptr_t socketDescriptor, void *data, size_t size);
    /// @brief Writes encrypted data on the QTcpSocket. Called indirectly by doWrite.
    static ssize_t record_push(gnutls_transport_ptr_t socketDescriptor, const void *data, size_t size);
    /// @brief Receives GnuTLS logs.
    static void    log(gnutls_session_t session, const char *log);

private:
    /// @brief Loads the configuration of the plugin.
    void        _loadConfiguration();
    /// @brief Loads the RSA private key, and generates it if necessary.
    void        _loadPrivateKey();
    /// @brief Loads the self-signed X.509 certificate based on the private key,
    /// and generates it if necessary.
    void        _loadCertificate();
    /// @brief Loads the Diffie-Hellman parameters for use with DHE kx algorithms,
    /// and generates them if necessary.
    void        _loadDHParams();
    /// @brief Generates a 16 bytes random serial number for the X.509 certificate.
    QByteArray  _generateSerial();
    /// @brief Generates a password for the private key.
    QByteArray  _generatePassword(unsigned int size = 32);
    /// @brief Deinit all the structures allocated by GnuTLS.
    void        _deinit();

    static Plugin         *instance;            ///< Allows the static methods to access the plugin.
    LightBird::IApi       *api;                 ///< The LightBird Api.
    gnutls_priority_t     priority;             ///< The priority strings for the handshake algorithms.
    gnutls_sec_param_t    secParam;             ///< Security parameters for passive attacks.
    gnutls_x509_crt_t     crt;                  ///< The X.509 certificate.
    gnutls_x509_privkey_t key;                  ///< The RSA private key.
    gnutls_certificate_credentials_t x509_cred; ///< The X.509 certificate associated with its private key.
    QString               crtFile;              ///< The name of the certificate file.
    QString               keyFile;              ///< The name of the key file.
    gnutls_dh_params_t    dhParams;             ///< The Diffie-Hellman parameters.
    QByteArray            priorityStrings;      ///< The handshake algorithms allowed by the server.
    QString               dhParamsFile;         ///< The name of the file that stores the Diffie-Hellman parameters cache.
    QDateTime             dhParamsExpiration;   ///< The expiration of the Diffie-Hellman parameters.
    int                   handshakeTimeout;     ///< The maximum duration of the handshake.
    QByteArray            keyPassword;          ///< The password with which the private key is encrypted and stored.
    QReadWriteLock        mutex;                ///< Makes this->clients thread safe.
    QHash<int, LightBird::IClient *> clients;   ///< Associates a socket descriptor with its IClient.
    QStringList           init;                 ///< The list of the GnuTLS structures initialized.
};

// Throws an exception if the gnutls call failed
# define ASSERT(GNUTLS_FUNC) \
if ((error = GNUTLS_FUNC) != GNUTLS_E_SUCCESS) \
    throw Properties("error", gnutls_strerror(error)).add("line", __LINE__); \
else (void)0

// Throws an exception if the gnutls_init call failed.
// Otherwise marks the structure as initialized.
# define ASSERT_INIT(GNUTLS_FUNC, STRUCT) \
if ((error = GNUTLS_FUNC) != GNUTLS_E_SUCCESS) \
    throw Properties("error", gnutls_strerror(error)).add("line", __LINE__); \
else \
    this->init << STRUCT

#endif // PLUGIN_H
