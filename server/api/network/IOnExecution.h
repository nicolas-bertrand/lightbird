#ifndef IONEXECUTION_H
# define IONEXECUTION_H

# include "IClient.h"

namespace LightBird
{
    /// @brief This interface is called after IDoExecution.
    class IOnExecution
    {
    public:
        virtual ~IOnExecution() {}

        /// @brief Allows to consult the request and the response.
        /// @param client : The client that made the request.
        /// @return If at least one plugin called via this interface or IDoExecution
        /// returns false, no response will be sent to the client.
        virtual bool    onExecution(LightBird::IClient &client) = 0;
    };
}

Q_DECLARE_INTERFACE(LightBird::IOnExecution, "cc.lightbird.IOnExecution");

#endif // IONEXECUTION_H
