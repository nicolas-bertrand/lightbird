#ifndef IDOREAD_H
# define IDOREAD_H

# include <QByteArray>

# include "IClient.h"

namespace LightBird
{
    /// @brief Interface uses to read on a socket.
    /// It replaces the default read made by the server. This interface is not called
    /// if the client is connected via UDP in SERVER mode, because the server needs
    /// to read the UDP datagram itself in order to know which client sent it.
    class IDoRead
    {
    public:
        virtual ~IDoRead() {}

        /// @brief Called to read on a socket.
        /// @param client : Informations about the client and its socket.
        /// @param data : The data that have been read.
        /// @return False if an error occured, true otherwise.
        virtual bool    doRead(LightBird::IClient &client, QByteArray &data) = 0;
    };
}

Q_DECLARE_INTERFACE(LightBird::IDoRead, "cc.lightbird.IDoRead");

#endif // IDOREAD_H
