#ifndef IONUNSERIALIZE_H
# define IONUNSERIALIZE_H

# include "IClient.h"

namespace LightBird
{
    /// @brief This interface is called after each last calls of the IDoUnserialize*
    /// interfaces, for every plugins that implements it and has the correct context.
    /// Notice that this interface is called only if the IDoUnserialize interface called
    /// before has been implemented by a plugin. Unlike the header and the footer, IOnUnserialize
    /// is called after each calls to IDoUnserializeContent.
    class IOnUnserialize
    {
    public:
        virtual ~IOnUnserialize() {}

        /// Allows to know which interface has been called before IOnUnserialize.
        enum Unserialize
        {
            IDoUnserializeHeader,   ///< The header has been unserialized (called only if IDoUnserializeHeader has been called)
            IDoUnserializeContent,  ///< The content is beeing unserialized (called only if IDoUnserializeContent has been called)
            IDoUnserializeFooter,   ///< The footer has been unserialized (called only if IDoUnserializeFooter has been called)
            IDoUnserialize          ///< The request has been completely unserialized (called only if one of the IDoUnserialize* has been called)
        };

        /// @brief Method called to handle the request after IDoUnserialize Header,
        /// Content, or Footer.
        /// @param client : The client that has sent the request.
        /// @param type : Used to know which of the onUnserialize interfaces that has
        /// been called before.
        virtual void    onUnserialize(LightBird::IClient &client, LightBird::IOnUnserialize::Unserialize type) = 0;
    };
}

Q_DECLARE_INTERFACE(LightBird::IOnUnserialize, "cc.lightbird.IOnUnserialize")

#endif // IONUNSERIALIZE_H
