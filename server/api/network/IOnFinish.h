#ifndef IONFINISH_H
# define IONFINISH_H

# include "IClient.h"

namespace LightBird
{
    /// @brief Inheriting this interface allows to be aware when the processing
    /// of a request has been completed. This allows for example to disconnect a
    /// client when the response is complete.
    class IOnFinish
    {
    public:
        virtual ~IOnFinish() {}

        /// @brief Method called the server has finish to process a request.
        /// @param client : The client that sent the request.
        virtual void    onFinish(LightBird::IClient &client) = 0;
    };
}

Q_DECLARE_INTERFACE(LightBird::IOnFinish, "cc.lightbird.IOnFinish");

#endif // IONFINISH_H
