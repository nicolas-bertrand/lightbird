#ifndef IDOEXECUTION_H
# define IDOEXECUTION_H

# include "IClient.h"

namespace Streamit
{
    /// @brief The aim of this interface is to execute a request and generate its
    /// response if necessary.
    class IDoExecution
    {
    public:
        virtual ~IDoExecution() {}

        /// @brief Allows to execute a request and generate its response.
        /// @param client : The client that made the request.
        /// @return True if a response is waited by the client.
        /// If this interface or at least one plugin called via IOnExecution
        /// returns false, no response will be sent to the client.
        virtual bool    doExecution(Streamit::IClient &client) = 0;
    };
}

Q_DECLARE_INTERFACE(Streamit::IDoExecution, "cc.lightbird.IDoExecution");

#endif // IDOEXECUTION_H
