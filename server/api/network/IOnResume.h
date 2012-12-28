#ifndef IONRESUME_H
# define IONRESUME_H

# include "IClient.h"

namespace LightBird
{
    /// @brief Called when the network workflow of a client is resumed.
    /// @see LightBird::INetwork::pause
    /// @see LightBird::INetwork::resume
    class IOnResume
    {
    public:
        virtual ~IOnResume() {}

        /// @brief This method is called after LightBird::INetwork::resume.
        /// @param client : The client resumed.
        /// @param timeout : True if the pause duration elapsed, and false if
        /// LightBird::INetwork::resume has been called or the client is going
        /// to be destroyed because IOnDisconnect returned true.
        virtual void    onResume(LightBird::IClient &client, bool timeout) = 0;
    };
}

Q_DECLARE_INTERFACE(LightBird::IOnResume, "cc.lightbird.IOnResume")

# endif // IONRESUME_H
