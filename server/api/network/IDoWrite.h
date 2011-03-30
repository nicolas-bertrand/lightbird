#ifndef IDOWRITE_H
# define IDOWRITE_H

# include <QByteArray>
# include "IClient.h"

namespace Streamit
{
    /**
     * @brief Interface used to write on a socket. If implemented, it allows
     * to repplace the default write made by the server.
     * @see Streamit::IDoRead
     */
    class IDoWrite
    {
    public:
        virtual ~IDoWrite() {}
        /**
         * @brief Called to write on a socket.
         * @param client : The client to whom the informations must be sent.
         * @param data : The data to send through the network.
         * @return False if an error occured, true otherwise.
         */
        virtual bool    doWrite(Streamit::IClient &client, QByteArray &data)  = 0;
    };
}

Q_DECLARE_INTERFACE (Streamit::IDoWrite, "fr.streamit.IDoWrite");

#endif // IDOWRITE_H
