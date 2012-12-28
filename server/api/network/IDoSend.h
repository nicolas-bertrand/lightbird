#ifndef LIGHTBIRD_IDOSEND_H
# define LIGHTBIRD_IDOSEND_H

# include "IClient.h"

namespace LightBird
{
    /// @brief This interface is called just after a call to INetwork::send
    /// for the plugin that made the call. It is used to generate the request
    /// to send to the client. The request will then be serialized and sent
    /// through the network. It is called only when no requests are beeing
    /// processed for the client.
    /// @see LightBird::INetwork::send
    class IDoSend
    {
    public:
        virtual ~IDoSend() {}

        /// @brief Generates a request to send to the client.
        /// @param client : The client that will receive the request.
        /// @return False if there is nothing to send.
        virtual bool    doSend(LightBird::IClient &client) = 0;
    };
}

Q_DECLARE_INTERFACE(LightBird::IDoSend, "cc.lightbird.IDoSend")

#endif // LIGHTBIRD_IDOSEND_H
