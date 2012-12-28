#ifndef LIGHTBIRD_IDOWRITE_H
# define LIGHTBIRD_IDOWRITE_H

# include <QByteArray>

# include "IClient.h"

namespace LightBird
{
    /// @brief Interface used to write on a socket. If implemented, it allows
    /// to replace the default write made by the server.
    class IDoWrite
    {
    public:
        virtual ~IDoWrite() {}

        /// @brief Called to write on a socket.
        /// @param client : The client to whom the informations must be sent.
        /// @param data : The data to send through the network.
        /// @param size : The size of the data.
        /// @return Returns the number of bytes that were actually written,
        /// or -1 if an error occurred.
        virtual qint64  doWrite(LightBird::IClient &client, const char *data, qint64 size) = 0;
    };
}

Q_DECLARE_INTERFACE(LightBird::IDoWrite, "cc.lightbird.IDoWrite")

#endif // LIGHTBIRD_IDOWRITE_H
