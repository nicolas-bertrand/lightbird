#ifndef IDOREAD_H
# define IDOREAD_H

# include <QByteArray>
# include "IClient.h"

namespace Streamit
{
    /**
     * @brief Interface uses to read on a socket.
     *
     * It replace the default read made by the server. This interface is not called
     * if the client is connected via UDP, because the server needs to read the UDP
     * datagram in order to know which client sent it.
     * The doRead method is called only once per request, even if several plugins
     * implements it in the correct context. The execution priority of the plugins
     * depends on the order in which they are loaded.
     */
    class IDoRead
    {
    public:
        virtual ~IDoRead() {}
        /**
         * @brief Called to read on a socket.
         * @param client : Informations about the client and its socket.
         * @param data : The data that have been read.
         * @return False if an error occured, true otherwise.
         */
        virtual bool    doRead(Streamit::IClient &client, QByteArray &data) = 0;
    };
}

Q_DECLARE_INTERFACE (Streamit::IDoRead, "fr.streamit.IDoRead");

#endif // IDOREAD_H
