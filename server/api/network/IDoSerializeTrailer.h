#ifndef LIGHTBIRD_IDOSERIALIZETRAILER_H
# define LIGHTBIRD_IDOSERIALIZETRAILER_H

# include <QByteArray>

# include "IClient.h"

namespace LightBird
{
    /// @brief This interface is called to serialize the trailer of a response, i.e convert
    /// it to a string that can be sent to the client through the network. doSerializeTrailer
    /// is called one time per response, just after the last IDoSerializeContent call.
    class IDoSerializeTrailer
    {
    public:
        virtual ~IDoSerializeTrailer() {}

        /// @brief This method serialize the trailer of the response.
        /// @param client : The client to whom the response will be send. The trailer of
        /// the response is stored in IResponse, that can be get with IClient::getResponse().
        /// @param data : The serialized trailer.
        virtual void    doSerializeTrailer(LightBird::IClient &client, QByteArray &data) = 0;
    };
}

Q_DECLARE_INTERFACE(LightBird::IDoSerializeTrailer, "cc.lightbird.IDoSerializeTrailer")

#endif // LIGHTBIRD_IDOSERIALIZETRAILER_H
