#ifndef IDOSERIALIZEHEADER_H
# define IDOSERIALIZEHEADER_H

# include <QByteArray>

# include "IClient.h"

namespace LightBird
{
    /// @brief This interface is called to serialize the header of a response, ie convert
    /// it to a string that can be sent to the client through the network. doSerializeHeader
    /// is called one time per response, just before the first IDoSerializeContent call.
    class IDoSerializeHeader
    {
    public:
        virtual ~IDoSerializeHeader() {}

        /// @brief This method serialize the header of the response.
        /// @param client : The client to whom the response will be send. The header of
        /// the response is stored in IResponse, that can be get with IClient::getResponse().
        /// @param data : The serialized header.
        virtual void    doSerializeHeader(LightBird::IClient &client, QByteArray &data) = 0;
    };
}

Q_DECLARE_INTERFACE(LightBird::IDoSerializeHeader, "cc.lightbird.IDoSerializeHeader")

#endif // IDOSERIALIZEHEADER_H
