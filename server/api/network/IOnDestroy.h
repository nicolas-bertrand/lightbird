#ifndef IONDESTROY_H
# define IONDESTROY_H

# include "IClient.h"

namespace LightBird
{
    /// @brief This interface is called just before a disconnected client is
    /// destroyed, after IOnDisconnect, and allows plugins to clean all the data
    /// they gathered during its connection. Indeed, if IOnDisconnect returned
    /// false, the client is not deleted until all its data has been processed,
    /// so this interface allows plugins to know when the client is actually
    /// destroyed.
    /// @see LightBird::IOnDisconnect.
    class IOnDestroy
    {
    public:
        virtual ~IOnDestroy() {}

        /// @brief This method is called after IOnDisconnect, just before a
        /// client is actually destroyed.
        /// @param client : The client being deleted.
        virtual void    onDestroy(LightBird::IClient &client) = 0;
    };
}

Q_DECLARE_INTERFACE(LightBird::IOnDestroy, "cc.lightbird.IOnDestroy");

# endif // IONDESTROY_H
