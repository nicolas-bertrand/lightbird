#ifndef LIGHTBIRD_IONDISCONNECT_H
# define LIGHTBIRD_IONDISCONNECT_H

# include "IClient.h"

namespace LightBird
{
    /// @brief By inheriting this interface one can be aware of the disconnection
    /// of a client.
    /// If at least one plugin that implements this interface returns false,
    /// the client will not be destroyed immediatly, and its data will continue
    /// to be processed until none remain. A client in this state can be identified
    /// via IClient::isDisconnecting. If no plugin implements this interface, the
    /// client is destroyed as soon as it is disconnected. In any case, IOnDestroy
    /// is called when the client is finally destroyed.
    /// If the disconnection is fatal, the return value of onDisconnect is ignored.
    /// @see LightBird::IClient::isDisconnecting
    /// @see LightBird::IOnDestroy
    class IOnDisconnect
    {
    public:
        virtual ~IOnDisconnect() {}

        /// @brief This method is called during the disconnection of a client,
        /// reguardless of the cause.
        /// @param client : This object represents the client.
        /// @param fatal : True if the disconnection is fatal. This happens when
        /// INetwork::disconnect is called using the fatal parameter. In this
        /// case the client is disconnected immediatly, and the return value of
        /// onDisconnect is ignored.
        /// @return Whether the client has to be destroyed immediatly (true),
        /// or when all its data have been processed (false). The return value
        /// is ignored if the disconnection is fatal.
        /// @see LightBird::INetwork::disconnect
        virtual bool    onDisconnect(LightBird::IClient &client, bool fatal) = 0;
    };
}

Q_DECLARE_INTERFACE(LightBird::IOnDisconnect, "cc.lightbird.IOnDisconnect")

#endif // LIGHTBIRD_IONDISCONNECT_H
