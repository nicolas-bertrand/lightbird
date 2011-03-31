#ifndef IONCONNECT_H
# define IONCONNECT_H

# include "IClient.h"

namespace Streamit
{
    /// @brief By inheriting this interface, the onConnect method will be called
    /// each time a new client connects to a port of the server.
    /// It allows to denied the connection. This is useful to banish some clients.
    class IOnConnect
    {
    public:
        virtual ~IOnConnect() {}

        /// @brief This method is called when a new client is connected.
        /// @param client : This object represents the client.
        /// @return True if the client is accepted, false otherwise (the client will be disconnect).
        virtual bool    onConnect(Streamit::IClient &client) = 0;
    };
}

Q_DECLARE_INTERFACE(Streamit::IOnConnect, "cc.lightbird.IOnConnect");

#endif // IONCONNECT_H
