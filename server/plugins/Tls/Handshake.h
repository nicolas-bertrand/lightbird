#ifndef HANDSHAKE_H
# define HANDSHAKE_H

# include <QObject>

# include "IApi.h"
# include "IDoProtocol.h"
# include "IDoDeserializeContent.h"
# include "IDoExecution.h"
# include "IDoSerializeContent.h"
# include "IOnFinish.h"

# include "Tls.h"

class Plugin;

/// @brief Handles the handshake protocol of TLS, which appends after the connection.
/// The default context is disabled during the handshake, in order to avoid
/// interference. Once the handshake succeeded, the record context is started.
/// The client is disconnected if any error occures.
class Handshake
    : public QObject
    , public LightBird::IDoProtocol
    , public LightBird::IDoDeserializeContent
    , public LightBird::IDoExecution
    , public LightBird::IDoSerializeContent
    , public LightBird::IOnFinish
{
    Q_OBJECT
    Q_INTERFACES(LightBird::IDoProtocol
                 LightBird::IDoDeserializeContent
                 LightBird::IDoExecution
                 LightBird::IDoSerializeContent
                 LightBird::IOnFinish)

public:
    Handshake(LightBird::IApi *api, int handshakeTimeout);
    ~Handshake();

    /// @brief Starts the handshake.
    void    start(LightBird::IClient &client, gnutls_session_t session);
    /// @brief Removes the handshake timeout of the client.
    void    removeTimeout(LightBird::IClient &client);

    // Network
    /// @brief Allways returns the TLS protocol.
    bool    doProtocol(LightBird::IClient &client, const QByteArray &data, QString &protocol, bool &unknow);
    /// @brief Receives the data from the client.
    bool    doDeserializeContent(LightBird::IClient &client, const QByteArray &data, quint64 &used);
    /// @brief Performs the handshake with the data available.
    bool    doExecution(LightBird::IClient &client);
    /// @brief Sends the data to the client.
    bool    doSerializeContent(LightBird::IClient &client, QByteArray &data);
    /// @brief Waits for more data, disconnects the client, or goes to the record
    /// context, depending on the state of the handshake.
    void    onFinish(LightBird::IClient &client);

    // Callbacks
    /// @brief Behaves like a non-blocking recv.
    static ssize_t pull(gnutls_transport_ptr_t client, void *data, size_t size);
    /// @brief Behaves like a send that always works.
    static ssize_t push(gnutls_transport_ptr_t client, const void *data, size_t size);

private slots:
    /// @brief Disconnects the client when the handshake duration is elapsed.
    void    timeout();

private:
    LightBird::IApi *api;             ///< The LightBird api.
    int             handshakeTimeout; ///< The maximum duration of the handshake.
};

#endif // HANDSHAKE_H
