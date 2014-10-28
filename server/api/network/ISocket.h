#ifndef LIGHTBIRD_ISOCKET_H
# define LIGHTBIRD_ISOCKET_H

# include <QByteArray>
# include <QHostAddress>

namespace LightBird
{
    /// @brief Abstract representation of a socket.
    class ISocket
    {
    public:
        virtual ~ISocket() {}

        /// @brief Returns true while the socket is connected.
        virtual bool isConnected() const = 0;
        /// @brief Returns the amount of data available for read.
        virtual qint64 size() const = 0;
        /// @brief Reads at most size and set the result in data.
        /// @return The number of bytes read or -1 if an error occured.
        virtual qint64 read(char *data, qint64 size) = 0;
        /// @brief Writes the data.
        /// @return The number of bytes written, which can be lower than size,
        /// or -1 if an error occured.
        /// If 0 is returned, this method will be called again later with
        /// the same parameters.
        virtual qint64 write(const char *data, qint64 size) = 0;
        /// @brief Returns the address of the peer.
        virtual const QHostAddress &peerAddress() const = 0;
        /// @brief Rethrns the name of the peer.
        virtual const QString &peerName() const = 0;
        /// @brief Returns the port of the peer.
        virtual quint16 peerPort() const = 0;
        /// @brief Returns the local port.
        virtual quint16 localPort() const = 0;
        /// @brief Returns the raw socket descriptor.
        virtual qintptr descriptor() const = 0;
    };
}

#endif // LIGHTBIRD_ISOCKET_H
