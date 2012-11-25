#ifndef IONDESERIALIZE_H
# define IONDESERIALIZE_H

# include "IClient.h"

namespace LightBird
{
    /// @brief This interface is called after each last calls of the IDoDeserialize*
    /// interfaces, for every plugins that implements it and has the correct context.
    /// Notice that this interface is called only if the IDoDeserialize interface called
    /// before has been implemented by a plugin. Unlike the header and the trailer, IOnDeserialize
    /// is called after each calls to IDoDeserializeContent.
    class IOnDeserialize
    {
    public:
        virtual ~IOnDeserialize() {}

        /// Allows to know which interface has been called before IOnDeserialize.
        enum Deserialize
        {
            IDoDeserializeHeader,  ///< The header has been deserialized (called only if IDoDeserializeHeader has been called).
            IDoDeserializeContent, ///< The content is beeing deserialized (called only if IDoDeserializeContent has been called).
            IDoDeserializeTrailer, ///< The trailer has been deserialized (called only if IDoDeserializeTrailer has been called).
            IDoDeserialize         ///< The request has been completely deserialized (called only if one of the IDoDeserialize* has been called).
        };

        /// @brief Method called to handle the request after IDoDeserialize Header,
        /// Content, or Trailer.
        /// @param client : The client that has sent the request.
        /// @param type : Used to know which of the onDeserialize interfaces that has
        /// been called before.
        virtual void    onDeserialize(LightBird::IClient &client, LightBird::IOnDeserialize::Deserialize type) = 0;
    };
}

Q_DECLARE_INTERFACE(LightBird::IOnDeserialize, "cc.lightbird.IOnDeserialize")

#endif // IONDESERIALIZE_H
