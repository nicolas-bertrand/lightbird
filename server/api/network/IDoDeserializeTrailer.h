#ifndef IDODESERIALIZETRAILER_H
# define IDODESERIALIZETRAILER_H

# include <QByteArray>

# include "IClient.h"

namespace LightBird
{
    /// @brief Converts the data received by the network to the trailer of the
    /// request, so that it can be processed. The trailer is stored in IRequest
    /// that can be accessed through IClient::getRequest(). In the case that
    /// the data received exceeds the trailer size, plugins have to indicates
    /// how many bytes they have used to fill the trailer. The other bytes will
    /// be used by an other request.
    class IDoDeserializeTrailer
    {
    public:
        virtual ~IDoDeserializeTrailer() {}

        /// @brief Converts the data received through the network to the trailer. The trailer
        /// is stored in the object IRequest that can be accessed via IClient::getRequest.
        /// @param client : The client that sent the request.
        /// @param data : The data received.
        /// @param used : If true is returned, users have to set the number of bytes used
        /// from the data, in this parameter, so that the remaining data can be used in
        /// IDoDeserializeHeader of the next request. If all the data has been consumed,
        /// the value of "used" must be equal or highter to the size of the received data.
        /// If the data received represents more than the content, users have to set the
        /// length used from data to this variable. Let zero if no data have to be used,
        /// ie if there is no content.
        /// @return True if the trailer is complete. This method will be called while
        /// false is returned.
        virtual bool    doDeserializeTrailer(LightBird::IClient &client, const QByteArray &data, quint64 &used) = 0;
    };
}

Q_DECLARE_INTERFACE(LightBird::IDoDeserializeTrailer, "cc.lightbird.IDoDeserializeTrailer")

#endif // IDODESERIALIZETRAILER_H
