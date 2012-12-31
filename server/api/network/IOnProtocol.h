#ifndef LIGHTBIRD_IONPROTOCOL_H
# define LIGHTBIRD_IONPROTOCOL_H

# include "IClient.h"

namespace LightBird
{
    /// @brief Called just after the protocol of the request has been determined
    /// in IDoProtocol, and before IDoDeserializeHeader, this interface allows the
    /// plugins to prepare the execution of the request, based on its protocol.
    class IOnProtocol
    {
    public:
        virtual ~IOnProtocol() {}

        /// @brief This method is called after the last call to IDoProtocol,
        /// and before the first IDoDeserializeHeader.
        /// @param client : This object represents the client.
        virtual void    onProtocol(LightBird::IClient &client) = 0;
    };
}

Q_DECLARE_INTERFACE(LightBird::IOnProtocol, "cc.lightbird.IOnProtocol")

#endif // LIGHTBIRD_IONPROTOCOL_H
