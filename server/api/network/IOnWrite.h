#ifndef LIGHTBIRD_IONWRITE_H
# define LIGHTBIRD_IONWRITE_H

# include <QByteArray>

# include "IClient.h"

namespace LightBird
{
    /// @brief Inheriting this interface allows to handle the response
    /// before it is sent. Since a response can be sent in multiple parts,
    /// IOnWrite is called before sending each part.
    class IOnWrite
    {
    public:
        virtual ~IOnWrite() {}

        /// @brief Called before sending the response.
        /// @param client : The client to whom the response will be sent.
        /// @param data : The data that will be sent.
        virtual void    onWrite(LightBird::IClient &client, QByteArray &data) = 0;
    };
}

Q_DECLARE_INTERFACE(LightBird::IOnWrite, "cc.lightbird.IOnWrite")

#endif // LIGHTBIRD_IONWRITE_H
