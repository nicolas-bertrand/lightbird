#ifndef IONDISCONNECT_H
# define IONDISCONNECT_H

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
    /// @see LightBird::IClient::isDisconnecting
    /// @see LightBird::IOnDestroy
    class IOnDisconnect
    {
    public:
        virtual ~IOnDisconnect() {}

        /// @brief This method is called after the disconnection of a client,
        /// reguardless of the cause.
        /// @param client : This object represents the client.
        /// @return Whether the client has to be destroyed immediatly (true),
        /// or when all its data have been processed (false).
        virtual bool    onDisconnect(LightBird::IClient &client) = 0;
    };
}

Q_DECLARE_INTERFACE(LightBird::IOnDisconnect, "cc.lightbird.IOnDisconnect")

# endif // IONDISCONNECT_H
