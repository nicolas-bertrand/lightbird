#include "ClientsNetwork.h"
#ifdef Q_OS_WIN
# include "ClientsNetworkWindows.h"
#elif defined Q_OS_LINUX
# include "ClientsNetworkLinux.h"
#endif // Q_OS_WIN

ClientsNetwork *ClientsNetwork::create()
{
    ClientsNetwork *clientsNetwork = NULL;

#ifdef Q_OS_WIN
    clientsNetwork = new ClientsNetworkWindows();
#elif defined Q_OS_LINUX
    clientsNetwork = new ClientsNetworkLinux();
#endif // Q_OS_WIN

    if (clientsNetwork && !*clientsNetwork)
    {
        delete clientsNetwork;
        clientsNetwork = NULL;
    }
    return clientsNetwork;
}

ClientsNetwork::ClientsNetwork()
    : _listening(false)
{
}

ClientsNetwork::~ClientsNetwork()
{
}
