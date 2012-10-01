#ifndef IDOSERIALIZEFOOTER_H
# define IDOSERIALIZEFOOTER_H

# include <QByteArray>

# include "IClient.h"

namespace LightBird
{
    /// @brief This interface is called to serialize the footer of a response, i.e convert
    /// it to a string that can be sent to the client through the network. doSerializeFooter
    /// is called one time per response, just after the last IDoSerializeContent call.
    class IDoSerializeFooter
    {
    public:
        virtual ~IDoSerializeFooter() {}

        /// @brief This method serialize the footer of the response.
        /// @param client : The client to whom the response will be send. The footer of
        /// the response is stored in IResponse, that can be get with IClient::getResponse().
        /// @param data : The serialized footer.
        virtual void    doSerializeFooter(LightBird::IClient &client, QByteArray &data) = 0;
    };
}

Q_DECLARE_INTERFACE(LightBird::IDoSerializeFooter, "cc.lightbird.IDoSerializeFooter")

#endif // IDOSERIALIZEFOOTER_H
