#ifndef IONCONNECT_H
# define IONCONNECT_H

# include "IClient.h"

namespace LightBird
{
    /// @brief By inheriting this interface the onConnect method will be called
    /// each time a new client connects to a port of the server.
    /// It allows to deny the connection. This is useful to banish some clients.
    class IOnConnect
    {
    public:
        virtual ~IOnConnect() {}

        /// @brief This method is called when a new client is connected.
        /// @param client : This object represents the client.
        /// @return True if the client is accepted. Otherwise IOnDestroy is
        /// called and the client is disconnected.
        /// @see LightBird::IOnDestroy
        virtual bool    onConnect(LightBird::IClient &client) = 0;
    };
}

Q_DECLARE_INTERFACE(LightBird::IOnConnect, "cc.lightbird.IOnConnect")

#endif // IONCONNECT_H
