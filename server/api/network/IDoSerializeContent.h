#ifndef IDOSERIALIZECONTENT_H
# define IDOSERIALIZECONTENT_H

# include "IClient.h"

namespace LightBird
{
    /// @brief This interface is called to serialize the content of a response, i.e convert
    /// it to a string that can be sent to the client through the network. doSerializeContent
    /// is called while the entire content of the response has not been sent, and after IDoSerializeHeader.
    class IDoSerializeContent
    {
    public:
        virtual ~IDoSerializeContent() {}

        /// @brief This method serialize the content. It is called while false is returned,
        /// which means that the content has not been completely sent yet.
        /// @param client : The client to whom the response will be send. The content of
        /// the response is stored in IResponse, that can be accessed via IClient::getResponse().
        /// @param data : The content serialized by the plugin.
        /// @return False while more data have to be serialized, true otherwise.
        virtual bool    doSerializeContent(LightBird::IClient &client, QByteArray &data) = 0;
    };
}

Q_DECLARE_INTERFACE(LightBird::IDoSerializeContent, "cc.lightbird.IDoSerializeContent");

#endif // IDOSERIALIZECONTENT_H
