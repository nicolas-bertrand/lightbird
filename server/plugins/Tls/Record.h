#ifndef RECORD_H
# define RECORD_H

# include <QObject>

# include "IApi.h"
# include "IDoRead.h"
# include "IDoWrite.h"

# include "Tls.h"

class Plugin;

/// @brief Handles the record protocol of TLS.
class Record
    : public QObject
    , public LightBird::IDoRead
    , public LightBird::IDoWrite
{
    Q_OBJECT
    Q_INTERFACES(LightBird::IDoRead
                 LightBird::IDoWrite)

public:
    Record(LightBird::IApi *api);
    ~Record();

    // Network
    /// @brief Reads and decrypts data from the socket.
    bool    doRead(LightBird::IClient &client, QByteArray &data);
    /// @brief Encrypts and writes data on the socket.
    qint64  doWrite(LightBird::IClient &client, const char *data, qint64 size);

    // Callbacks
    /// @brief Reads encrypted data from the QTcpSocket. Called indirectly by doRead.
    static ssize_t pull(gnutls_transport_ptr_t client, void *data, size_t size);
    /// @brief Writes encrypted data on the QTcpSocket. Called indirectly by doWrite.
    static ssize_t push(gnutls_transport_ptr_t client, const void *data, size_t size);

private:
    LightBird::IApi *api;
};

#endif // RECORD_H
