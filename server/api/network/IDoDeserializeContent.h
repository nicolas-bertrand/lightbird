#ifndef LIGHTBIRD_IDODESERIALIZECONTENT_H
# define LIGHTBIRD_IDODESERIALIZECONTENT_H

# include <QByteArray>

# include "IClient.h"

namespace LightBird
{
    /// @brief Converts the data received by the network to the content of the
    /// request, so that it can be processed. The content is stored in IRequest
    /// that can be accessed through IClient::getRequest(). In the case that
    /// the data received exceeds the content size, plugins have to indicate
    /// how many bytes they have used to fill the content. The other bytes will
    /// be used by IDoDeserializeTrailer.
    class IDoDeserializeContent
    {
    public:
        virtual ~IDoDeserializeContent() {}

        /// @brief Convert the data received through the network to the content. The content
        /// is stored in the object IRequest that can be accessed via IClient::getRequest.
        /// @param client : The client that sent the request.
        /// @param data : The data received.
        /// @param used : If true is returned, users have to set the number of bytes used
        /// from the data, in this parameter, so that the remaining data can be used in
        /// IDoDeserializeTrailer. If all the data has been consumed, the value of "used"
        /// must be equal or highter than the size of the received data. If the data received
        /// represents more than the content, users have to set the length used from data to
        /// this variable. Let zero if no data have to be used, i.e if there is no content.
        /// @return True if the content is complete. This method will be called while
        /// false is returned.
        virtual bool    doDeserializeContent(LightBird::IClient &client, const QByteArray &data, quint64 &used) = 0;
    };
}

Q_DECLARE_INTERFACE(LightBird::IDoDeserializeContent, "cc.lightbird.IDoDeserializeContent")

#endif // LIGHTBIRD_IDODESERIALIZECONTENT_H
