#ifndef IONWROTE_H
# define IONWROTE_H

# include "IClient.h"

namespace Streamit
{
    /// @brief Inheriting this interface allows to be aware when a response
    /// has been completely sent. This allows for example to disconnect a
    /// client when the response is complete.
    class IOnWrote
    {
    public:
        virtual ~IOnWrote() {}

        /// @brief Method called after sending the entire response.
        /// @param client : The client to whom the response has been sent.
        virtual void    onWrote(Streamit::IClient &client) = 0;
    };
}

Q_DECLARE_INTERFACE(Streamit::IOnWrote, "cc.lightbird.IOnWrote");

#endif // IONWRITE_H
