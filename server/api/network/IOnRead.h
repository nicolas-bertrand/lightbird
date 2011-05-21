#ifndef IONREAD_H
# define IONREAD_H

# include <QByteArray>

# include "IClient.h"

namespace LightBird
{
    /// @brief The onRead method of this interface is called after each IDoRead calls.
    class IOnRead
    {
    public:
        virtual ~IOnRead() {}

        /// @brief Method called to handle the request after reading.
        /// @param client : The client that sent the request.
        /// @param data : The data received from the network.
        virtual void    onRead(LightBird::IClient &client, QByteArray &data) = 0;
    };
}

Q_DECLARE_INTERFACE(LightBird::IOnRead, "cc.lightbird.IOnRead");

#endif // IONREAD_H
