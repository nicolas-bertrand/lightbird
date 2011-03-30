#ifndef IONREAD_H
# define IONREAD_H

# include "IClient.h"

namespace Streamit
{
    /// @brief The onRead method of this interface is called after each IDoRead calls.
    class IOnRead
    {
    public:
        virtual ~IOnRead() {}
        /**
         * @brief Method called to handle the request after reading.
         * @param client : The client that has sent the request.
         * @param data : The data received from the network.
         */
        virtual void    onRead(Streamit::IClient &client, QByteArray &data) = 0;
    };
}

Q_DECLARE_INTERFACE (Streamit::IOnRead, "fr.streamit.IOnRead");

#endif // IONREAD_H
