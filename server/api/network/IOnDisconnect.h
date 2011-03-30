#ifndef IONDISCONNECT_H
# define IONDISCONNECT_H

# include "IClient.h"

namespace Streamit
{
    /**
     * @brief By inheriting this interface, one can execute a fonction
     * just after the disconnection of a client.
     */
    class IOnDisconnect
    {
    public:
        virtual ~IOnDisconnect() {}

        /**
         * @brief This method is called after the disconnection of a client.
         * @param client : This object represents the client.
         */
        virtual void    onDisconnect(Streamit::IClient &client) = 0;
    };
}

Q_DECLARE_INTERFACE (Streamit::IOnDisconnect, "fr.streamit.IOnDisconnect");

# endif // IONDISCONNECT_H
