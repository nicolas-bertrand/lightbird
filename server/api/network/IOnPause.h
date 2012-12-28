#ifndef IONPAUSE_H
# define IONPAUSE_H

# include "IClient.h"

namespace LightBird
{
    /// @brief Called when the network workflow of a client is paused using
    /// the LightBird::INetwork::pause method. Once this interface is called,
    /// no network interface will be called except IOnDisconnect, and eventually
    /// IOnDestroy if onDisconnect returned true.
    /// IOnResume is called when the network workflow is resumed.
    /// @see LightBird::INetwork::pause
    /// @see LightBird::INetwork::resume
    /// @see LightBird::IOnResume
    /// @see LightBird::IOnDisconnect
    class IOnPause
    {
    public:
        virtual ~IOnPause() {}

        /// @brief This method is called after LightBird::INetwork::pause.
        /// @param client : The client paused.
        virtual void    onPause(LightBird::IClient &client) = 0;
    };
}

Q_DECLARE_INTERFACE(LightBird::IOnPause, "cc.lightbird.IOnPause")

# endif // IONPAUSE_H
