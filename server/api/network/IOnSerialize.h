#ifndef IONSERIALIZE_H
# define IONSERIALIZE_H

# include "IClient.h"

namespace LightBird
{
    /// @brief This interface is called before each calls of the IDoSerialize*
    /// interfaces, for every plugins that implements it and has the correct context.
    /// Notice that this interface is called only if the IDoSerialize interface called
    /// after has been implemented by a plugin.
    class IOnSerialize
    {
    public:
        virtual ~IOnSerialize() {}

        /// Allows to know which interface will be called after IOnSerialize.
        enum Serialize
        {
            IDoSerialize,           ///< The request is going to be serialized
            IDoSerializeHeader,     ///< The header is going to be serialized (called only if IDoSerializeHeader will be called)
            IDoSerializeContent,    ///< The content is going to be serialized (called only if IDoSerializeContent will be called)
            IDoSerializeFooter      ///< The footer is going to be serialized (called only if IDoSerializeFooter will be called)
        };

        /// @brief Method called to handle the request before IDoSerialize Header,
        /// Content, or Footer.
        /// @param client : The client that has sent the request.
        /// @param type : Used to know which of the IDoSerialize interfaces is going
        /// to be called.
        /// @return In CLIENT mode, false is returned if the request doesn't need
        /// a response, for Serialize::IDoSerialize. In SERVER mode the returned
        /// value is ignored.
        virtual bool    onSerialize(LightBird::IClient &client, LightBird::IOnSerialize::Serialize type) = 0;
    };
}

Q_DECLARE_INTERFACE(LightBird::IOnSerialize, "cc.lightbird.IOnSerialize");

#endif // IONSERIALIZE_H
