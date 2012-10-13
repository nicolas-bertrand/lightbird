#ifndef IONPROTOCOL_H
# define IONPROTOCOL_H

# include <QByteArray>
# include <QString>

# include "IClient.h"

namespace LightBird
{
    /// @brief By inheriting this interface, a plugin can define the name of the protocol
    /// used by a client in its request. It is called before IDoDeserializeHeader, while
    /// the plugin has not found which protocol uses the client to communicate with the server.
    /// This allows a client to use several protocols on the same port and connection (one per request).
    /// The protocol is defined from the first plugin that finds it.
    class IOnProtocol
    {
    public:
        virtual ~IOnProtocol() {}

        /// @brief This method is called at the beginning of each request, before
        /// IDoDeserializeHeader, and while false returned. It allows to define the
        /// name of the protocol used in a request. The protocol must be determined as
        /// soon as possible.
        /// @param client : This object represents the client.
        /// @param data : The data received so far by the server. It is used to define
        /// the protocol of the request. These data may contains a part of a request, or
        /// several requests.
        /// @param protocol : The name of the protocol. Must be filled when true is returned.
        /// @param unknow : If this parameter is set to true by the plugin, it means that the
        /// it doesn't know the protocol used by the request. Then this method will not be
        /// called again for this request.
        /// @return False while the protocol of the request has not been found. True is
        /// returned when the protocol name has been filled in the protocol parameter.
        virtual bool    onProtocol(LightBird::IClient &client, const QByteArray &data, QString &protocol, bool &unknow) = 0;
    };
}

Q_DECLARE_INTERFACE(LightBird::IOnProtocol, "cc.lightbird.IOnProtocol")

#endif // IONPROTOCOL_H
