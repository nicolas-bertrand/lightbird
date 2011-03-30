#ifndef IDOSERIALIZECONTENT_H
# define IDOSERIALIZECONTENT_H

# include "IClient.h"

namespace Streamit
{
    /// @brief This interface is called to serialize the content of a response, ie convert
    /// it to a string that can be sent to the client through the network. doSerializeContent
    /// is called while the entire content of the response has been sent, and after IDoSerializeHeader.
    class IDoSerializeContent
    {
    public:
        virtual ~IDoSerializeContent() {}

        /**
         * @brief This method serialize the content. It is called while false is returned,
         * which means that the content has not been completely sent yet.
         * @param client : The client to whom the response will be send. The content of
         * the response is stored in IResponse, that can be get with IClient::getResponse().
         * @param data : The serialized content.
         * @return False while more data have to be serialized, true otherwise.
         */
        virtual bool    doSerializeContent(Streamit::IClient &client, QByteArray &data) = 0;
    };
}

Q_DECLARE_INTERFACE (Streamit::IDoSerializeContent, "fr.streamit.IDoSerializeContent");

#endif // IDOSERIALIZECONTENT_H
