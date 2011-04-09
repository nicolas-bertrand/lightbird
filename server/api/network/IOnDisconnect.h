#ifndef IONDISCONNECT_H
# define IONDISCONNECT_H

# include "IClient.h"

namespace LightBird
{
    /// @brief By inheriting this interface one can be aware of the
    /// disconnection of a client.
    class IOnDisconnect
    {
    public:
        virtual ~IOnDisconnect() {}

        /// @brief This method is called after the disconnection of a client.
        /// @param client : This object represents the client.
        virtual void    onDisconnect(LightBird::IClient &client) = 0;
    };
}

Q_DECLARE_INTERFACE(LightBird::IOnDisconnect, "cc.lightbird.IOnDisconnect");

# endif // IONDISCONNECT_H
