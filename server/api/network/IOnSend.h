#ifndef IONSEND_H
# define IONSEND_H

# include "IClient.h"

namespace LightBird
{
    /// @brief This interface is called just after IDoSend, and can be used
    /// to prepare the request that is going to be sent to a client.
    /// @see LightBird::IDoSend
    class IOnSend
    {
    public:
        virtual ~IOnSend() {}

        /// @brief Called just after IDoSend.
        /// @param client : The client that will receive the request.
        /// @return False to cancel the send. If at least one plugin that
        /// implements IOnSend returns false the request is canceled.
        virtual bool    onSend(LightBird::IClient &client) = 0;
    };
}

Q_DECLARE_INTERFACE(LightBird::IOnSend, "cc.lightbird.IOnSend");

#endif // IONSEND_H
